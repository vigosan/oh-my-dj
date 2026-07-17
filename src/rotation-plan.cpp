#include "rotation-plan.hpp"

namespace ohmydj {

namespace {

// Express a whole number of seconds as the largest unit that divides it evenly,
// so the table shows "7 minutes" rather than "420 seconds".
void SecondsToAmountUnit(int seconds, int &amount, DurationUnit &unit)
{
	if (seconds < 1)
		seconds = 1;
	if (seconds % 3600 == 0) {
		amount = seconds / 3600;
		unit = DurationUnit::Hours;
	} else if (seconds % 60 == 0) {
		amount = seconds / 60;
		unit = DurationUnit::Minutes;
	} else {
		amount = seconds;
		unit = DurationUnit::Seconds;
	}
}

RotationStep MakeStep(const std::string &scene, int seconds, const std::string &transition)
{
	RotationStep step;
	step.scene = scene;
	SecondsToAmountUnit(seconds, step.amount, step.unit);
	step.transition = transition;
	return step;
}

} // namespace

int MainSecondsFor(Pacing pacing)
{
	switch (pacing) {
	case Pacing::Calm:
		return 540; // 9 min
	case Pacing::Dynamic:
		return 300; // 5 min
	case Pacing::Balanced:
	default:
		return 420; // 7 min
	}
}

int OtherSecondsFor(Pacing pacing)
{
	switch (pacing) {
	case Pacing::Calm:
		return 40;
	case Pacing::Dynamic:
		return 50;
	case Pacing::Balanced:
	default:
		return 45;
	}
}

std::vector<RotationStep> BuildRotationPlan(const PlanInput &in)
{
	std::vector<RotationStep> plan;
	if (in.main.empty())
		return plan;

	if (in.others.empty()) {
		plan.push_back(MakeStep(in.main, in.mainSeconds, in.transition));
		return plan;
	}

	for (const std::string &other : in.others) {
		plan.push_back(MakeStep(in.main, in.mainSeconds, in.transition));
		RotationStep cutaway = MakeStep(other, in.otherSeconds, in.transition);
		if (in.randomDurations) {
			// ±25% around the pacing's cut-away, kept in seconds so the
			// window survives even when the base would collapse to minutes.
			int min = in.otherSeconds - in.otherSeconds / 4;
			if (min < 1)
				min = 1;
			cutaway.amount = min;
			cutaway.amountMax = in.otherSeconds + in.otherSeconds / 4;
			cutaway.unit = DurationUnit::Seconds;
		}
		plan.push_back(std::move(cutaway));
	}
	return plan;
}

} // namespace ohmydj
