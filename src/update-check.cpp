#include "update-check.hpp"

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVector>

#include <string>
#include <thread>

namespace ohmydj {

namespace detail {
// Defined per platform (see update-check-mac.mm and the non-Apple fallback
// below). Returns the response body, or an empty string on any failure.
// Blocking — must run off the GUI thread.
std::string HttpGet(const std::string &url);
} // namespace detail

namespace {

const char *kLatestReleaseApi = "https://api.github.com/repos/vigosan/oh-my-dj/releases/latest";
// Send users to the curated download page on the project site rather than the
// raw GitHub releases list.
const char *kDownloadPage = "https://oh-my-dj.vicent.io/#download";

QVector<int> ParseVersion(const QString &version)
{
	QVector<int> parts;
	for (const QString &part : version.split('.'))
		parts.push_back(part.toInt());
	while (parts.size() < 3)
		parts.push_back(0);
	return parts;
}

// true when `candidate` is strictly newer than `current` (both "X.Y.Z").
bool IsNewer(const QString &current, const QString &candidate)
{
	const QVector<int> a = ParseVersion(current);
	const QVector<int> b = ParseVersion(candidate);
	for (int i = 0; i < 3; ++i) {
		if (b[i] != a[i])
			return b[i] > a[i];
	}
	return false;
}

} // namespace

void CheckForUpdate(QObject *context, const QString &currentVersion,
		    std::function<void(const QString &, const QString &)> onNewer)
{
	auto *notifier = new UpdateNotifier();
	QObject::connect(notifier, &UpdateNotifier::newerAvailable, context,
			 [onNewer](const QString &version, const QString &url) { onNewer(version, url); });
	QObject::connect(notifier, &UpdateNotifier::newerAvailable, notifier, &QObject::deleteLater);

	std::thread([notifier, currentVersion]() {
		const std::string body = detail::HttpGet(kLatestReleaseApi);
		if (body.empty()) {
			notifier->deleteLater();
			return;
		}

		const QJsonObject release = QJsonDocument::fromJson(QByteArray::fromStdString(body)).object();
		QString tag = release.value("tag_name").toString();
		if (tag.startsWith('v'))
			tag = tag.mid(1);
		if (tag.isEmpty() || !IsNewer(currentVersion, tag)) {
			notifier->deleteLater();
			return;
		}

		notifier->report(tag, QString::fromUtf8(kDownloadPage));
	}).detach();
}

#if !defined(__APPLE__)
namespace detail {
// TODO: Windows/Linux transport (e.g. libcurl). Until then the update check is
// a no-op off macOS — the dock simply never shows the notice there.
std::string HttpGet(const std::string &)
{
	return {};
}
} // namespace detail
#endif

} // namespace ohmydj
