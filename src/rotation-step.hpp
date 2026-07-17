#pragma once

#include <string>
#include <vector>

#include "duration.hpp"

namespace ohmydj {

// One entry in the camera/scene rotation flow:
// "show `scene` for `amount` `unit`, then go to `onExpire`".
struct RotationStep {
	std::string scene;                       // OBS scene to display
	int amount = 30;                         // duration amount (>= 1); with amountMax set, the range minimum
	int amountMax = 0;                       // 0 or == amount => fixed duration; > amount => random in [amount, amountMax]
	DurationUnit unit = DurationUnit::Seconds;
	std::string onExpire;                    // empty => next step (loop); else jump to step with this scene
	std::string transition;                  // OBS transition name to use when switching INTO this scene; empty => OBS's current one

	long long seconds() const { return static_cast<long long>(amount) * UnitToSeconds(unit); }
};

// Duration (in seconds) for one pass of `step`: fixed steps return seconds();
// ranged steps map `roll` onto the inclusive [min, max] window in seconds, so
// both bounds are reachable and the result is deterministic for tests. `roll`
// is a non-negative integer supplied by the caller (the RNG in production).
// A max at or below the min behaves as fixed. Pure logic, no OBS/Qt/RNG.
long long RandomizedSeconds(const RotationStep &step, long long roll);

// Index of the step to activate once the step at `fromIndex` expires.
//   - empty onExpire  => next step, wrapping around (loop)
//   - set  onExpire   => first step whose scene matches; if none, falls back to next
//   - no steps        => -1
// Pure logic, no OBS/Qt dependencies, so it is unit-testable on its own.
int ResolveNextIndex(const std::vector<RotationStep> &steps, int fromIndex);

// Index of the next step in shuffle mode: a random pick that avoids `current`
// and — once there are >= 3 steps — also avoids `previous`, so the flow never
// stutters back and forth between the same two scenes. `roll` is a non-negative
// integer supplied by the caller (the RNG in production); it is mapped onto the
// surviving candidates, so any value is valid and the result is deterministic.
//   - count <= 1 => 0 (a single scene loops onto itself), or -1 if count == 0
// Pure logic, no OBS/Qt/RNG dependencies, so it is unit-testable on its own.
int ResolveShuffleIndex(int count, int current, int previous, int roll);

} // namespace ohmydj
