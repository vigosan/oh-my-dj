// Pure-logic tests for the rotation "Suggest" assistant. No Qt / no libobs.

#include <cstdio>

#include "rotation-plan.hpp"

using namespace ohmydj;

static int g_failures = 0;

#define CHECK(expr)                                                          \
	do {                                                                 \
		if (!(expr)) {                                               \
			std::printf("  FAIL: %s (line %d)\n", #expr, __LINE__); \
			++g_failures;                                       \
		}                                                           \
	} while (0)

static void test_interleaves_main_before_each_secondary()
{
	std::printf("plan re-shows the main scene before every secondary\n");
	PlanInput in;
	in.main = "Top Cam";
	in.others = {"Left Cam", "Right Cam"};
	in.mainSeconds = 420; // 7 min
	in.otherSeconds = 45;

	const auto plan = BuildRotationPlan(in);
	CHECK(plan.size() == 4);
	CHECK(plan[0].scene == "Top Cam");
	CHECK(plan[1].scene == "Left Cam");
	CHECK(plan[2].scene == "Top Cam");
	CHECK(plan[3].scene == "Right Cam");
}

static void test_durations_use_friendly_units()
{
	std::printf("durations collapse to whole minutes/hours when they divide evenly\n");
	PlanInput in;
	in.main = "Main";
	in.others = {"B"};
	in.mainSeconds = 420; // 7 min
	in.otherSeconds = 45; // stays in seconds

	const auto plan = BuildRotationPlan(in);
	CHECK(plan[0].amount == 7);
	CHECK(plan[0].unit == DurationUnit::Minutes);
	CHECK(plan[0].seconds() == 420);
	CHECK(plan[1].amount == 45);
	CHECK(plan[1].unit == DurationUnit::Seconds);
	CHECK(plan[1].seconds() == 45);
}

static void test_loops_with_no_jump_and_carries_transition()
{
	std::printf("every step loops (no onExpire) and carries the chosen transition\n");
	PlanInput in;
	in.main = "Main";
	in.others = {"A", "B"};
	in.transition = "Fade";

	const auto plan = BuildRotationPlan(in);
	for (const RotationStep &s : plan) {
		CHECK(s.onExpire.empty());
		CHECK(s.transition == "Fade");
	}
}

static void test_empty_transition_left_blank()
{
	std::printf("an empty transition leaves the step's transition blank (OBS default)\n");
	PlanInput in;
	in.main = "Main";
	in.others = {"A"};
	const auto plan = BuildRotationPlan(in);
	CHECK(plan[0].transition.empty());
}

static void test_edge_cases()
{
	std::printf("edge cases: no secondaries, no main\n");
	PlanInput justMain;
	justMain.main = "Solo";
	const auto p1 = BuildRotationPlan(justMain);
	CHECK(p1.size() == 1);
	CHECK(p1[0].scene == "Solo");

	PlanInput noMain;
	noMain.others = {"A", "B"};
	const auto p2 = BuildRotationPlan(noMain);
	CHECK(p2.empty());
}

static void test_random_durations_spread_secondary_cutaways()
{
	std::printf("random pacing turns each cut-away into a min-max window\n");
	PlanInput in;
	in.main = "Main";
	in.others = {"A"};
	in.mainSeconds = 420;
	in.otherSeconds = 40;
	in.randomDurations = true;

	const auto plan = BuildRotationPlan(in);
	// The main scene keeps its exact duration: the DJ counts on its pacing.
	CHECK(plan[0].amountMax == 0);
	CHECK(plan[0].seconds() == 420);
	// Cut-aways get a ±25% window (40s => 30-50s) so switches feel organic.
	CHECK(plan[1].amount == 30);
	CHECK(plan[1].amountMax == 50);
	CHECK(plan[1].unit == DurationUnit::Seconds);
}

static void test_fixed_durations_leave_no_range()
{
	std::printf("without random pacing every step keeps a single fixed duration\n");
	PlanInput in;
	in.main = "Main";
	in.others = {"A"};
	const auto plan = BuildRotationPlan(in);
	CHECK(plan[0].amountMax == 0);
	CHECK(plan[1].amountMax == 0);
}

static void test_pacing_presets_ordered()
{
	std::printf("calmer pacing holds the main longer and cuts away shorter\n");
	CHECK(MainSecondsFor(Pacing::Calm) > MainSecondsFor(Pacing::Balanced));
	CHECK(MainSecondsFor(Pacing::Balanced) > MainSecondsFor(Pacing::Dynamic));
	// Cut-aways grow as pacing gets more dynamic.
	CHECK(OtherSecondsFor(Pacing::Calm) < OtherSecondsFor(Pacing::Dynamic));
	// All presets keep the main scene clearly dominant per pass.
	CHECK(MainSecondsFor(Pacing::Dynamic) > OtherSecondsFor(Pacing::Dynamic));
}

int main()
{
	test_interleaves_main_before_each_secondary();
	test_durations_use_friendly_units();
	test_loops_with_no_jump_and_carries_transition();
	test_empty_transition_left_blank();
	test_edge_cases();
	test_random_durations_spread_secondary_cutaways();
	test_fixed_durations_leave_no_range();
	test_pacing_presets_ordered();

	if (g_failures == 0) {
		std::printf("\nAll plan tests passed.\n");
		return 0;
	}
	std::printf("\n%d check(s) failed.\n", g_failures);
	return 1;
}
