#include "stream-presets.hpp"

namespace ohmydj {

const std::vector<StreamPreset> &StreamPresets()
{
	static const std::vector<StreamPreset> presets = {
		{"Twitch", "rtmp://live.twitch.tv/app"},
		{"YouTube", "rtmp://a.rtmp.youtube.com/live2"},
		{"Facebook Live", "rtmps://live-api-s.facebook.com:443/rtmp/"},
		{"Custom", ""},
	};
	return presets;
}

std::string PresetUrl(const std::string &label)
{
	for (const StreamPreset &preset : StreamPresets()) {
		if (preset.label == label)
			return preset.url;
	}
	return "";
}

bool IsPresetUrl(const std::string &url)
{
	if (url.empty())
		return true;
	for (const StreamPreset &preset : StreamPresets()) {
		if (!preset.url.empty() && preset.url == url)
			return true;
	}
	return false;
}

namespace {

// Friendly label -> exact OBS rtmp_common service name (from services.json).
struct PlatformMap {
	const char *label;
	const char *service;
};

const std::vector<PlatformMap> &PlatformMaps()
{
	static const std::vector<PlatformMap> maps = {
		{"Twitch", "Twitch"},
		{"YouTube", "YouTube - RTMPS"},
		{"Facebook Live", "Facebook Live"},
	};
	return maps;
}

} // namespace

std::string PlatformServiceName(const std::string &label)
{
	for (const PlatformMap &m : PlatformMaps()) {
		if (label == m.label)
			return m.service;
	}
	return "";
}

MigratedTarget MigrateLegacyTarget(const std::string &name, const std::string &url)
{
	// Prefer the label; fall back to recognising the legacy preset URL when the
	// user renamed the row but kept the platform's server address.
	std::string service = PlatformServiceName(name);
	if (service.empty() && !url.empty()) {
		for (const StreamPreset &preset : StreamPresets()) {
			if (!preset.url.empty() && preset.url == url) {
				service = PlatformServiceName(preset.label);
				break;
			}
		}
	}

	MigratedTarget out;
	if (!service.empty()) {
		out.service = service;
		out.server = "auto";
		out.isCustom = false;
	}
	return out;
}

} // namespace ohmydj
