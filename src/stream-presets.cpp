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

} // namespace ohmydj
