#pragma once

#include <QDialog>

#include <QStringList>
#include <QVector>

#include "rotation-plan.hpp"

class QCheckBox;
class QComboBox;
class QLabel;
class QWidget;

namespace ohmydj {

// Small assistant that builds a rotation keeping one "main" scene dominant with
// periodic cut-aways to secondary scenes. The user picks the main scene, the
// secondaries, a pacing, and an optional transition; the dialog hands back a
// ready-to-apply PlanInput. It is fed the available scene/transition names so
// it stays free of direct OBS calls.
class SuggestDialog : public QDialog {
	Q_OBJECT

public:
	SuggestDialog(const QStringList &scenes, const QStringList &transitions,
		      QWidget *parent = nullptr);

	// Valid only after exec() returns QDialog::Accepted.
	PlanInput plan() const;

private:
	// Repopulate the secondary-scene list, leaving out whatever scene is the
	// current main one (you never cut away to the main scene itself), keeping any
	// existing checks.
	void rebuildOthers();
	// Refresh the live preview of the rotation the current choices would build.
	void updatePreview();

	QStringList scenes_;
	QComboBox *main_;
	QWidget *others_;
	QVector<QCheckBox *> otherBoxes_;
	QComboBox *pacing_;
	QCheckBox *random_;
	QComboBox *transition_;
	QLabel *preview_;
};

} // namespace ohmydj
