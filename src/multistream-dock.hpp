#pragma once

#include <QWidget>

#include <vector>

#include "multistream-engine.hpp"

class QTableWidget;
class QPushButton;
class QSpinBox;

namespace ohmydj {

// The "Streaming" tab: edit destinations and go live to all of them at once.
class MultistreamDock : public QWidget {
	Q_OBJECT

public:
	explicit MultistreamDock(QWidget *parent = nullptr);

	void persist();

private:
	void addRow(const StreamTarget &target);
	std::vector<StreamTarget> collectTargets() const;
	StreamConfig collectConfig() const;

	void onAdd();
	void onRemove();
	void onToggleRun();
	void onEdited();
	void onRunningChanged(bool running);
	void onTargetStatusChanged(int index, int status);

	MultistreamEngine engine_;
	QTableWidget *table_;
	QSpinBox *bitrate_;
	QPushButton *runButton_;
	bool updating_ = false;
};

} // namespace ohmydj
