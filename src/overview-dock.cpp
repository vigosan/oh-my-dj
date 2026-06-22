#include "overview-dock.hpp"

#include <obs-module.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "multistream-dock.hpp"
#include "rotation-dock.hpp"
#include "stream-target.hpp"

namespace ohmydj {

namespace {

QString T(const char *key)
{
	return QString::fromUtf8(obs_module_text(key));
}

const char *DotColor(StreamStatus status)
{
	switch (status) {
	case StreamStatus::Live:
		return "#3fbf3f";
	case StreamStatus::Connecting:
		return "#e0a23b";
	case StreamStatus::Error:
		return "#e23b3b";
	default:
		return "#888888";
	}
}

} // namespace

OverviewDock::OverviewDock(RotationDock *rotation, MultistreamDock *multistream, QWidget *settings,
			   QWidget *parent)
	: QWidget(parent), settings_(settings)
{
	setObjectName("oh-my-dj-overview");

	rotation_ = new QLabel(this);
	stream_ = new QLabel(this);
	stream_->setTextFormat(Qt::RichText);

	auto *settingsBtn = new QPushButton(T("OhMyDj.Overview.Settings"), this);

	auto *buttonRow = new QHBoxLayout();
	buttonRow->addWidget(settingsBtn);
	buttonRow->addStretch();

	auto *layout = new QVBoxLayout(this);
	layout->setAlignment(Qt::AlignTop);
	layout->addWidget(rotation_);
	layout->addWidget(stream_);
	layout->addLayout(buttonRow);

	connect(settingsBtn, &QPushButton::clicked, this, [this]() {
		settings_->show();
		settings_->raise();
		settings_->activateWindow();
	});
	connect(rotation, &RotationDock::summaryChanged, this, &OverviewDock::onRotationSummary);
	connect(multistream, &MultistreamDock::summaryChanged, this, &OverviewDock::onStreamSummary);

	rotation->pushSummary();
	multistream->pushSummary();
}

void OverviewDock::onRotationSummary(const QString &line)
{
	rotation_->setText(QStringLiteral("🎬 %1").arg(line));
}

void OverviewDock::onStreamSummary(bool streaming, bool sync, const QStringList &names,
				   const QList<int> &statuses)
{
	const QString prefix = QStringLiteral("📡 ");
	const auto dim = [](const QString &text) {
		return QStringLiteral("<span style='color:#888888'>%1</span>").arg(text);
	};

	if (names.isEmpty()) {
		stream_->setText(prefix + dim(QStringLiteral("—")));
		return;
	}
	if (!sync) {
		stream_->setText(prefix + dim(T("OhMyDj.Overview.SyncOff")));
		return;
	}
	if (!streaming) {
		stream_->setText(prefix + dim(T("OhMyDj.Overview.WaitingObs")));
		return;
	}

	QString html = prefix;
	for (int i = 0; i < names.size(); ++i) {
		const auto status = static_cast<StreamStatus>(statuses.value(i));
		html += QStringLiteral("<span style='color:%1'>●</span> %2&nbsp;&nbsp;")
				.arg(QString::fromUtf8(DotColor(status)), names.at(i).toHtmlEscaped());
	}
	stream_->setText(html);
}

} // namespace ohmydj
