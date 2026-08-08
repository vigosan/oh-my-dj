#include "rotation-step.hpp"

namespace ohmydj {

long long RandomizedSeconds(const RotationStep &step, long long roll)
{
	const long long min = step.seconds();
	if (step.amountMax <= step.amount)
		return min;
	const long long max = static_cast<long long>(step.amountMax) * UnitToSeconds(step.unit);
	const long long span = max - min + 1; // inclusive window
	return min + (roll % span + span) % span;
}

int ResolveNextIndex(const std::vector<RotationStep> &steps, int fromIndex)
{
	const int count = static_cast<int>(steps.size());
	if (count == 0)
		return -1;

	const bool validFrom = fromIndex >= 0 && fromIndex < count;

	if (validFrom) {
		const std::string &target = steps[fromIndex].onExpire;
		if (!target.empty()) {
			for (int i = 0; i < count; ++i) {
				if (steps[i].enabled && steps[i].scene == target)
					return i;
			}
			// Target is disabled or no longer in the flow: fall through to next.
		}
	}

	// Walk forward from the current position, wrapping, until an enabled step
	// turns up. Starting at 1 keeps a lone enabled step resolving onto itself
	// only after every other candidate has been rejected.
	const int from = validFrom ? fromIndex : count - 1;
	for (int hop = 1; hop <= count; ++hop) {
		const int candidate = (from + hop) % count;
		if (steps[candidate].enabled)
			return candidate;
	}
	return -1; // nothing enabled
}

int ResolveShuffleIndex(const std::vector<RotationStep> &steps, int current, int previous, int roll)
{
	const int count = static_cast<int>(steps.size());

	std::vector<int> enabled;
	enabled.reserve(count);
	for (int i = 0; i < count; ++i) {
		if (steps[i].enabled)
			enabled.push_back(i);
	}

	if (enabled.empty())
		return -1;
	if (enabled.size() == 1)
		return enabled.front();

	// Exclude the current step; exclude the previous one too, but only while
	// that still leaves something to pick (i.e. once there are >= 3 enabled).
	const bool excludePrevious = enabled.size() >= 3;

	std::vector<int> candidates;
	candidates.reserve(enabled.size());
	for (int i : enabled) {
		if (i == current)
			continue;
		if (excludePrevious && i == previous)
			continue;
		candidates.push_back(i);
	}

	if (candidates.empty())
		return current; // degenerate guard; should not happen given the rules above

	const int size = static_cast<int>(candidates.size());
	return candidates[((roll % size) + size) % size];
}

} // namespace ohmydj
