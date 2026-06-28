#include "multistream-engine.hpp"

#include <obs.h>
#include <obs-frontend-api.h>
#include <util/platform.h>

#include <QTimer>

#include <string>

#include "stream-health.hpp"
#include "stream-presets.hpp"

namespace ohmydj {

namespace {

constexpr int kReconnectDelayMs = 5000;
constexpr int kHealthIntervalMs = 1000;

// A target is ready to go live when it is on and has somewhere to send to: a
// known platform (OBS resolves its server) or a custom URL.
bool TargetReady(const StreamTarget &t)
{
	if (!t.enabled)
		return false;
	return t.service.empty() ? !t.url.empty() : true;
}

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
	uint64_t lastBytes = 0; // cumulative bytes at the previous health sample
};

struct MultistreamEngine::Impl {
	std::vector<StreamTarget> targets;
	int videoBitrate = 6000;
	bool running = false;
	bool ownEncoders = false; // true => we created venc/aenc and must release them

	obs_encoder_t *venc = nullptr;
	obs_encoder_t *aenc = nullptr;
	std::vector<OutputContext *> outputs;
	QTimer healthTimer;
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
	if (code == OBS_OUTPUT_SUCCESS) {
		EmitStatus(ctx, StreamStatus::Stopped);
		return;
	}
	// Unexpected drop (network blip, server reset): flag it and retry. Our own
	// stop() disconnects this handler first, so this only fires on real drops.
	EmitStatus(ctx, StreamStatus::Error);
	MultistreamEngine *engine = ctx->engine;
	const int index = ctx->index;
	QMetaObject::invokeMethod(
		engine, [engine, index]() { engine->reconnect(index); }, Qt::QueuedConnection);
}

MultistreamEngine::MultistreamEngine(QObject *parent) : QObject(parent), d_(std::make_unique<Impl>())
{
	d_->healthTimer.setInterval(kHealthIntervalMs);
	connect(&d_->healthTimer, &QTimer::timeout, this, &MultistreamEngine::sampleHealth);
}

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
		if (TargetReady(t)) {
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
		if (!TargetReady(t))
			continue;

		const std::string id = std::to_string(i);

		obs_data_t *ssettings = obs_data_create();
		obs_data_set_string(ssettings, "key", t.key.c_str());
		const char *serviceType;
		if (t.service.empty()) {
			// Custom destination: stream straight to the user's URL.
			obs_data_set_string(ssettings, "server", t.url.c_str());
			serviceType = "rtmp_custom";
		} else {
			// Known platform: let OBS pick the server (regional "auto").
			obs_data_set_string(ssettings, "service", t.service.c_str());
			obs_data_set_string(ssettings, "server", t.server.c_str());
			serviceType = "rtmp_common";
		}
		obs_service_t *service = obs_service_create(serviceType, ("ohmydj-svc-" + id).c_str(),
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
	d_->healthTimer.start();
	emit runningChanged(true);
}

void MultistreamEngine::stop()
{
	if (!d_->running)
		return;

	d_->healthTimer.stop();

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

void MultistreamEngine::reconnect(int index)
{
	if (!d_->running)
		return;

	emit targetStatusChanged(index, static_cast<int>(StreamStatus::Connecting));
	QTimer::singleShot(kReconnectDelayMs, this, [this, index]() {
		if (!d_->running)
			return;
		for (OutputContext *ctx : d_->outputs) {
			if (ctx->index != index)
				continue;
			if (!obs_output_start(ctx->output))
				emit targetStatusChanged(index, static_cast<int>(StreamStatus::Error));
			return;
		}
	});
}

void MultistreamEngine::sampleHealth()
{
	for (OutputContext *ctx : d_->outputs) {
		if (!ctx->output)
			continue;
		const uint64_t bytes = obs_output_get_total_bytes(ctx->output);
		const int kbps = BitrateKbps(ctx->lastBytes, bytes, kHealthIntervalMs);
		ctx->lastBytes = bytes;

		const int total = obs_output_get_total_frames(ctx->output);
		const int dropped = obs_output_get_frames_dropped(ctx->output);
		const int pct = DroppedPercent(dropped, total);

		emit targetHealthChanged(ctx->index, kbps, pct);
	}
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
			if (obs_data_has_user_value(o, "service")) {
				// New format: stored explicitly.
				t.service = obs_data_get_string(o, "service");
				t.server = obs_data_get_string(o, "server");
			} else {
				// Legacy format: derive the OBS service from the
				// label/URL so known platforms gain regional "auto".
				const MigratedTarget m = MigrateLegacyTarget(t.name, t.url);
				t.service = m.service;
				t.server = m.server;
			}
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
		obs_data_set_string(o, "service", t.service.c_str());
		obs_data_set_string(o, "server", t.server.c_str());
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
