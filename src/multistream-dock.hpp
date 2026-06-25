#pragma once

#include <QWidget>
#include <QList>
#include <QStringList>

#include <vector>

#include "multistream-engine.hpp"

class QTableWidget;
class QCheckBox;

namespace ohmydj {

// The "Streaming" tab: a list of destinations that go live together with OBS.
// No manual button — it reuses OBS's own encoder and follows OBS's Start/Stop
// Streaming. The DJ only ticks the platforms and the sync switch; each row
// surfaces its live status plus bitrate and dropped-frame health while live.
class MultistreamDock : public QWidget {
	Q_OBJECT

public:
	explicit MultistreamDock(QWidget *parent = nullptr);

	void onObsStreamingStarted();  // forwarded from OBS frontend events
	void onObsStreamingStopping();
	void persist();
	void pushSummary();  // re-broadcast platforms + status (for late subscribers)

signals:
	// `streaming` is whether OBS itself is live and `sync` whether the sync
	// switch is on; `names`/`statuses` are parallel per-platform lists (the
	// status is a StreamStatus as int).
	void summaryChanged(bool streaming, bool sync, const QStringList &names,
			    const QList<int> &statuses);

private:
	void addRow(const StreamTarget &target);
	std::vector<StreamTarget> collectTargets() const;
	StreamConfig collectConfig() const;
	void emitSummary();

	void onAdd();
	void onRemove();
	void onEdited();
	void onSyncToggled(bool enabled);
	void onRunningChanged(bool running);
	void onTargetStatusChanged(int index, int status);
	void onTargetHealthChanged(int index, int bitrateKbps, int droppedPercent);
	void renderStatus(int row);

	MultistreamEngine engine_;
	QTableWidget *table_;
	QCheckBox *sync_;
	std::vector<int> statuses_;
	std::vector<int> bitrates_;
	std::vector<int> drops_;
	bool obsStreaming_ = false;
	bool updating_ = false;
};

} // namespace ohmydj
