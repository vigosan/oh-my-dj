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

// --- OBS rtmp_common migration -------------------------------------------
// The dock now drives OBS's own "rtmp_common" service so the DJ inherits its
// per-platform server list. These map our friendly label to the exact service
// name OBS expects in services.json (e.g. "YouTube" -> "YouTube - RTMPS").

// OBS service name for a platform label, or "" if the label is custom/unknown.
std::string PlatformServiceName(const std::string &label);

// Result of migrating a legacy {name, url} stream target to the rtmp_common model.
struct MigratedTarget {
	std::string service;  // OBS service name, empty for custom
	std::string server;   // OBS server id ("auto" for known platforms), empty for custom
	bool isCustom = true; // true => keep using rtmp_custom with a free-form URL
};

// Migrates a legacy target. A label/url that matches a known platform yields its
// OBS service with server="auto"; anything else stays custom (free-form URL).
// Pure logic, no OBS/Qt, so it is unit-testable on its own.
MigratedTarget MigrateLegacyTarget(const std::string &name, const std::string &url);

} // namespace ohmydj
