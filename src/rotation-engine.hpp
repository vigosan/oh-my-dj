#pragma once

#include <QObject>
#include <QTimer>

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

	// Called on every OBS scene change (and once when enabled) to re-evaluate.
	void onSceneChanged();

	int currentIndex() const { return current_; }

	// Where the flow goes when the active step ends (-1 if there is no flow).
	int nextIndex() const { return ResolveNextIndex(steps_, current_); }

	// Seconds left on the active step, or -1 when parked/idle.
	int remainingSeconds() const
	{
		if (!enabled_ || current_ < 0)
			return -1;
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

	std::vector<RotationStep> steps_;
	QTimer timer_;
	bool enabled_ = false;
	int current_ = -1;

	// Distinguish scene changes we trigger ourselves from the DJ's.
	bool expectingSelfSwitch_ = false;
	std::string pendingScene_;
};

// Persistence (per OBS profile): the flow plus its enabled state.
struct RotationConfig {
	std::vector<RotationStep> steps;
	bool enabled = false;
};

RotationConfig LoadRotationConfig();
void SaveRotationConfig(const RotationConfig &config);

} // namespace ohmydj
