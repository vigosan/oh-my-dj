#pragma once

#include <string>
#include <vector>

#include "rotation-step.hpp"

namespace ohmydj {

// Pacing presets for the rotation assistant. Music genre is not an input — it is
// just a label for how long the main camera holds vs. the secondary cut-aways.
enum class Pacing { Calm = 0, Balanced = 1, Dynamic = 2 };

// How long (in seconds) the main scene shows per appearance, and how long each
// secondary cut-away lasts, for a given pacing.
int MainSecondsFor(Pacing pacing);
int OtherSecondsFor(Pacing pacing);

// Inputs the user fills in the "Suggest" dialog.
struct PlanInput {
	std::string main;                // main scene (shown most of the time)
	std::vector<std::string> others; // secondary scenes, interleaved one per pass
	int mainSeconds = 420;           // duration of the main scene per appearance
	int otherSeconds = 45;           // duration of each secondary scene
	std::string transition;          // applied to every step; empty => OBS default
};

// Builds a rotation that keeps `main` dominant by re-showing it before every
// secondary scene: main, other0, main, other1, ..., main, otherN  (then loops).
// Durations are emitted in whole seconds. Every step loops to the next (no
// onExpire jump). Returns just {main} when there are no secondaries, or empty
// when there is no main scene. Pure logic, no OBS/Qt, so it is unit-testable.
std::vector<RotationStep> BuildRotationPlan(const PlanInput &in);

} // namespace ohmydj
