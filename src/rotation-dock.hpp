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

	void refreshScenes();                                     // repopulate scene pickers from OBS
	void renameScene(const QString &from, const QString &to); // follow an OBS scene rename
	void onSceneChanged();                                    // forwarded from the OBS frontend event
	void setObsStreaming(bool on); // the flow only runs while OBS streams
	void persist();                // flush to disk (on OBS exit)
	void pushSummary();            // re-broadcast the current status (for late subscribers)

	// Live controls, driven from the overview's quick-action bar.
	void skip();
	void togglePaused();
	void toggleEnabled();

signals:
	void summaryChanged(const QString &line);
	void rotationStateChanged(bool enabled, bool running, bool paused);

private:
	void addRow(const RotationStep &step);
	std::vector<RotationStep> collectSteps() const;
	QStringList sceneNames() const;
	QStringList transitionNames() const;
	void fillSceneCombo(QComboBox *combo, const QString &selected) const;
	void fillOnExpireCombo(QComboBox *combo, const QString &selected) const;
	void fillTransitionCombo(QComboBox *combo, const QString &selected) const;

	void onAdd();
	void onRemove();
	void onSuggest();
	void onMove(int delta);
	void onEdited();
	void onEnableToggled(bool enabled);
	void onShuffleToggled(bool shuffle);
	void onStepChanged(int index);
	void refreshStatus();

	RotationEngine engine_;
	QTableWidget *table_;
	QCheckBox *enable_;
	QCheckBox *shuffle_;
	QLabel *status_;
	QTimer *tick_;
	bool obsStreaming_ = false;
	bool updating_ = false;
};

} // namespace ohmydj
