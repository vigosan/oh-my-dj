#include "suggest-dialog.hpp"

#include <obs-module.h>

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QListWidget>
#include <QSet>
#include <QVBoxLayout>

#include "duration.hpp"

namespace ohmydj {

namespace {

QString T(const char *key)
{
	return QString::fromUtf8(obs_module_text(key));
}

} // namespace

SuggestDialog::SuggestDialog(const QStringList &scenes, const QStringList &transitions, QWidget *parent)
	: QDialog(parent), scenes_(scenes)
{
	setWindowTitle(T("OhMyDj.Suggest.Title"));

	main_ = new QComboBox(this);
	main_->addItems(scenes_);

	others_ = new QListWidget(this);
	// Only the tick boxes carry meaning here; the row-selection highlight just
	// makes the multi-select checklist look single-choice, so switch it off.
	others_->setSelectionMode(QAbstractItemView::NoSelection);
	rebuildOthers();

	pacing_ = new QComboBox(this);
	pacing_->addItem(T("OhMyDj.Suggest.Pacing.Calm"), static_cast<int>(Pacing::Calm));
	pacing_->addItem(T("OhMyDj.Suggest.Pacing.Balanced"), static_cast<int>(Pacing::Balanced));
	pacing_->addItem(T("OhMyDj.Suggest.Pacing.Dynamic"), static_cast<int>(Pacing::Dynamic));
	pacing_->setCurrentIndex(1); // Balanced

	random_ = new QCheckBox(T("OhMyDj.Suggest.Random"), this);
	random_->setToolTip(T("OhMyDj.Suggest.RandomHint"));

	transition_ = new QComboBox(this);
	transition_->addItem(T("OhMyDj.Transition.Default")); // empty selection => OBS default
	transition_->addItems(transitions);

	auto *form = new QFormLayout();
	form->addRow(T("OhMyDj.Suggest.Main"), main_);
	form->addRow(T("OhMyDj.Suggest.Others"), others_);
	form->addRow(T("OhMyDj.Suggest.Pacing"), pacing_);
	form->addRow(QString(), random_);
	form->addRow(T("OhMyDj.Suggest.Transition"), transition_);

	preview_ = new QLabel(this);
	preview_->setWordWrap(true);
	preview_->setTextFormat(Qt::PlainText);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

	// Changing the main scene removes it from the secondary list; any choice
	// updates the live preview.
	connect(main_, &QComboBox::currentTextChanged, this, [this]() {
		rebuildOthers();
		updatePreview();
	});
	connect(others_, &QListWidget::itemChanged, this, [this]() { updatePreview(); });
	connect(pacing_, &QComboBox::currentIndexChanged, this, [this]() { updatePreview(); });
	connect(random_, &QCheckBox::toggled, this, [this]() { updatePreview(); });
	connect(transition_, &QComboBox::currentIndexChanged, this, [this]() { updatePreview(); });

	auto *layout = new QVBoxLayout(this);
	auto *hint = new QLabel(T("OhMyDj.Suggest.Hint"), this);
	hint->setWordWrap(true);
	hint->setEnabled(false);
	layout->addWidget(hint);
	layout->addLayout(form);
	layout->addWidget(preview_);
	layout->addWidget(buttons);

	updatePreview();
}

void SuggestDialog::rebuildOthers()
{
	const QString main = main_->currentText();

	// Remember what was already ticked so a main-scene change keeps the rest.
	QSet<QString> checked;
	for (int i = 0; i < others_->count(); ++i) {
		QListWidgetItem *item = others_->item(i);
		if (item->checkState() == Qt::Checked)
			checked.insert(item->text());
	}

	others_->blockSignals(true);
	others_->clear();
	for (const QString &scene : scenes_) {
		if (scene == main) // the main scene is never a cut-away target
			continue;
		auto *item = new QListWidgetItem(scene, others_);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(checked.contains(scene) ? Qt::Checked : Qt::Unchecked);
	}
	others_->blockSignals(false);
}

void SuggestDialog::updatePreview()
{
	const std::vector<RotationStep> steps = BuildRotationPlan(plan());
	if (steps.empty()) {
		preview_->setText(T("OhMyDj.Suggest.PreviewEmpty"));
		return;
	}

	QStringList parts;
	for (const RotationStep &step : steps) {
		QString clock = QString::fromStdString(FormatClock(static_cast<int>(step.seconds())));
		if (step.amountMax > step.amount) {
			const int maxSecs = step.amountMax * UnitToSeconds(step.unit);
			clock += "–" + QString::fromStdString(FormatClock(maxSecs));
		}
		parts << QString::fromStdString(step.scene) + " " + clock;
	}
	preview_->setText(T("OhMyDj.Suggest.Preview").arg(parts.join(" → ")));
}

PlanInput SuggestDialog::plan() const
{
	PlanInput in;
	in.main = main_->currentText().toStdString();

	for (int i = 0; i < others_->count(); ++i) {
		QListWidgetItem *item = others_->item(i);
		if (item->checkState() != Qt::Checked)
			continue;
		const std::string scene = item->text().toStdString();
		if (scene == in.main) // never cut away to the main scene itself
			continue;
		in.others.push_back(scene);
	}

	const auto pacing = static_cast<Pacing>(pacing_->currentData().toInt());
	in.mainSeconds = MainSecondsFor(pacing);
	in.otherSeconds = OtherSecondsFor(pacing);
	in.randomDurations = random_->isChecked();

	// First entry is the "Default (OBS)" placeholder => leave transition empty.
	in.transition = transition_->currentIndex() == 0 ? std::string()
							 : transition_->currentText().toStdString();
	return in;
}

} // namespace ohmydj
