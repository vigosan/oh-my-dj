#include "duration.hpp"

#include <cstdio>

namespace ohmydj {

int UnitToSeconds(DurationUnit unit)
{
	switch (unit) {
	case DurationUnit::Hours:
		return 3600;
	case DurationUnit::Minutes:
		return 60;
	case DurationUnit::Seconds:
	default:
		return 1;
	}
}

DurationUnit UnitFromInt(int value)
{
	switch (value) {
	case 1:
		return DurationUnit::Minutes;
	case 2:
		return DurationUnit::Hours;
	default:
		return DurationUnit::Seconds;
	}
}

std::string FormatClock(int seconds)
{
	if (seconds < 0)
		seconds = 0;
	const int h = seconds / 3600;
	const int m = (seconds % 3600) / 60;
	const int s = seconds % 60;
	char buf[16];
	if (h > 0)
		std::snprintf(buf, sizeof(buf), "%d:%02d:%02d", h, m, s);
	else
		std::snprintf(buf, sizeof(buf), "%d:%02d", m, s);
	return buf;
}

} // namespace ohmydj
