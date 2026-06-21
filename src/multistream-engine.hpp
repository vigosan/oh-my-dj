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

	void start();
	void stop();
	bool running() const;

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
};

StreamConfig LoadStreamConfig();
void SaveStreamConfig(const StreamConfig &config);

} // namespace ohmydj
