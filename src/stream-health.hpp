#pragma once

#include <cstdint>

namespace ohmydj {

// Live health of a single streaming destination, sampled once a second.
struct StreamHealth {
	int bitrateKbps = 0;  // average upload bitrate since the previous sample
	int droppedPercent = 0; // frames dropped out of total, [0, 100]
};

// Average bitrate in kbps between two readings of an output's cumulative byte
// counter. Returns 0 when no time elapsed or the counter did not advance (so a
// counter reset never yields a negative or absurd value).
// Pure logic, no OBS/Qt dependencies, so it is unit-testable on its own.
int BitrateKbps(uint64_t prevBytes, uint64_t nowBytes, int elapsedMs);

// Percentage of frames dropped, [0, 100], rounded. Returns 0 when no frames
// have been sent yet. Pure logic, unit-testable on its own.
int DroppedPercent(int64_t droppedFrames, int64_t totalFrames);

} // namespace ohmydj
