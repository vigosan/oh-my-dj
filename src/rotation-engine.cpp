#include "rotation-engine.hpp"

#include <obs.h>
#include <obs-frontend-api.h>
#include <util/platform.h>

#include <string>

namespace ohmydj {

namespace {

std::string CurrentSceneName()
{
	obs_source_t *scene = obs_frontend_get_current_scene();
	if (!scene)
		return {};
	std::string name = obs_source_get_name(scene);
	obs_source_release(scene);
	return name;
}

void SetCurrentSceneByName(const std::string &name)
{
	obs_source_t *scene = obs_get_source_by_name(name.c_str());
	if (!scene)
		return;
	obs_frontend_set_current_scene(scene);
	obs_source_release(scene);
}

std::string ConfigFilePath()
{
	char *profile = obs_frontend_get_current_profile_path();
	if (!profile)
		return {};
	std::string path = profile;
	path += "/oh-my-dj.json";
	bfree(profile);
	return path;
}

int IndexOfScene(const std::vector<RotationStep> &steps, const std::string &scene)
{
	for (int i = 0; i < static_cast<int>(steps.size()); ++i) {
		if (steps[i].scene == scene)
			return i;
	}
	return -1;
}

} // namespace

RotationEngine::RotationEngine(QObject *parent) : QObject(parent)
{
	timer_.setSingleShot(true);
	connect(&timer_, &QTimer::timeout, this, &RotationEngine::onTimeout);
}

void RotationEngine::setSteps(std::vector<RotationStep> steps)
{
	steps_ = std::move(steps);
	if (active())
		evaluate();
}

void RotationEngine::setEnabled(bool enabled)
{
	if (enabled_ == enabled)
		return;
	enabled_ = enabled;
	if (active())
		evaluate();
	else
		park();
}

void RotationEngine::setStreaming(bool streaming)
{
	if (streaming_ == streaming)
		return;
	streaming_ = streaming;
	if (active())
		evaluate();
	else
		park();
}

void RotationEngine::onSceneChanged()
{
	if (active())
		evaluate();
}

void RotationEngine::evaluate()
{
	const std::string scene = CurrentSceneName();

	// Ignore the scene change we triggered ourselves: its timer is already armed.
	if (expectingSelfSwitch_ && scene == pendingScene_) {
		expectingSelfSwitch_ = false;
		return;
	}

	const int index = IndexOfScene(steps_, scene);
	if (index >= 0)
		arm(index); // we are already on this scene; just start its timer
	else
		park();
}

void RotationEngine::arm(int index)
{
	current_ = index;
	long long ms = steps_[index].seconds() * 1000;
	if (ms < 1000)
		ms = 1000;
	timer_.start(static_cast<int>(ms));
	emit stepChanged(index);
}

void RotationEngine::park()
{
	timer_.stop();
	if (current_ != -1) {
		current_ = -1;
		emit stepChanged(-1);
	}
}

void RotationEngine::advance()
{
	const int next = ResolveNextIndex(steps_, current_);
	if (next < 0) {
		park();
		return;
	}
	current_ = next;
	pendingScene_ = steps_[next].scene;
	expectingSelfSwitch_ = true;
	SetCurrentSceneByName(pendingScene_);

	long long ms = steps_[next].seconds() * 1000;
	if (ms < 1000)
		ms = 1000;
	timer_.start(static_cast<int>(ms));
	emit stepChanged(next);
}

void RotationEngine::onTimeout()
{
	if (!active() || current_ < 0)
		return;
	advance();
}

RotationConfig LoadRotationConfig()
{
	RotationConfig config;
	std::string path = ConfigFilePath();
	if (path.empty())
		return config;

	obs_data_t *root = obs_data_create_from_json_file_safe(path.c_str(), "bak");
	if (!root)
		return config;

	config.enabled = obs_data_get_bool(root, "enabled");

	obs_data_array_t *arr = obs_data_get_array(root, "steps");
	if (arr) {
		const size_t n = obs_data_array_count(arr);
		for (size_t i = 0; i < n; ++i) {
			obs_data_t *o = obs_data_array_item(arr, i);
			RotationStep step;
			step.scene = obs_data_get_string(o, "scene");
			step.amount = static_cast<int>(obs_data_get_int(o, "amount"));
			if (step.amount < 1)
				step.amount = 1;
			step.unit = UnitFromInt(static_cast<int>(obs_data_get_int(o, "unit")));
			step.onExpire = obs_data_get_string(o, "on_expire");
			config.steps.push_back(std::move(step));
			obs_data_release(o);
		}
		obs_data_array_release(arr);
	}
	obs_data_release(root);
	return config;
}

void SaveRotationConfig(const RotationConfig &config)
{
	std::string path = ConfigFilePath();
	if (path.empty())
		return;

	obs_data_t *root = obs_data_create();
	obs_data_set_bool(root, "enabled", config.enabled);
	obs_data_array_t *arr = obs_data_array_create();
	for (const RotationStep &step : config.steps) {
		obs_data_t *o = obs_data_create();
		obs_data_set_string(o, "scene", step.scene.c_str());
		obs_data_set_int(o, "amount", step.amount);
		obs_data_set_int(o, "unit", static_cast<int>(step.unit));
		obs_data_set_string(o, "on_expire", step.onExpire.c_str());
		obs_data_array_push_back(arr, o);
		obs_data_release(o);
	}
	obs_data_set_array(root, "steps", arr);
	obs_data_save_json_safe(root, path.c_str(), "tmp", "bak");
	obs_data_array_release(arr);
	obs_data_release(root);
}

} // namespace ohmydj
