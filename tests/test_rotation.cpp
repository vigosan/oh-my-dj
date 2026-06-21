// Pure-logic tests for the scene-rotation flow. No Qt / no libobs, so they build
// and run anywhere with a C++17 compiler.

#include <cstdio>
#include <vector>

#include "duration.hpp"
#include "rotation-step.hpp"

using namespace ohmydj;

static int g_failures = 0;

#define CHECK(expr)                                                          \
	do {                                                                 \
		if (!(expr)) {                                               \
			std::printf("  FAIL: %s (line %d)\n", #expr, __LINE__); \
			++g_failures;                                       \
		}                                                           \
	} while (0)

static RotationStep step(const char *scene, int amount, DurationUnit unit, const char *onExpire = "")
{
	RotationStep s;
	s.scene = scene;
	s.amount = amount;
	s.unit = unit;
	s.onExpire = onExpire;
	return s;
}

static void test_unit_conversion()
{
	std::printf("unit conversion\n");
	CHECK(UnitToSeconds(DurationUnit::Seconds) == 1);
	CHECK(UnitToSeconds(DurationUnit::Minutes) == 60);
	CHECK(UnitToSeconds(DurationUnit::Hours) == 3600);

	CHECK(UnitFromInt(0) == DurationUnit::Seconds);
	CHECK(UnitFromInt(1) == DurationUnit::Minutes);
	CHECK(UnitFromInt(2) == DurationUnit::Hours);
	CHECK(UnitFromInt(99) == DurationUnit::Seconds); // out-of-range clamps to a safe default
}

static void test_format_clock()
{
	std::printf("clock formatting for the countdown\n");
	CHECK(FormatClock(0) == "0:00");
	CHECK(FormatClock(9) == "0:09");
	CHECK(FormatClock(75) == "1:15");
	CHECK(FormatClock(600) == "10:00");
	CHECK(FormatClock(3600) == "1:00:00");
	CHECK(FormatClock(3661) == "1:01:01");
	CHECK(FormatClock(-5) == "0:00"); // never show a negative countdown
}

static void test_step_seconds()
{
	std::printf("step duration in seconds\n");
	CHECK(step("CAM", 30, DurationUnit::Seconds).seconds() == 30);
	CHECK(step("CAM", 2, DurationUnit::Minutes).seconds() == 120);
	CHECK(step("CAM", 1, DurationUnit::Hours).seconds() == 3600);
}

static void test_resolve_loop()
{
	std::printf("rotation loops to the top by default\n");
	std::vector<RotationStep> flow = {
		step("CAM1", 2, DurationUnit::Minutes),
		step("CAM2", 30, DurationUnit::Seconds),
		step("CAM3", 1, DurationUnit::Minutes),
	};
	CHECK(ResolveNextIndex(flow, 0) == 1);
	CHECK(ResolveNextIndex(flow, 1) == 2);
	CHECK(ResolveNextIndex(flow, 2) == 0); // wrap => loop, the DJ's repeating cycle
}

static void test_resolve_jump()
{
	std::printf("a step can jump to a specific scene\n");
	std::vector<RotationStep> flow = {
		step("CAM1", 10, DurationUnit::Seconds, "CAM3"),
		step("CAM2", 10, DurationUnit::Seconds),
		step("CAM3", 10, DurationUnit::Seconds),
	};
	CHECK(ResolveNextIndex(flow, 0) == 2); // jumps straight to CAM3, skipping CAM2
}

static void test_resolve_jump_fallback()
{
	std::printf("a jump to a missing scene falls back to next\n");
	std::vector<RotationStep> flow = {
		step("CAM1", 10, DurationUnit::Seconds, "GHOST"),
		step("CAM2", 10, DurationUnit::Seconds),
	};
	CHECK(ResolveNextIndex(flow, 0) == 1);
}

static void test_resolve_edge_cases()
{
	std::printf("edge cases\n");
	std::vector<RotationStep> empty;
	CHECK(ResolveNextIndex(empty, 0) == -1); // nothing to play

	std::vector<RotationStep> one = {step("CAM1", 5, DurationUnit::Seconds)};
	CHECK(ResolveNextIndex(one, 0) == 0); // single scene loops onto itself

	std::vector<RotationStep> flow = {
		step("CAM1", 5, DurationUnit::Seconds),
		step("CAM2", 5, DurationUnit::Seconds),
	};
	CHECK(ResolveNextIndex(flow, 99) == 0);  // out-of-range index recovers to the start
	CHECK(ResolveNextIndex(flow, -1) == 0);
}

static void test_resolve_duplicate_target()
{
	std::printf("jump resolves to the first matching scene\n");
	std::vector<RotationStep> flow = {
		step("INTRO", 5, DurationUnit::Seconds, "CAM"),
		step("CAM", 5, DurationUnit::Seconds),
		step("CAM", 5, DurationUnit::Seconds),
	};
	CHECK(ResolveNextIndex(flow, 0) == 1);
}

static void test_resolve_repeated_scene_cycle()
{
	std::printf("a scene can recur in the cycle: Main1, Main2, Main1, Main3, loop\n");
	std::vector<RotationStep> flow = {
		step("Main 1", 1, DurationUnit::Minutes),
		step("Main 2", 8, DurationUnit::Minutes),
		step("Main 1", 1, DurationUnit::Minutes),
		step("Main 3", 1, DurationUnit::Minutes),
	};
	// Advancement is positional, so the second "Main 1" is its own step and
	// leads to Main 3 rather than collapsing back onto the first occurrence.
	CHECK(ResolveNextIndex(flow, 0) == 1); // Main 1 -> Main 2
	CHECK(ResolveNextIndex(flow, 1) == 2); // Main 2 -> Main 1 (again)
	CHECK(ResolveNextIndex(flow, 2) == 3); // Main 1 -> Main 3
	CHECK(ResolveNextIndex(flow, 3) == 0); // Main 3 -> Main 1, cycle restarts
}

int main()
{
	test_unit_conversion();
	test_format_clock();
	test_step_seconds();
	test_resolve_loop();
	test_resolve_jump();
	test_resolve_jump_fallback();
	test_resolve_edge_cases();
	test_resolve_duplicate_target();
	test_resolve_repeated_scene_cycle();

	if (g_failures == 0) {
		std::printf("\nAll rotation tests passed.\n");
		return 0;
	}
	std::printf("\n%d check(s) failed.\n", g_failures);
	return 1;
}
