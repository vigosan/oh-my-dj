#include "rotation-dock.hpp"

#include <obs.h>
#include <obs-frontend-api.h>
#include <obs-module.h>

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#include "duration.hpp"

namespace ohmydj {

namespace {

enum Column { ColScene = 0, ColAmount, ColUnit, ColOnExpire, ColCount };

QString T(const char *key)
{
	return QString::fromUtf8(obs_module_text(key));
}

QComboBox *Combo(QTableWidget *table, int row, int col)
{
	return qobject_cast<QComboBox *>(table->cellWidget(row, col));
}

} // namespace

RotationDock::RotationDock(QWidget *parent) : QWidget(parent)
{
	setObjectName("oh-my-dj-cameras");
	setMinimumWidth(760);

	table_ = new QTableWidget(this);
	table_->setColumnCount(ColCount);
	table_->setHorizontalHeaderLabels(
		{T("OhMyDj.Col.Scene"), T("OhMyDj.Col.Duration"), T("OhMyDj.Col.Unit"),
		 T("OhMyDj.Col.OnExpire")});
	table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	table_->verticalHeader()->setVisible(false);
	table_->setSelectionBehavior(QAbstractItemView::SelectRows);
	table_->setSelectionMode(QAbstractItemView::SingleSelection);

	auto *addBtn = new QPushButton(T("OhMyDj.Btn.Add"), this);
	auto *removeBtn = new QPushButton(T("OhMyDj.Btn.Remove"), this);
	auto *upBtn = new QPushButton(T("OhMyDj.Btn.Up"), this);
	auto *downBtn = new QPushButton(T("OhMyDj.Btn.Down"), this);

	auto *editRow = new QHBoxLayout();
	editRow->addWidget(addBtn);
	editRow->addWidget(removeBtn);
	editRow->addWidget(upBtn);
	editRow->addWidget(downBtn);
	editRow->addStretch();

	enable_ = new QCheckBox(T("OhMyDj.Rotation.Enable"), this);
	status_ = new QLabel(this);
	tick_ = new QTimer(this);
	tick_->setInterval(1000);

	status_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	auto *statusRow = new QHBoxLayout();
	statusRow->addWidget(enable_);
	statusRow->addStretch();
	statusRow->addWidget(status_);

	auto *layout = new QVBoxLayout(this);
	layout->addWidget(table_);
	layout->addLayout(editRow);
	layout->addLayout(statusRow);

	connect(addBtn, &QPushButton::clicked, this, &RotationDock::onAdd);
	connect(removeBtn, &QPushButton::clicked, this, &RotationDock::onRemove);
	connect(upBtn, &QPushButton::clicked, this, [this]() { onMove(-1); });
	connect(downBtn, &QPushButton::clicked, this, [this]() { onMove(1); });
	connect(enable_, &QCheckBox::toggled, this, &RotationDock::onEnableToggled);
	connect(&engine_, &RotationEngine::stepChanged, this, &RotationDock::onStepChanged);
	connect(tick_, &QTimer::timeout, this, &RotationDock::refreshStatus);

	updating_ = true;
	RotationConfig config = LoadRotationConfig();
	for (const RotationStep &step : config.steps)
		addRow(step);
	enable_->setChecked(config.enabled);
	updating_ = false;

	engine_.setSteps(collectSteps());
	obsStreaming_ = obs_frontend_streaming_active();
	engine_.setStreaming(obsStreaming_);
	engine_.setEnabled(config.enabled);
	onStepChanged(engine_.currentIndex());
}

void RotationDock::setObsStreaming(bool on)
{
	obsStreaming_ = on;
	engine_.setStreaming(on);
	onStepChanged(engine_.currentIndex());
}

QStringList RotationDock::sceneNames() const
{
	QStringList names;
	char **scenes = obs_frontend_get_scene_names();
	if (scenes) {
		for (char **name = scenes; *name; ++name)
			names << QString::fromUtf8(*name);
		bfree(scenes);
	}
	return names;
}

void RotationDock::fillSceneCombo(QComboBox *combo, const QString &selected) const
{
	combo->blockSignals(true);
	combo->clear();
	combo->addItems(sceneNames());
	if (!selected.isEmpty() && combo->findText(selected) < 0)
		combo->addItem(selected);
	combo->setCurrentIndex(qMax(0, combo->findText(selected)));
	combo->blockSignals(false);
}

void RotationDock::fillOnExpireCombo(QComboBox *combo, const QString &selected) const
{
	combo->blockSignals(true);
	combo->clear();
	combo->addItem(T("OhMyDj.OnExpire.Next"));
	combo->addItems(sceneNames());
	if (!selected.isEmpty() && combo->findText(selected) < 0)
		combo->addItem(selected);
	combo->setCurrentIndex(qMax(0, combo->findText(selected)));
	combo->blockSignals(false);
}

void RotationDock::addRow(const RotationStep &step)
{
	const int row = table_->rowCount();
	table_->insertRow(row);

	auto *scene = new QComboBox();
	fillSceneCombo(scene, QString::fromStdString(step.scene));
	table_->setCellWidget(row, ColScene, scene);

	auto *amount = new QSpinBox();
	amount->setRange(1, 9999);
	amount->setValue(step.amount);
	table_->setCellWidget(row, ColAmount, amount);

	auto *unit = new QComboBox();
	unit->addItems({T("OhMyDj.Unit.Seconds"), T("OhMyDj.Unit.Minutes"), T("OhMyDj.Unit.Hours")});
	unit->setCurrentIndex(static_cast<int>(step.unit));
	table_->setCellWidget(row, ColUnit, unit);

	auto *onExpire = new QComboBox();
	fillOnExpireCombo(onExpire, QString::fromStdString(step.onExpire));
	table_->setCellWidget(row, ColOnExpire, onExpire);

	connect(scene, &QComboBox::currentIndexChanged, this, &RotationDock::onEdited);
	connect(amount, &QSpinBox::valueChanged, this, &RotationDock::onEdited);
	connect(unit, &QComboBox::currentIndexChanged, this, &RotationDock::onEdited);
	connect(onExpire, &QComboBox::currentIndexChanged, this, &RotationDock::onEdited);
}

std::vector<RotationStep> RotationDock::collectSteps() const
{
	std::vector<RotationStep> steps;
	for (int row = 0; row < table_->rowCount(); ++row) {
		RotationStep step;
		step.scene = Combo(table_, row, ColScene)->currentText().toStdString();
		step.amount = qobject_cast<QSpinBox *>(table_->cellWidget(row, ColAmount))->value();
		step.unit = UnitFromInt(Combo(table_, row, ColUnit)->currentIndex());
		QComboBox *onExpire = Combo(table_, row, ColOnExpire);
		step.onExpire = onExpire->currentIndex() == 0 ? std::string()
								: onExpire->currentText().toStdString();
		steps.push_back(std::move(step));
	}
	return steps;
}

void RotationDock::onAdd()
{
	RotationStep step;
	const QStringList scenes = sceneNames();
	if (!scenes.isEmpty())
		step.scene = scenes.first().toStdString();
	addRow(step);
	onEdited();
}

void RotationDock::onRemove()
{
	const int row = table_->currentRow();
	if (row < 0)
		return;
	table_->removeRow(row);
	onEdited();
}

void RotationDock::onMove(int delta)
{
	const int row = table_->currentRow();
	const int target = row + delta;
	if (row < 0 || target < 0 || target >= table_->rowCount())
		return;

	std::vector<RotationStep> steps = collectSteps();
	std::swap(steps[row], steps[target]);

	updating_ = true;
	while (table_->rowCount() > 0)
		table_->removeRow(0);
	for (const RotationStep &step : steps)
		addRow(step);
	updating_ = false;

	table_->selectRow(target);
	onEdited();
}

void RotationDock::onEdited()
{
	if (updating_)
		return;
	const std::vector<RotationStep> steps = collectSteps();
	engine_.setSteps(steps);
	SaveRotationConfig({steps, enable_->isChecked()});
}

void RotationDock::onEnableToggled(bool enabled)
{
	engine_.setEnabled(enabled);
	if (!updating_)
		SaveRotationConfig({collectSteps(), enabled});
	onStepChanged(engine_.currentIndex());
}

void RotationDock::onStepChanged(int index)
{
	const bool active = enable_->isChecked() && index >= 0 && index < table_->rowCount();
	if (active)
		table_->selectRow(index);
	else
		table_->clearSelection();
	if (active)
		tick_->start();
	else
		tick_->stop();
	refreshStatus();
}

void RotationDock::refreshStatus()
{
	QString text;
	if (!enable_->isChecked()) {
		text = T("OhMyDj.Rotation.Disabled");
	} else {
		const int index = engine_.currentIndex();
		if (index < 0 || index >= table_->rowCount()) {
			text = obsStreaming_ ? T("OhMyDj.Rotation.Waiting")
					     : T("OhMyDj.Rotation.WaitingObs");
		} else {
			const QString scene = Combo(table_, index, ColScene)->currentText();
			const int secs = engine_.remainingSeconds();
			const int next = engine_.nextIndex();
			if (secs < 0 || next < 0 || next >= table_->rowCount()) {
				text = T("OhMyDj.Rotation.Active").arg(scene);
			} else {
				const QString clock = QString::fromStdString(FormatClock(secs));
				const QString nextScene = Combo(table_, next, ColScene)->currentText();
				text = T("OhMyDj.Rotation.ActiveCountdown").arg(scene, clock, nextScene);
			}
		}
	}
	status_->setText(text);
	emit summaryChanged(text);
}

void RotationDock::pushSummary()
{
	refreshStatus();
}

void RotationDock::onSceneChanged()
{
	engine_.onSceneChanged();
}

void RotationDock::refreshScenes()
{
	updating_ = true;
	for (int row = 0; row < table_->rowCount(); ++row) {
		QComboBox *scene = Combo(table_, row, ColScene);
		fillSceneCombo(scene, scene->currentText());
		QComboBox *onExpire = Combo(table_, row, ColOnExpire);
		fillOnExpireCombo(onExpire, onExpire->currentIndex() == 0 ? QString()
									 : onExpire->currentText());
	}
	updating_ = false;
}

void RotationDock::persist()
{
	SaveRotationConfig({collectSteps(), enable_->isChecked()});
}

} // namespace ohmydj
