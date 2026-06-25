// Pure-logic tests for the stream-health sampling math. No Qt / no libobs.

#include <cstdio>

#include "stream-health.hpp"

using namespace ohmydj;

static int g_failures = 0;

#define CHECK(expr)                                                          \
	do {                                                                 \
		if (!(expr)) {                                               \
			std::printf("  FAIL: %s (line %d)\n", #expr, __LINE__); \
			++g_failures;                                       \
		}                                                           \
	} while (0)

static void test_bitrate_basic()
{
	std::printf("bitrate from byte delta over time\n");
	// 750000 bytes in 1s = 6_000_000 bits = 6000 kbps.
	CHECK(BitrateKbps(0, 750000, 1000) == 6000);
	// Same volume over 2s halves the rate.
	CHECK(BitrateKbps(0, 750000, 2000) == 3000);
	// Only the delta counts, not the absolute counter.
	CHECK(BitrateKbps(1000000, 1750000, 1000) == 6000);
}

static void test_bitrate_guards()
{
	std::printf("bitrate guards against zero time and counter resets\n");
	CHECK(BitrateKbps(0, 750000, 0) == 0);     // no elapsed time
	CHECK(BitrateKbps(0, 750000, -5) == 0);    // bogus negative interval
	CHECK(BitrateKbps(2000000, 1000, 1000) == 0); // counter went backwards (reset)
	CHECK(BitrateKbps(500000, 500000, 1000) == 0); // no new bytes => idle
}

static void test_dropped_percent()
{
	std::printf("dropped-frame percentage\n");
	CHECK(DroppedPercent(0, 0) == 0);     // nothing sent yet
	CHECK(DroppedPercent(0, 1000) == 0);  // healthy
	CHECK(DroppedPercent(50, 1000) == 5);
	CHECK(DroppedPercent(125, 1000) == 13); // rounds to nearest
	CHECK(DroppedPercent(1000, 1000) == 100);
	CHECK(DroppedPercent(5, 0) == 0); // dropped but no total: avoid divide-by-zero
}

int main()
{
	test_bitrate_basic();
	test_bitrate_guards();
	test_dropped_percent();

	if (g_failures == 0) {
		std::printf("\nAll health tests passed.\n");
		return 0;
	}
	std::printf("\n%d check(s) failed.\n", g_failures);
	return 1;
}
