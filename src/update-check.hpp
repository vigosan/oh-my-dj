#pragma once

#include <QObject>
#include <QString>

#include <functional>

namespace ohmydj {

// Delivers the result on the GUI thread via a queued connection, so the worker
// thread never touches widgets or a context that may already be gone.
class UpdateNotifier : public QObject {
	Q_OBJECT

public:
	void report(const QString &version, const QString &url) { emit newerAvailable(version, url); }

signals:
	void newerAvailable(const QString &version, const QString &url);
};

// Asynchronously checks GitHub for a newer release and, only if one exists,
// invokes onNewer(version, url) on `context`'s thread. Fully non-blocking; on
// any error (no network, timeout, unsupported platform) it does nothing.
void CheckForUpdate(QObject *context, const QString &currentVersion,
		    std::function<void(const QString &version, const QString &url)> onNewer);

} // namespace ohmydj
