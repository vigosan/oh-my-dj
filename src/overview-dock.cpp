#include "overview-dock.hpp"

#include <obs-frontend-api.h>
#include <obs-module.h>

#include <QCursor>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QSize>
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

QPushButton *ToolButton(const QString &iconPath, const QString &tip, QWidget *parent)
{
	auto *button = new QPushButton(parent);
	button->setIcon(QIcon(iconPath));
	button->setIconSize(QSize(16, 16));
	button->setFlat(true);
	button->setToolTip(tip);
	button->setFixedSize(30, 26);
	button->setCursor(Qt::PointingHandCursor);
	return button;
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

	skipBtn_ = ToolButton(QStringLiteral(":/oh-my-dj/icons/skip.svg"), T("OhMyDj.Overview.Skip"), this);
	pauseBtn_ = ToolButton(QStringLiteral(":/oh-my-dj/icons/pause.svg"), T("OhMyDj.Overview.Pause"), this);
	enableBtn_ = ToolButton(QStringLiteral(":/oh-my-dj/icons/rotate.svg"),
				T("OhMyDj.Overview.ToggleRotation"), this);
	enableBtn_->setCheckable(true);
	stopBtn_ = ToolButton(QStringLiteral(":/oh-my-dj/icons/stop.svg"), T("OhMyDj.Overview.StopStream"),
			      this);
	auto *settingsBtn =
		ToolButton(QStringLiteral(":/oh-my-dj/icons/gear.svg"), T("OhMyDj.Overview.Settings"), this);

	auto *divider = new QFrame(this);
	divider->setFrameShape(QFrame::VLine);
	divider->setFrameShadow(QFrame::Sunken);

	// Full-width line that sits just above the bottom toolbar, like the
	// separator above the OBS Scenes/Sources dock toolbars.
	auto *separator = new QFrame(this);
	separator->setFrameShape(QFrame::HLine);
	separator->setFrameShadow(QFrame::Sunken);

	auto *info = new QVBoxLayout();
	info->setContentsMargins(8, 8, 8, 8);
	info->setSpacing(4);
	info->addWidget(rotation_);
	info->addWidget(stream_);

	auto *toolbar = new QHBoxLayout();
	toolbar->setContentsMargins(6, 4, 6, 4);
	toolbar->setSpacing(4);
	toolbar->addWidget(skipBtn_);
	toolbar->addWidget(pauseBtn_);
	toolbar->addWidget(enableBtn_);
	toolbar->addWidget(stopBtn_);
	toolbar->addStretch();
	toolbar->addWidget(divider);
	toolbar->addWidget(settingsBtn);

	// Info at the top, toolbar pinned to the bottom edge of the dock.
	auto *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->addLayout(info);
	layout->addStretch();
	layout->addWidget(separator);
	layout->addLayout(toolbar);

	connect(skipBtn_, &QPushButton::clicked, rotation, &RotationDock::skip);
	connect(pauseBtn_, &QPushButton::clicked, rotation, &RotationDock::togglePaused);
	connect(enableBtn_, &QPushButton::clicked, rotation, &RotationDock::toggleEnabled);
	connect(stopBtn_, &QPushButton::clicked, this, []() { obs_frontend_streaming_stop(); });
	connect(settingsBtn, &QPushButton::clicked, this, [this]() {
		settings_->show();
		settings_->raise();
		settings_->activateWindow();
	});

	connect(rotation, &RotationDock::summaryChanged, this, &OverviewDock::onRotationSummary);
	connect(rotation, &RotationDock::rotationStateChanged, this, &OverviewDock::onRotationState);
	connect(multistream, &MultistreamDock::summaryChanged, this, &OverviewDock::onStreamSummary);

	rotation->pushSummary();
	multistream->pushSummary();
}

void OverviewDock::onRotationSummary(const QString &line)
{
	rotation_->setText(QStringLiteral("🎬 %1").arg(line));
}

void OverviewDock::onRotationState(bool enabled, bool running, bool paused)
{
	skipBtn_->setEnabled(running);
	pauseBtn_->setEnabled(running);
	pauseBtn_->setIcon(QIcon(paused ? QStringLiteral(":/oh-my-dj/icons/play.svg")
					: QStringLiteral(":/oh-my-dj/icons/pause.svg")));
	pauseBtn_->setToolTip(paused ? T("OhMyDj.Overview.Resume") : T("OhMyDj.Overview.Pause"));
	enableBtn_->blockSignals(true);
	enableBtn_->setChecked(enabled);
	enableBtn_->blockSignals(false);
}

void OverviewDock::onStreamSummary(bool streaming, bool sync, const QStringList &names,
				   const QList<int> &statuses)
{
	stopBtn_->setEnabled(streaming);

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
