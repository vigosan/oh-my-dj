#pragma once

#include <string>

namespace ohmydj {

// One streaming destination (Twitch, YouTube, ...). The same OBS output is
// pushed to every enabled target at once.
struct StreamTarget {
	std::string name; // friendly label, e.g. "Twitch"
	std::string url;  // RTMP server URL
	std::string key;  // stream key
	bool enabled = true;
};

// Live status of a single destination, surfaced in the UI.
enum class StreamStatus { Idle = 0, Connecting = 1, Live = 2, Error = 3, Stopped = 4 };

} // namespace ohmydj
