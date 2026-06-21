#include "multistream-engine.hpp"

#include <obs.h>
#include <obs-frontend-api.h>
#include <util/platform.h>

#include <string>

namespace ohmydj {

namespace {

std::string ConfigFilePath()
{
	char *profile = obs_frontend_get_current_profile_path();
	if (!profile)
		return {};
	std::string path = profile;
	path += "/oh-my-dj-stream.json";
	bfree(profile);
	return path;
}

} // namespace

// Per-output context handed to the libobs signal callbacks. It bounces the
// status back onto the Qt thread, where the engine lives.
struct OutputContext {
	MultistreamEngine *engine;
	int index;
	obs_output_t *output = nullptr;
	obs_service_t *service = nullptr;
};

struct MultistreamEngine::Impl {
	std::vector<StreamTarget> targets;
	int videoBitrate = 6000;
	bool running = false;
	bool ownEncoders = false; // true => we created venc/aenc and must release them

	obs_encoder_t *venc = nullptr;
	obs_encoder_t *aenc = nullptr;
	std::vector<OutputContext *> outputs;
};

static void EmitStatus(OutputContext *ctx, StreamStatus status)
{
	MultistreamEngine *engine = ctx->engine;
	const int index = ctx->index;
	const int value = static_cast<int>(status);
	// Hop to the engine's (UI) thread before touching Qt widgets downstream.
	QMetaObject::invokeMethod(
		engine, [engine, index, value]() { emit engine->targetStatusChanged(index, value); },
		Qt::QueuedConnection);
}

static void OnOutputStart(void *data, calldata_t *)
{
	EmitStatus(static_cast<OutputContext *>(data), StreamStatus::Live);
}

static void OnOutputStop(void *data, calldata_t *cd)
{
	auto *ctx = static_cast<OutputContext *>(data);
	const long long code = calldata_int(cd, "code");
	EmitStatus(ctx, code == OBS_OUTPUT_SUCCESS ? StreamStatus::Stopped : StreamStatus::Error);
}

MultistreamEngine::MultistreamEngine(QObject *parent) : QObject(parent), d_(std::make_unique<Impl>()) {}

MultistreamEngine::~MultistreamEngine()
{
	stop();
}

void MultistreamEngine::setTargets(std::vector<StreamTarget> targets)
{
	d_->targets = std::move(targets);
}

void MultistreamEngine::setVideoBitrate(int kbps)
{
	if (kbps > 0)
		d_->videoBitrate = kbps;
}

bool MultistreamEngine::running() const
{
	return d_->running;
}

void MultistreamEngine::start(bool useMainEncoders)
{
	if (d_->running)
		return;

	bool any = false;
	for (const StreamTarget &t : d_->targets) {
		if (t.enabled && !t.url.empty()) {
			any = true;
			break;
		}
	}
	if (!any)
		return;

	if (useMainEncoders) {
		// Borrow OBS's main streaming encoders: one encode feeds both OBS's
		// own destination and ours. They exist only while OBS is streaming.
		obs_output_t *mainOutput = obs_frontend_get_streaming_output();
		if (!mainOutput)
			return;
		// Borrowed pointers; the output keeps the encoders alive while it streams.
		d_->venc = obs_output_get_video_encoder(mainOutput);
		d_->aenc = obs_output_get_audio_encoder(mainOutput, 0);
		obs_output_release(mainOutput);
		d_->ownEncoders = false;
		if (!d_->venc || !d_->aenc)
			return;
	} else {
		// One shared video + audio encoder bound to the global program output.
		obs_data_t *vsettings = obs_data_create();
		obs_data_set_int(vsettings, "bitrate", d_->videoBitrate);
		d_->venc = obs_video_encoder_create("obs_x264", "ohmydj-venc", vsettings, nullptr);
		obs_data_release(vsettings);
		obs_encoder_set_video(d_->venc, obs_get_video());

		obs_data_t *asettings = obs_data_create();
		obs_data_set_int(asettings, "bitrate", 160);
		d_->aenc = obs_audio_encoder_create("ffmpeg_aac", "ohmydj-aenc", asettings, 0, nullptr);
		obs_data_release(asettings);
		obs_encoder_set_audio(d_->aenc, obs_get_audio());
		d_->ownEncoders = true;
	}

	for (int i = 0; i < static_cast<int>(d_->targets.size()); ++i) {
		const StreamTarget &t = d_->targets[i];
		if (!t.enabled || t.url.empty())
			continue;

		const std::string id = std::to_string(i);

		obs_data_t *ssettings = obs_data_create();
		obs_data_set_string(ssettings, "server", t.url.c_str());
		obs_data_set_string(ssettings, "key", t.key.c_str());
		obs_service_t *service = obs_service_create("rtmp_custom", ("ohmydj-svc-" + id).c_str(),
							    ssettings, nullptr);
		obs_data_release(ssettings);

		obs_output_t *output =
			obs_output_create("rtmp_output", ("ohmydj-out-" + id).c_str(), nullptr, nullptr);
		obs_output_set_service(output, service);
		obs_output_set_video_encoder(output, d_->venc);
		obs_output_set_audio_encoder(output, d_->aenc, 0);

		auto *ctx = new OutputContext{this, i, output, service};
		signal_handler_t *sh = obs_output_get_signal_handler(output);
		signal_handler_connect(sh, "start", OnOutputStart, ctx);
		signal_handler_connect(sh, "stop", OnOutputStop, ctx);
		d_->outputs.push_back(ctx);

		emit targetStatusChanged(i, static_cast<int>(StreamStatus::Connecting));
		if (!obs_output_start(output))
			emit targetStatusChanged(i, static_cast<int>(StreamStatus::Error));
	}

	d_->running = true;
	emit runningChanged(true);
}

void MultistreamEngine::stop()
{
	if (!d_->running)
		return;

	for (OutputContext *ctx : d_->outputs) {
		signal_handler_t *sh = obs_output_get_signal_handler(ctx->output);
		signal_handler_disconnect(sh, "start", OnOutputStart, ctx);
		signal_handler_disconnect(sh, "stop", OnOutputStop, ctx);
		obs_output_stop(ctx->output);
		obs_output_release(ctx->output);
		obs_service_release(ctx->service);
		emit targetStatusChanged(ctx->index, static_cast<int>(StreamStatus::Stopped));
		delete ctx;
	}
	d_->outputs.clear();

	if (d_->ownEncoders) {
		if (d_->venc)
			obs_encoder_release(d_->venc);
		if (d_->aenc)
			obs_encoder_release(d_->aenc);
	}
	d_->venc = nullptr;
	d_->aenc = nullptr;

	d_->running = false;
	emit runningChanged(false);
}

StreamConfig LoadStreamConfig()
{
	StreamConfig config;
	std::string path = ConfigFilePath();
	if (path.empty())
		return config;

	obs_data_t *root = obs_data_create_from_json_file_safe(path.c_str(), "bak");
	if (!root)
		return config;

	if (obs_data_has_user_value(root, "bitrate"))
		config.videoBitrate = static_cast<int>(obs_data_get_int(root, "bitrate"));
	config.syncWithObs = obs_data_get_bool(root, "sync_with_obs");

	obs_data_array_t *arr = obs_data_get_array(root, "targets");
	if (arr) {
		const size_t n = obs_data_array_count(arr);
		for (size_t i = 0; i < n; ++i) {
			obs_data_t *o = obs_data_array_item(arr, i);
			StreamTarget t;
			t.name = obs_data_get_string(o, "name");
			t.url = obs_data_get_string(o, "url");
			t.key = obs_data_get_string(o, "key");
			t.enabled = obs_data_get_bool(o, "enabled");
			config.targets.push_back(std::move(t));
			obs_data_release(o);
		}
		obs_data_array_release(arr);
	}
	obs_data_release(root);
	return config;
}

void SaveStreamConfig(const StreamConfig &config)
{
	std::string path = ConfigFilePath();
	if (path.empty())
		return;

	obs_data_t *root = obs_data_create();
	obs_data_set_int(root, "bitrate", config.videoBitrate);
	obs_data_set_bool(root, "sync_with_obs", config.syncWithObs);
	obs_data_array_t *arr = obs_data_array_create();
	for (const StreamTarget &t : config.targets) {
		obs_data_t *o = obs_data_create();
		obs_data_set_string(o, "name", t.name.c_str());
		obs_data_set_string(o, "url", t.url.c_str());
		obs_data_set_string(o, "key", t.key.c_str());
		obs_data_set_bool(o, "enabled", t.enabled);
		obs_data_array_push_back(arr, o);
		obs_data_release(o);
	}
	obs_data_set_array(root, "targets", arr);
	obs_data_save_json_safe(root, path.c_str(), "tmp", "bak");
	obs_data_array_release(arr);
	obs_data_release(root);
}

} // namespace ohmydj
