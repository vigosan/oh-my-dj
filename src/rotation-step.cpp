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

} // namespace ohmydj
