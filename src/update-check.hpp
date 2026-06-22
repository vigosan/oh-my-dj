#pragma once

#include <QString>

#include <functional>

class QObject;

namespace ohmydj {

// Asynchronously asks GitHub for the latest release and, only if it is strictly
// newer than currentVersion, invokes onNewer(version, url) on the GUI thread.
// Fully non-blocking: with no network (or any error/timeout) it silently does
// nothing. The callback is bound to `context`, so it never fires once the
// context object has been destroyed.
void CheckForUpdate(QObject *context, const QString &currentVersion,
		    std::function<void(const QString &version, const QString &url)> onNewer);

} // namespace ohmydj
