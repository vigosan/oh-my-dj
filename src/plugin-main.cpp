#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QDialog>
#include <QMainWindow>
#include <QMetaObject>
#include <QPointer>
#include <QString>
#include <QTabWidget>
#include <QVBoxLayout>

#include "about-tab.hpp"
#include "multistream-dock.hpp"
#include "overview-dock.hpp"
#include "rotation-dock.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("oh-my-dj", "en-US")
OBS_MODULE_AUTHOR("Vicent")

#ifndef PLUGIN_VERSION
#define PLUGIN_VERSION "0.0.0"
#endif

namespace {

// QPointers auto-null when Qt destroys the widget. The settings dialog is
// parented to the OBS main window and the docks live inside it, so OBS tears
// them down (when the main window dies) before the module is unloaded. Tracking
// them with QPointer keeps every null check honest and avoids a double free.
QPointer<ohmydj::RotationDock> g_rotation;
QPointer<ohmydj::MultistreamDock> g_multistream;
QPointer<QDialog> g_settings;

void OnFrontendEvent(enum obs_frontend_event event, void *)
{
	switch (event) {
	case OBS_FRONTEND_EVENT_SCENE_CHANGED:
		if (g_rotation)
			g_rotation->onSceneChanged();
		break;
	case OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED:
		if (g_rotation)
			g_rotation->refreshScenes();
		break;
	case OBS_FRONTEND_EVENT_FINISHED_LOADING:
		if (g_rotation) {
			g_rotation->refreshScenes();
			g_rotation->setObsStreaming(obs_frontend_streaming_active());
			g_rotation->onSceneChanged();
		}
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STARTED:
		if (g_rotation)
			g_rotation->setObsStreaming(true);
		if (g_multistream)
			g_multistream->onObsStreamingStarted();
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STOPPING:
		if (g_rotation)
			g_rotation->setObsStreaming(false);
		if (g_multistream)
			g_multistream->onObsStreamingStopping();
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

void OnSourceRename(void *, calldata_t *cd)
{
	auto *source = static_cast<obs_source_t *>(calldata_ptr(cd, "source"));
	if (!source || obs_source_get_type(source) != OBS_SOURCE_TYPE_SCENE)
		return;

	const QString from = QString::fromUtf8(calldata_string(cd, "prev_name"));
	const QString to = QString::fromUtf8(calldata_string(cd, "new_name"));

	// libobs raises this on a non-GUI thread; hop to the dock's thread before
	// touching any widgets.
	ohmydj::RotationDock *dock = g_rotation;
	if (!dock)
		return;
	QMetaObject::invokeMethod(
		dock,
		[from, to]() {
			if (g_rotation)
				g_rotation->renameScene(from, to);
		},
		Qt::QueuedConnection);
}

} // namespace

bool obs_module_load(void)
{
	auto *main = static_cast<QMainWindow *>(obs_frontend_get_main_window());

	g_rotation = new ohmydj::RotationDock();
	g_multistream = new ohmydj::MultistreamDock();

	auto *tabs = new QTabWidget();
	tabs->setObjectName("oh-my-dj-tabs");
	tabs->addTab(g_rotation, obs_module_text("OhMyDj.Tab.Cameras"));
	tabs->addTab(g_multistream, obs_module_text("OhMyDj.Tab.Streaming"));
	tabs->addTab(ohmydj::CreateAboutTab(), obs_module_text("OhMyDj.Tab.About"));

	// Settings live in a floating window (opened from the overview), not a dock,
	// so the always-on overview is what the DJ anchors in their layout.
	g_settings = new QDialog(main);
	g_settings->setWindowTitle(obs_module_text("OhMyDj.Dock.Title"));
	g_settings->resize(820, 480);
	auto *settingsLayout = new QVBoxLayout(g_settings);
	settingsLayout->setContentsMargins(0, 0, 0, 0);
	settingsLayout->addWidget(tabs);

	auto *overview = new ohmydj::OverviewDock(g_rotation, g_multistream, g_settings);

	if (!obs_frontend_add_dock_by_id("oh-my-dj-overview", obs_module_text("OhMyDj.Overview.Title"),
					 overview)) {
		delete overview;
		delete g_settings;
		g_settings = nullptr;
		g_rotation = nullptr;
		g_multistream = nullptr;
		return false;
	}

	obs_frontend_add_event_callback(OnFrontendEvent, nullptr);
	signal_handler_connect(obs_get_signal_handler(), "source_rename", OnSourceRename, nullptr);
	blog(LOG_INFO, "[oh-my-dj] loaded version %s", PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_frontend_remove_event_callback(OnFrontendEvent, nullptr);
	signal_handler_disconnect(obs_get_signal_handler(), "source_rename", OnSourceRename, nullptr);
	// Only delete it if Qt hasn't already (OBS usually destroys the main window,
	// and with it this dialog and both docks, before unloading the module).
	if (g_settings)
		delete g_settings; // also destroys the config tabs and both docks
	blog(LOG_INFO, "[oh-my-dj] unloaded");
}

MODULE_EXPORT const char *obs_module_description(void)
{
	return "Oh My DJ - simple scene rotation and multistream for DJs";
}
