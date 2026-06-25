#pragma once

#include <QDialog>

#include <QStringList>

#include "rotation-plan.hpp"

class QComboBox;
class QListWidget;

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
	QStringList scenes_;
	QComboBox *main_;
	QListWidget *others_;
	QComboBox *pacing_;
	QComboBox *transition_;
};

} // namespace ohmydj
