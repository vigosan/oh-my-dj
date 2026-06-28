#pragma once

#include <string>

namespace ohmydj {

// One streaming destination (Twitch, YouTube, ...). The same OBS output is
// pushed to every enabled target at once.
//
// A known platform drives OBS's own "rtmp_common" service (so it inherits OBS's
// per-platform server list and regional "auto" ingest selection): `service` is
// the OBS service name and `server` its server id ("auto"). A custom target
// leaves both empty and streams to a free-form `url` via "rtmp_custom".
struct StreamTarget {
	std::string name;    // friendly label, e.g. "Twitch"
	std::string service; // OBS rtmp_common service name; empty => custom
	std::string server;  // OBS server id, e.g. "auto"; empty => custom
	std::string url;     // RTMP server URL (custom only)
	std::string key;     // stream key
	bool enabled = true;
};

// Live status of a single destination, surfaced in the UI.
enum class StreamStatus { Idle = 0, Connecting = 1, Live = 2, Error = 3, Stopped = 4 };

} // namespace ohmydj
