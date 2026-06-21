#pragma once

#include <string>
#include <vector>

namespace ohmydj {

// A known streaming platform and its RTMP ingest URL, so a DJ picks the
// platform instead of hunting for the server address.
struct StreamPreset {
	std::string label;
	std::string url; // empty => the user types their own (Custom)
};

const std::vector<StreamPreset> &StreamPresets();

// RTMP URL for a platform label, or "" if unknown / custom.
std::string PresetUrl(const std::string &label);

// True when `url` is empty or matches a preset URL, i.e. it is safe to
// auto-overwrite (we never clobber a URL the user typed by hand).
bool IsPresetUrl(const std::string &url);

} // namespace ohmydj
