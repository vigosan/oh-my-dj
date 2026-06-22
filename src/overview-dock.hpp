#pragma once

#include <QWidget>
#include <QList>
#include <QStringList>

class QLabel;
class QToolButton;

namespace ohmydj {

class RotationDock;
class MultistreamDock;

// Compact, dockable live panel: shows the rotation countdown and which
// platforms are streaming at a glance, plus a quick-action bar (skip, pause,
// toggle rotation, stop streaming, settings). It owns no logic — it reflects
// the two config docks' summaries and forwards button clicks to them.
class OverviewDock : public QWidget {
	Q_OBJECT

public:
	OverviewDock(RotationDock *rotation, MultistreamDock *multistream, QWidget *settings,
		     QWidget *parent = nullptr);

private slots:
	void onRotationSummary(const QString &line);
	void onRotationState(bool enabled, bool running, bool paused);
	void onStreamSummary(bool streaming, bool sync, const QStringList &names,
			     const QList<int> &statuses);

private:
	QLabel *rotation_;
	QLabel *stream_;
	QToolButton *skipBtn_;
	QToolButton *pauseBtn_;
	QToolButton *enableBtn_;
	QToolButton *stopBtn_;
	QWidget *settings_;
};

} // namespace ohmydj
