#pragma once

#include <string>
#include <vector>

#include "duration.hpp"

namespace ohmydj {

// One entry in the camera/scene rotation flow:
// "show `scene` for `amount` `unit`, then go to `onExpire`".
struct RotationStep {
	std::string scene;                       // OBS scene to display
	int amount = 30;                         // duration amount (>= 1)
	DurationUnit unit = DurationUnit::Seconds;
	std::string onExpire;                    // empty => next step (loop); else jump to step with this scene

	long long seconds() const { return static_cast<long long>(amount) * UnitToSeconds(unit); }
};

// Index of the step to activate once the step at `fromIndex` expires.
//   - empty onExpire  => next step, wrapping around (loop)
//   - set  onExpire   => first step whose scene matches; if none, falls back to next
//   - no steps        => -1
// Pure logic, no OBS/Qt dependencies, so it is unit-testable on its own.
int ResolveNextIndex(const std::vector<RotationStep> &steps, int fromIndex);

} // namespace ohmydj
