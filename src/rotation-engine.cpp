#include "rotation-engine.hpp"

#include <obs.h>
#include <obs-frontend-api.h>
#include <util/platform.h>

#include <string>

namespace ohmydj {

namespace {

void SetCurrentSceneByName(const std::string &name)
{
	// A scene is a regular source; the frontend call expects that source.
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

} // namespace

RotationEngine::RotationEngine(QObject *parent) : QObject(parent)
{
	timer_.setSingleShot(true);
	connect(&timer_, &QTimer::timeout, this, &RotationEngine::onTimeout);
}

void RotationEngine::setSteps(std::vector<RotationStep> steps)
{
	steps_ = std::move(steps);
	if (running_ && current_ >= static_cast<int>(steps_.size()))
		stop();
}

void RotationEngine::start()
{
	if (running_ || steps_.empty())
		return;
	running_ = true;
	emit runningChanged(true);
	activate(0);
}

void RotationEngine::stop()
{
	if (!running_)
		return;
	timer_.stop();
	running_ = false;
	current_ = -1;
	emit runningChanged(false);
	emit stepActivated(-1);
}

void RotationEngine::activate(int index)
{
	if (index < 0 || index >= static_cast<int>(steps_.size())) {
		stop();
		return;
	}
	current_ = index;
	const RotationStep &step = steps_[index];
	SetCurrentSceneByName(step.scene);
	emit stepActivated(index);

	long long ms = step.seconds() * 1000;
	if (ms < 1000)
		ms = 1000; // never busy-loop on a misconfigured step
	timer_.start(static_cast<int>(ms));
}

void RotationEngine::onTimeout()
{
	if (!running_)
		return;
	activate(ResolveNextIndex(steps_, current_));
}

std::vector<RotationStep> LoadRotationSteps()
{
	std::vector<RotationStep> steps;
	std::string path = ConfigFilePath();
	if (path.empty())
		return steps;

	obs_data_t *root = obs_data_create_from_json_file_safe(path.c_str(), "bak");
	if (!root)
		return steps;

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
			steps.push_back(std::move(step));
			obs_data_release(o);
		}
		obs_data_array_release(arr);
	}
	obs_data_release(root);
	return steps;
}

void SaveRotationSteps(const std::vector<RotationStep> &steps)
{
	std::string path = ConfigFilePath();
	if (path.empty())
		return;

	obs_data_t *root = obs_data_create();
	obs_data_array_t *arr = obs_data_array_create();
	for (const RotationStep &step : steps) {
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
