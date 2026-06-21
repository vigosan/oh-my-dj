#pragma once

#include <string>

namespace ohmydj {

enum class DurationUnit { Seconds = 0, Minutes = 1, Hours = 2 };

// Multiplier to convert a value expressed in `unit` into seconds.
int UnitToSeconds(DurationUnit unit);

// Clamp an arbitrary integer back into a valid DurationUnit (for loaded data).
DurationUnit UnitFromInt(int value);

// Human-readable clock: "m:ss", or "h:mm:ss" once it reaches an hour.
std::string FormatClock(int seconds);

} // namespace ohmydj
