#pragma once

#include <QObject>

#include <memory>
#include <vector>

#include "stream-target.hpp"

namespace ohmydj {

// Pushes the OBS program output to several RTMP destinations at once.
// All destinations share one video + one audio encoder (one encode, many
// outputs), so adding platforms costs bandwidth, not CPU.
class MultistreamEngine : public QObject {
	Q_OBJECT

public:
	explicit MultistreamEngine(QObject *parent = nullptr);
	~MultistreamEngine() override;

	void setTargets(std::vector<StreamTarget> targets);
	void setVideoBitrate(int kbps);

	// When `useMainEncoders` is true the outputs borrow OBS's main streaming
	// encoders (zero extra encode), which only exist while OBS is streaming —
	// that is the case in the "sync with OBS" mode. Otherwise we create our own.
	void start(bool useMainEncoders = false);
	void stop();
	bool running() const;

	// Invoked from the output's stop callback after an unexpected drop: retries
	// that one destination after a short delay while we are still meant to be live.
	void reconnect(int index);

signals:
	void runningChanged(bool running);
	// `status` is a StreamStatus; `index` refers to the target's row.
	void targetStatusChanged(int index, int status);

private:
	struct Impl;
	std::unique_ptr<Impl> d_;
};

// Persistence (per OBS profile), kept separate from the rotation config file.
struct StreamConfig {
	std::vector<StreamTarget> targets;
	int videoBitrate = 6000;
	bool syncWithObs = false; // start/stop together with OBS's own streaming
};

StreamConfig LoadStreamConfig();
void SaveStreamConfig(const StreamConfig &config);

} // namespace ohmydj
