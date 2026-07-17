#pragma once

#include <QObject>
#include <QTimer>

#include <random>
#include <string>
#include <vector>

#include "rotation-step.hpp"

namespace ohmydj {

// Reactive scene rotation: the DJ only enables/disables it. While enabled, the
// flow engages automatically whenever the program scene matches one of the
// steps, then advances + loops on its own. Switching to a scene outside the
// flow simply parks it until a flow scene is shown again.
class RotationEngine : public QObject {
	Q_OBJECT

public:
	explicit RotationEngine(QObject *parent = nullptr);

	void setSteps(std::vector<RotationStep> steps);
	const std::vector<RotationStep> &steps() const { return steps_; }

	void setEnabled(bool enabled);
	bool enabled() const { return enabled_; }

	// Shuffle mode: advance to a random step instead of the linear flow. The
	// pick avoids the current scene (and the previous one, once there are >= 3
	// steps), and ignores each step's onExpire jump.
	void setShuffle(bool shuffle) { shuffle_ = shuffle; }
	bool shuffle() const { return shuffle_; }

	// The flow only runs while OBS is actually streaming.
	void setStreaming(bool streaming);

	// Live controls: jump to the next step now, or hold on the current one.
	void skip();
	void setPaused(bool paused);
	bool paused() const { return paused_; }

	// Called on every OBS scene change (and once when enabled) to re-evaluate.
	void onSceneChanged();

	int currentIndex() const { return current_; }

	// Where the flow goes when the active step ends (-1 if there is no flow, or
	// in shuffle mode where the next step is decided randomly at advance time).
	int nextIndex() const { return shuffle_ ? -1 : ResolveNextIndex(steps_, current_); }

	// Seconds left on the active step, or -1 when parked/idle.
	int remainingSeconds() const
	{
		if (current_ < 0)
			return -1;
		if (paused_)
			return (pausedRemainingMs_ + 999) / 1000;
		const int ms = timer_.remainingTime();
		return ms < 0 ? -1 : (ms + 999) / 1000;
	}

signals:
	// index of the active step, or -1 when parked/idle.
	void stepChanged(int index);

private slots:
	void onTimeout();

private:
	void evaluate();
	void advance();
	void arm(int index);
	void park();
	int stepDurationMs(int index);
	bool active() const { return enabled_ && streaming_; }

	std::vector<RotationStep> steps_;
	QTimer timer_;
	bool enabled_ = false;
	bool streaming_ = false;
	bool shuffle_ = false;
	bool paused_ = false;
	int pausedRemainingMs_ = 0;
	int current_ = -1;
	int previous_ = -1; // step before current_, so shuffle can avoid bouncing back
	std::mt19937 rng_{std::random_device{}()};

	// Distinguish scene changes we trigger ourselves from the DJ's.
	bool expectingSelfSwitch_ = false;
	std::string pendingScene_;
};

// Persistence (per OBS profile): the flow plus its enabled state.
struct RotationConfig {
	std::vector<RotationStep> steps;
	bool enabled = false;
	bool shuffle = false;
};

RotationConfig LoadRotationConfig();
void SaveRotationConfig(const RotationConfig &config);

} // namespace ohmydj
