#include "multistream-dock.hpp"

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
	auto *hint = new QLabel(T("OhMyDj.Stream.Hint"), this);
	hint->setWordWrap(true);
	hint->setEnabled(false);

	auto *layout = new QVBoxLayout(this);
	layout->addWidget(table_);
	layout->addLayout(editRow);
	layout->addWidget(sync_);
	layout->addWidget(hint);

	connect(addBtn, &QPushButton::clicked, this, &MultistreamDock::onAdd);
	connect(removeBtn, &QPushButton::clicked, this, &MultistreamDock::onRemove);
	connect(sync_, &QCheckBox::toggled, this, &MultistreamDock::onSyncToggled);
	connect(&engine_, &MultistreamEngine::runningChanged, this, &MultistreamDock::onRunningChanged);
	connect(&engine_, &MultistreamEngine::targetStatusChanged, this,
		&MultistreamDock::onTargetStatusChanged);

	updating_ = true;
	StreamConfig config = LoadStreamConfig();
	sync_->setChecked(config.syncWithObs);
	for (const StreamTarget &target : config.targets)
		addRow(target);
	updating_ = false;
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

	auto *status = new QLabel(StatusText(StreamStatus::Idle));
	status->setAlignment(Qt::AlignCenter);
	table_->setCellWidget(row, ColStatus, status);

	// Picking a known platform fills its RTMP URL, unless the user typed one.
	connect(platform, &QComboBox::currentIndexChanged, this, [this, platform, url](int) {
		if (updating_)
			return;
		const QString presetUrl =
			QString::fromStdString(PresetUrl(platform->currentText().toStdString()));
		if (!presetUrl.isEmpty() && IsPresetUrl(url->text().trimmed().toStdString()))
			url->setText(presetUrl);
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
		t.url = Edit(table_, row, ColUrl)->text().trimmed().toStdString();
		t.key = Edit(table_, row, ColKey)->text().toStdString();
		QCheckBox *active = ActiveBox(table_, row);
		t.enabled = active && active->isChecked();
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
}

void MultistreamDock::onSyncToggled(bool enabled)
{
	if (!updating_)
		SaveStreamConfig(collectConfig());
	if (!enabled && engine_.running())
		engine_.stop();
}

void MultistreamDock::onObsStreamingStarted()
{
	if (!sync_->isChecked())
		return;
	engine_.setTargets(collectTargets());
	engine_.start(/*useMainEncoders=*/true);
}

void MultistreamDock::onObsStreamingStopping()
{
	engine_.stop();
}

void MultistreamDock::onRunningChanged(bool running)
{
	table_->setEnabled(!running);
	if (!running) {
		for (int row = 0; row < table_->rowCount(); ++row) {
			if (auto *label = qobject_cast<QLabel *>(table_->cellWidget(row, ColStatus)))
				label->setText(StatusText(StreamStatus::Idle));
		}
	}
}

void MultistreamDock::onTargetStatusChanged(int index, int status)
{
	if (index < 0 || index >= table_->rowCount())
		return;
	if (auto *label = qobject_cast<QLabel *>(table_->cellWidget(index, ColStatus)))
		label->setText(StatusText(static_cast<StreamStatus>(status)));
}

void MultistreamDock::persist()
{
	SaveStreamConfig(collectConfig());
}

} // namespace ohmydj
