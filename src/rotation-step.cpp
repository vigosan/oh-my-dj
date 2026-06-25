#include "rotation-step.hpp"

namespace ohmydj {

int ResolveNextIndex(const std::vector<RotationStep> &steps, int fromIndex)
{
	const int count = static_cast<int>(steps.size());
	if (count == 0)
		return -1;
	if (fromIndex < 0 || fromIndex >= count)
		return 0;

	const std::string &target = steps[fromIndex].onExpire;
	if (!target.empty()) {
		for (int i = 0; i < count; ++i) {
			if (steps[i].scene == target)
				return i;
		}
		// Target scene is no longer part of the flow: fall through to next.
	}
	return (fromIndex + 1) % count; // wrap => loop
}

int ResolveShuffleIndex(int count, int current, int previous, int roll)
{
	if (count <= 0)
		return -1;
	if (count == 1)
		return 0;

	// Exclude the current step; exclude the previous one too, but only while
	// that still leaves something to pick (i.e. once there are >= 3 steps).
	const bool excludePrevious = count >= 3;

	std::vector<int> candidates;
	candidates.reserve(count);
	for (int i = 0; i < count; ++i) {
		if (i == current)
			continue;
		if (excludePrevious && i == previous)
			continue;
		candidates.push_back(i);
	}

	if (candidates.empty())
		return current; // degenerate guard; should not happen given the rules above

	const int r = ((roll % static_cast<int>(candidates.size())) + static_cast<int>(candidates.size())) %
		      static_cast<int>(candidates.size());
	return candidates[r];
}

} // namespace ohmydj
