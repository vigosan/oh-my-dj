#pragma once

#include <QWidget>

#include <vector>

#include "rotation-engine.hpp"

class QTableWidget;
class QPushButton;
class QLabel;
class QComboBox;

namespace ohmydj {

// The single "Oh My DJ" dock: edit the scene-rotation flow and run it.
class RotationDock : public QWidget {
	Q_OBJECT

public:
	explicit RotationDock(QWidget *parent = nullptr);

	// Repopulate scene pickers from the scenes currently in OBS.
	void refreshScenes();
	// Flush the current flow to disk (used on OBS exit).
	void persist();

private:
	void addRow(const RotationStep &step);
	std::vector<RotationStep> collectSteps() const;
	QStringList sceneNames() const;
	void fillSceneCombo(QComboBox *combo, const QString &selected) const;
	void fillOnExpireCombo(QComboBox *combo, const QString &selected) const;

	void onAdd();
	void onRemove();
	void onMove(int delta);
	void onToggleRun();
	void onEdited();
	void onStepActivated(int index);
	void onRunningChanged(bool running);

	RotationEngine engine_;
	QTableWidget *table_;
	QPushButton *runButton_;
	QLabel *status_;
	bool updating_ = false;
};

} // namespace ohmydj
