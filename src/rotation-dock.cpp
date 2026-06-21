#include "rotation-dock.hpp"

#include <obs.h>
#include <obs-frontend-api.h>
#include <obs-module.h>

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

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
	setObjectName("oh-my-dj-dock");

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

	runButton_ = new QPushButton(T("OhMyDj.Btn.Start"), this);
	status_ = new QLabel(T("OhMyDj.Status.Stopped"), this);

	auto *layout = new QVBoxLayout(this);
	layout->addWidget(table_);
	layout->addLayout(editRow);
	layout->addWidget(runButton_);
	layout->addWidget(status_);

	connect(addBtn, &QPushButton::clicked, this, &RotationDock::onAdd);
	connect(removeBtn, &QPushButton::clicked, this, &RotationDock::onRemove);
	connect(upBtn, &QPushButton::clicked, this, [this]() { onMove(-1); });
	connect(downBtn, &QPushButton::clicked, this, [this]() { onMove(1); });
	connect(runButton_, &QPushButton::clicked, this, &RotationDock::onToggleRun);
	connect(&engine_, &RotationEngine::stepActivated, this, &RotationDock::onStepActivated);
	connect(&engine_, &RotationEngine::runningChanged, this, &RotationDock::onRunningChanged);

	updating_ = true;
	for (const RotationStep &step : LoadRotationSteps())
		addRow(step);
	updating_ = false;
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
		combo->addItem(selected); // keep a scene that was removed from OBS
	combo->setCurrentIndex(qMax(0, combo->findText(selected)));
	combo->blockSignals(false);
}

void RotationDock::fillOnExpireCombo(QComboBox *combo, const QString &selected) const
{
	combo->blockSignals(true);
	combo->clear();
	combo->addItem(T("OhMyDj.OnExpire.Next")); // index 0 => advance + loop
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
	SaveRotationSteps(collectSteps());
}

void RotationDock::onToggleRun()
{
	if (engine_.running()) {
		engine_.stop();
		return;
	}
	engine_.setSteps(collectSteps());
	engine_.start();
}

void RotationDock::onRunningChanged(bool running)
{
	runButton_->setText(running ? T("OhMyDj.Btn.Stop") : T("OhMyDj.Btn.Start"));
	table_->setEnabled(!running);
	if (!running)
		status_->setText(T("OhMyDj.Status.Stopped"));
}

void RotationDock::onStepActivated(int index)
{
	if (index < 0 || index >= table_->rowCount()) {
		table_->clearSelection();
		return;
	}
	table_->selectRow(index);
	const QString scene = Combo(table_, index, ColScene)->currentText();
	status_->setText(T("OhMyDj.Status.Playing").arg(scene));
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
	SaveRotationSteps(collectSteps());
}

} // namespace ohmydj
