#include "update-check.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QVector>

namespace ohmydj {

namespace {

const char *kLatestReleaseApi = "https://api.github.com/repos/vigosan/oh-my-dj/releases/latest";
const char *kReleasesPage = "https://github.com/vigosan/oh-my-dj/releases";

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
	auto *manager = new QNetworkAccessManager(context);

	QNetworkRequest request{QUrl(QString::fromUtf8(kLatestReleaseApi))};
	request.setRawHeader("Accept", "application/vnd.github+json");
	request.setRawHeader("User-Agent", "oh-my-dj"); // GitHub rejects requests without one.
	request.setTransferTimeout(8000);               // Never hold resources if the network stalls.

	QNetworkReply *reply = manager->get(request);
	QObject::connect(reply, &QNetworkReply::finished, context,
			 [reply, manager, currentVersion, onNewer]() {
				 reply->deleteLater();
				 manager->deleteLater();

				 // No network, timeout, rate limit, bad payload… stay silent.
				 if (reply->error() != QNetworkReply::NoError)
					 return;

				 const QJsonObject release =
					 QJsonDocument::fromJson(reply->readAll()).object();

				 QString tag = release.value("tag_name").toString();
				 if (tag.startsWith('v'))
					 tag = tag.mid(1);
				 if (tag.isEmpty() || !IsNewer(currentVersion, tag))
					 return;

				 QString url = release.value("html_url").toString();
				 if (url.isEmpty())
					 url = QString::fromUtf8(kReleasesPage);
				 onNewer(tag, url);
			 });
}

} // namespace ohmydj
