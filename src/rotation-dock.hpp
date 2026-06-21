#pragma once

#include <QWidget>

#include <vector>

#include "rotation-engine.hpp"

class QTableWidget;
class QCheckBox;
class QLabel;
class QComboBox;
class QTimer;

namespace ohmydj {

// The "Cameras" tab: edit the scene-rotation flow. There is no Start/Stop —
// the DJ just enables it, and the flow engages whenever the program scene
// matches a step.
class RotationDock : public QWidget {
	Q_OBJECT

public:
	explicit RotationDock(QWidget *parent = nullptr);

	void refreshScenes();   // repopulate scene pickers from OBS
	void onSceneChanged();  // forwarded from the OBS frontend event
	void persist();         // flush to disk (on OBS exit)

private:
	void addRow(const RotationStep &step);
	std::vector<RotationStep> collectSteps() const;
	QStringList sceneNames() const;
	void fillSceneCombo(QComboBox *combo, const QString &selected) const;
	void fillOnExpireCombo(QComboBox *combo, const QString &selected) const;

	void onAdd();
	void onRemove();
	void onMove(int delta);
	void onEdited();
	void onEnableToggled(bool enabled);
	void onStepChanged(int index);
	void refreshStatus();

	RotationEngine engine_;
	QTableWidget *table_;
	QCheckBox *enable_;
	QLabel *status_;
	QTimer *tick_;
	bool updating_ = false;
};

} // namespace ohmydj
