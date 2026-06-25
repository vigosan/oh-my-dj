#include "stream-health.hpp"

namespace ohmydj {

int BitrateKbps(uint64_t prevBytes, uint64_t nowBytes, int elapsedMs)
{
	if (elapsedMs <= 0 || nowBytes <= prevBytes)
		return 0;
	const uint64_t bits = (nowBytes - prevBytes) * 8;
	// bits over elapsedMs ms => kbps == bits / elapsedMs.
	return static_cast<int>(bits / static_cast<uint64_t>(elapsedMs));
}

int DroppedPercent(int64_t droppedFrames, int64_t totalFrames)
{
	if (totalFrames <= 0 || droppedFrames <= 0)
		return 0;
	return static_cast<int>((droppedFrames * 100 + totalFrames / 2) / totalFrames);
}

} // namespace ohmydj
