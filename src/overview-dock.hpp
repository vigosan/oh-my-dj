#pragma once

#include <QWidget>
#include <QList>
#include <QStringList>

class QLabel;

namespace ohmydj {

class RotationDock;
class MultistreamDock;

// Compact, dockable live panel: shows the rotation countdown and which
// platforms are streaming at a glance, with a button that opens the settings
// window. It owns no logic — it only reflects the two config docks' summaries.
class OverviewDock : public QWidget {
	Q_OBJECT

public:
	OverviewDock(RotationDock *rotation, MultistreamDock *multistream, QWidget *settings,
		     QWidget *parent = nullptr);

private slots:
	void onRotationSummary(const QString &line);
	void onStreamSummary(const QStringList &names, const QList<int> &statuses);

private:
	QLabel *rotation_;
	QLabel *stream_;
	QWidget *settings_;
};

} // namespace ohmydj
