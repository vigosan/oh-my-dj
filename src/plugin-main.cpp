#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QMainWindow>
#include <QTabWidget>

#include "multistream-dock.hpp"
#include "rotation-dock.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("oh-my-dj", "en-US")
OBS_MODULE_AUTHOR("Vicent")

#ifndef PLUGIN_VERSION
#define PLUGIN_VERSION "0.0.0"
#endif

namespace {

ohmydj::RotationDock *g_rotation = nullptr;
ohmydj::MultistreamDock *g_multistream = nullptr;

void OnFrontendEvent(enum obs_frontend_event event, void *)
{
	switch (event) {
	case OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED:
	case OBS_FRONTEND_EVENT_FINISHED_LOADING:
		if (g_rotation)
			g_rotation->refreshScenes();
		break;
	case OBS_FRONTEND_EVENT_EXIT:
		if (g_rotation)
			g_rotation->persist();
		if (g_multistream)
			g_multistream->persist();
		break;
	default:
		break;
	}
}

} // namespace

bool obs_module_load(void)
{
	auto *main = static_cast<QMainWindow *>(obs_frontend_get_main_window());

	g_rotation = new ohmydj::RotationDock();
	g_multistream = new ohmydj::MultistreamDock();

	auto *tabs = new QTabWidget(main);
	tabs->setObjectName("oh-my-dj-dock");
	tabs->addTab(g_rotation, obs_module_text("OhMyDj.Tab.Cameras"));
	tabs->addTab(g_multistream, obs_module_text("OhMyDj.Tab.Streaming"));

	if (!obs_frontend_add_dock_by_id("oh-my-dj-dock", obs_module_text("OhMyDj.Dock.Title"), tabs)) {
		delete tabs;
		g_rotation = nullptr;
		g_multistream = nullptr;
		return false;
	}

	obs_frontend_add_event_callback(OnFrontendEvent, nullptr);
	blog(LOG_INFO, "[oh-my-dj] loaded version %s", PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_frontend_remove_event_callback(OnFrontendEvent, nullptr);
	blog(LOG_INFO, "[oh-my-dj] unloaded");
}

MODULE_EXPORT const char *obs_module_description(void)
{
	return "Oh My DJ - simple scene rotation and multistream for DJs";
}
