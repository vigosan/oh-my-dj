#include "multistream-dock.hpp"

#include <obs-frontend-api.h>
#include <obs-module.h>

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "stream-presets.hpp"

namespace ohmydj {

namespace {

enum Column { ColName = 0, ColUrl, ColKey, ColActive, ColStatus, ColCount };

QString T(const char *key)
{
	return QString::fromUtf8(obs_module_text(key));
}

QLineEdit *Edit(QTableWidget *table, int row, int col)
{
	return qobject_cast<QLineEdit *>(table->cellWidget(row, col));
}

// For a known platform OBS resolves the ingest server itself, so the URL cell
// becomes a non-editable "Auto (OBS)" hint. A custom destination keeps an
// editable URL field. `customUrl` is only used in the custom case.
void ApplyUrlMode(QLineEdit *url, const QString &platformLabel, const QString &customUrl)
{
	const bool known = !PlatformServiceName(platformLabel.toStdString()).empty();
	url->blockSignals(true);
	if (known) {
		url->setText(T("OhMyDj.Stream.ServerAuto"));
		url->setReadOnly(true);
		url->setEnabled(false);
	} else {
		url->setText(customUrl);
		url->setReadOnly(false);
		url->setEnabled(true);
	}
	url->blockSignals(false);
}

QCheckBox *ActiveBox(QTableWidget *table, int row)
{
	QWidget *cell = table->cellWidget(row, ColActive);
	return cell ? cell->findChild<QCheckBox *>() : nullptr;
}

QString StatusText(StreamStatus status)
{
	switch (status) {
	case StreamStatus::Connecting:
		return T("OhMyDj.Stream.Status.Connecting");
	case StreamStatus::Live:
		return T("OhMyDj.Stream.Status.Live");
	case StreamStatus::Error:
		return T("OhMyDj.Stream.Status.Error");
	case StreamStatus::Stopped:
		return T("OhMyDj.Stream.Status.Stopped");
	case StreamStatus::Idle:
	default:
		return T("OhMyDj.Stream.Status.Idle");
	}
}

void ApplyStatus(QLabel *label, StreamStatus status, const QString &suffix = QString())
{
	QString text = StatusText(status);
	if (!suffix.isEmpty())
		text += QStringLiteral(" · ") + suffix;
	label->setText(text);
	const char *color = "";
	switch (status) {
	case StreamStatus::Live:
		color = "color: #e23b3b; font-weight: bold;";
		break;
	case StreamStatus::Error:
		color = "color: #e23b3b;";
		break;
	default:
		break;
	}
	label->setStyleSheet(color);
}

} // namespace

MultistreamDock::MultistreamDock(QWidget *parent) : QWidget(parent)
{
	setObjectName("oh-my-dj-stream");
	setMinimumWidth(760);

	table_ = new QTableWidget(this);
	table_->setColumnCount(ColCount);
	table_->setHorizontalHeaderLabels({T("OhMyDj.Stream.Col.Name"), T("OhMyDj.Stream.Col.Url"),
					   T("OhMyDj.Stream.Col.Key"), T("OhMyDj.Stream.Col.Active"),
					   T("OhMyDj.Stream.Col.Status")});
	QHeaderView *header = table_->horizontalHeader();
	header->setSectionResizeMode(ColName, QHeaderView::Interactive);
	header->setSectionResizeMode(ColUrl, QHeaderView::Stretch);
	header->setSectionResizeMode(ColKey, QHeaderView::Stretch);
	header->setSectionResizeMode(ColActive, QHeaderView::ResizeToContents);
	header->setSectionResizeMode(ColStatus, QHeaderView::ResizeToContents);
	table_->setColumnWidth(ColName, 130);
	table_->verticalHeader()->setVisible(false);
	table_->setSelectionBehavior(QAbstractItemView::SelectRows);
	table_->setSelectionMode(QAbstractItemView::SingleSelection);

	auto *addBtn = new QPushButton(T("OhMyDj.Stream.Btn.Add"), this);
	auto *removeBtn = new QPushButton(T("OhMyDj.Stream.Btn.Remove"), this);

	auto *editRow = new QHBoxLayout();
	editRow->addWidget(addBtn);
	editRow->addWidget(removeBtn);
	editRow->addStretch();

	sync_ = new QCheckBox(T("OhMyDj.Stream.Sync"), this);
	sync_->setToolTip(T("OhMyDj.Stream.SyncTip"));
	auto *hint = new QLabel(T("OhMyDj.Stream.Hint"), this);
	hint->setEnabled(false);
	hint->setToolTip(T("OhMyDj.Stream.SyncTip"));

	auto *syncRow = new QHBoxLayout();
	syncRow->addWidget(sync_);
	syncRow->addSpacing(8);
	syncRow->addWidget(hint);
	syncRow->addStretch();

	auto *layout = new QVBoxLayout(this);
	layout->addWidget(table_);
	layout->addLayout(editRow);
	layout->addLayout(syncRow);

	connect(addBtn, &QPushButton::clicked, this, &MultistreamDock::onAdd);
	connect(removeBtn, &QPushButton::clicked, this, &MultistreamDock::onRemove);
	connect(sync_, &QCheckBox::toggled, this, &MultistreamDock::onSyncToggled);
	connect(&engine_, &MultistreamEngine::runningChanged, this, &MultistreamDock::onRunningChanged);
	connect(&engine_, &MultistreamEngine::targetStatusChanged, this,
		&MultistreamDock::onTargetStatusChanged);
	connect(&engine_, &MultistreamEngine::targetHealthChanged, this,
		&MultistreamDock::onTargetHealthChanged);

	obsStreaming_ = obs_frontend_streaming_active();

	updating_ = true;
	StreamConfig config = LoadStreamConfig();
	sync_->setChecked(config.syncWithObs);
	for (const StreamTarget &target : config.targets)
		addRow(target);
	updating_ = false;
	emitSummary();
}

void MultistreamDock::addRow(const StreamTarget &target)
{
	const int row = table_->rowCount();
	table_->insertRow(row);

	auto *url = new QLineEdit(QString::fromStdString(target.url));
	url->setPlaceholderText("rtmp://...");

	auto *platform = new QComboBox();
	platform->setEditable(true);
	for (const StreamPreset &preset : StreamPresets())
		platform->addItem(QString::fromStdString(preset.label));
	const QString name = QString::fromStdString(target.name);
	const int presetIdx = platform->findText(name);
	if (presetIdx >= 0)
		platform->setCurrentIndex(presetIdx);
	else
		platform->setCurrentText(name);
	table_->setCellWidget(row, ColName, platform);
	table_->setCellWidget(row, ColUrl, url);
	ApplyUrlMode(url, platform->currentText(), QString::fromStdString(target.url));

	auto *key = new QLineEdit(QString::fromStdString(target.key));
	key->setEchoMode(QLineEdit::Password);
	table_->setCellWidget(row, ColKey, key);

	auto *activeCell = new QWidget();
	auto *active = new QCheckBox(activeCell);
	active->setChecked(target.enabled);
	auto *activeLayout = new QHBoxLayout(activeCell);
	activeLayout->addWidget(active);
	activeLayout->setAlignment(Qt::AlignCenter);
	activeLayout->setContentsMargins(0, 0, 0, 0);
	table_->setCellWidget(row, ColActive, activeCell);

	auto *status = new QLabel();
	status->setAlignment(Qt::AlignCenter);
	ApplyStatus(status, StreamStatus::Idle);
	table_->setCellWidget(row, ColStatus, status);

	// Switching platform retargets the URL cell: known platforms show the
	// "Auto (OBS)" hint, custom restores an editable URL field.
	connect(platform, &QComboBox::currentIndexChanged, this, [this, platform, url](int) {
		if (updating_)
			return;
		ApplyUrlMode(url, platform->currentText(), QString());
		onEdited();
	});
	connect(platform->lineEdit(), &QLineEdit::editingFinished, this, &MultistreamDock::onEdited);
	connect(url, &QLineEdit::editingFinished, this, &MultistreamDock::onEdited);
	connect(key, &QLineEdit::editingFinished, this, &MultistreamDock::onEdited);
	connect(active, &QCheckBox::toggled, this, &MultistreamDock::onEdited);
}

std::vector<StreamTarget> MultistreamDock::collectTargets() const
{
	std::vector<StreamTarget> targets;
	for (int row = 0; row < table_->rowCount(); ++row) {
		StreamTarget t;
		auto *platform = qobject_cast<QComboBox *>(table_->cellWidget(row, ColName));
		t.name = platform->currentText().toStdString();
		t.key = Edit(table_, row, ColKey)->text().toStdString();
		QCheckBox *active = ActiveBox(table_, row);
		t.enabled = active && active->isChecked();
		// A known platform drives OBS's rtmp_common service (regional "auto");
		// anything else stays custom and streams to the typed URL. The URL cell
		// only holds a real address in the custom case (otherwise it is the
		// non-editable "Auto (OBS)" hint, which must not be persisted).
		t.service = PlatformServiceName(t.name);
		if (t.service.empty()) {
			t.url = Edit(table_, row, ColUrl)->text().trimmed().toStdString();
		} else {
			t.server = "auto";
		}
		targets.push_back(std::move(t));
	}
	return targets;
}

StreamConfig MultistreamDock::collectConfig() const
{
	StreamConfig config;
	config.targets = collectTargets();
	config.syncWithObs = sync_->isChecked();
	return config;
}

void MultistreamDock::onAdd()
{
	addRow(StreamTarget{});
	onEdited();
}

void MultistreamDock::onRemove()
{
	const int row = table_->currentRow();
	if (row < 0)
		return;
	table_->removeRow(row);
	onEdited();
}

void MultistreamDock::onEdited()
{
	if (updating_)
		return;
	SaveStreamConfig(collectConfig());
	emitSummary();
}

void MultistreamDock::emitSummary()
{
	QStringList names;
	QList<int> statuses;
	const std::vector<StreamTarget> targets = collectTargets();
	for (int i = 0; i < static_cast<int>(targets.size()); ++i) {
		names << QString::fromStdString(targets[i].name);
		statuses << (i < static_cast<int>(statuses_.size())
				     ? statuses_[i]
				     : static_cast<int>(StreamStatus::Idle));
	}
	emit summaryChanged(obsStreaming_, sync_->isChecked(), names, statuses);
}

void MultistreamDock::pushSummary()
{
	emitSummary();
}

void MultistreamDock::onSyncToggled(bool enabled)
{
	if (!updating_)
		SaveStreamConfig(collectConfig());
	if (!enabled && engine_.running())
		engine_.stop();
	emitSummary();
}

void MultistreamDock::onObsStreamingStarted()
{
	obsStreaming_ = true;
	if (sync_->isChecked()) {
		engine_.setTargets(collectTargets());
		engine_.start(/*useMainEncoders=*/true);
	}
	emitSummary();
}

void MultistreamDock::onObsStreamingStopping()
{
	obsStreaming_ = false;
	engine_.stop();
	emitSummary();
}

void MultistreamDock::onRunningChanged(bool running)
{
	table_->setEnabled(!running);
	if (!running) {
		statuses_.assign(table_->rowCount(), static_cast<int>(StreamStatus::Idle));
		bitrates_.assign(table_->rowCount(), 0);
		drops_.assign(table_->rowCount(), 0);
		for (int row = 0; row < table_->rowCount(); ++row)
			renderStatus(row);
	}
	emitSummary();
}

void MultistreamDock::onTargetStatusChanged(int index, int status)
{
	if (index < 0 || index >= table_->rowCount())
		return;
	if (index >= static_cast<int>(statuses_.size()))
		statuses_.resize(index + 1, static_cast<int>(StreamStatus::Idle));
	statuses_[index] = status;
	// A fresh connection has no health figures yet; clear any stale ones.
	if (status != static_cast<int>(StreamStatus::Live)) {
		if (index < static_cast<int>(bitrates_.size()))
			bitrates_[index] = 0;
		if (index < static_cast<int>(drops_.size()))
			drops_[index] = 0;
	}
	renderStatus(index);
	emitSummary();
}

void MultistreamDock::onTargetHealthChanged(int index, int bitrateKbps, int droppedPercent)
{
	if (index < 0 || index >= table_->rowCount())
		return;
	if (index >= static_cast<int>(bitrates_.size()))
		bitrates_.resize(index + 1, 0);
	if (index >= static_cast<int>(drops_.size()))
		drops_.resize(index + 1, 0);
	bitrates_[index] = bitrateKbps;
	drops_[index] = droppedPercent;
	renderStatus(index);
}

void MultistreamDock::renderStatus(int row)
{
	if (row < 0 || row >= table_->rowCount())
		return;
	auto *label = qobject_cast<QLabel *>(table_->cellWidget(row, ColStatus));
	if (!label)
		return;

	const StreamStatus status = row < static_cast<int>(statuses_.size())
					    ? static_cast<StreamStatus>(statuses_[row])
					    : StreamStatus::Idle;

	QString suffix;
	if (status == StreamStatus::Live) {
		const int kbps = row < static_cast<int>(bitrates_.size()) ? bitrates_[row] : 0;
		const int pct = row < static_cast<int>(drops_.size()) ? drops_[row] : 0;
		if (kbps > 0)
			suffix = T("OhMyDj.Stream.Bitrate").arg(kbps);
		if (pct > 0) {
			const QString drop = T("OhMyDj.Stream.Dropped").arg(pct);
			suffix = suffix.isEmpty() ? drop : suffix + QStringLiteral(" · ") + drop;
		}
	}
	ApplyStatus(label, status, suffix);
}

void MultistreamDock::persist()
{
	SaveStreamConfig(collectConfig());
}

} // namespace ohmydj
