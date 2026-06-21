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
// No bitrate, no manual button — it reuses OBS's own encoder and follows OBS's
// Start/Stop Streaming. The DJ only ticks the platforms and the sync switch.
class MultistreamDock : public QWidget {
	Q_OBJECT

public:
	explicit MultistreamDock(QWidget *parent = nullptr);

	void onObsStreamingStarted();  // forwarded from OBS frontend events
	void onObsStreamingStopping();
	void persist();
	void pushSummary();  // re-broadcast platforms + status (for late subscribers)

signals:
	// One entry per configured platform, parallel lists: display name and its
	// current StreamStatus (as int).
	void summaryChanged(const QStringList &names, const QList<int> &statuses);

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

	MultistreamEngine engine_;
	QTableWidget *table_;
	QCheckBox *sync_;
	std::vector<int> statuses_;
	bool updating_ = false;
};

} // namespace ohmydj
