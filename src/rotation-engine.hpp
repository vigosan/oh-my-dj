#pragma once

#include <QObject>
#include <QTimer>

#include <vector>

#include "rotation-step.hpp"

namespace ohmydj {

// Drives the rotation on the UI thread via a single-shot QTimer, so every
// scene switch happens where libobs expects it.
class RotationEngine : public QObject {
	Q_OBJECT

public:
	explicit RotationEngine(QObject *parent = nullptr);

	void setSteps(std::vector<RotationStep> steps);
	const std::vector<RotationStep> &steps() const { return steps_; }

	void start();
	void stop();
	bool running() const { return running_; }
	int currentIndex() const { return current_; }

signals:
	void runningChanged(bool running);
	void stepActivated(int index);

private slots:
	void onTimeout();

private:
	void activate(int index);

	std::vector<RotationStep> steps_;
	QTimer timer_;
	int current_ = -1;
	bool running_ = false;
};

// Persistence (per OBS profile). Returns false on a missing/empty config.
std::vector<RotationStep> LoadRotationSteps();
void SaveRotationSteps(const std::vector<RotationStep> &steps);

} // namespace ohmydj
