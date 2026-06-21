#include "duration.hpp"

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

} // namespace ohmydj
