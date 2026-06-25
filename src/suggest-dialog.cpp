#include "suggest-dialog.hpp"

#include <obs-module.h>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

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
	for (const QString &scene : scenes_) {
		auto *item = new QListWidgetItem(scene, others_);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Unchecked);
	}

	pacing_ = new QComboBox(this);
	pacing_->addItem(T("OhMyDj.Suggest.Pacing.Calm"), static_cast<int>(Pacing::Calm));
	pacing_->addItem(T("OhMyDj.Suggest.Pacing.Balanced"), static_cast<int>(Pacing::Balanced));
	pacing_->addItem(T("OhMyDj.Suggest.Pacing.Dynamic"), static_cast<int>(Pacing::Dynamic));
	pacing_->setCurrentIndex(1); // Balanced

	transition_ = new QComboBox(this);
	transition_->addItem(T("OhMyDj.Transition.Default")); // empty selection => OBS default
	transition_->addItems(transitions);

	auto *form = new QFormLayout();
	form->addRow(T("OhMyDj.Suggest.Main"), main_);
	form->addRow(T("OhMyDj.Suggest.Others"), others_);
	form->addRow(T("OhMyDj.Suggest.Pacing"), pacing_);
	form->addRow(T("OhMyDj.Suggest.Transition"), transition_);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto *layout = new QVBoxLayout(this);
	auto *hint = new QLabel(T("OhMyDj.Suggest.Hint"), this);
	hint->setWordWrap(true);
	hint->setEnabled(false);
	layout->addWidget(hint);
	layout->addLayout(form);
	layout->addWidget(buttons);
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

	// First entry is the "Default (OBS)" placeholder => leave transition empty.
	in.transition = transition_->currentIndex() == 0 ? std::string()
							 : transition_->currentText().toStdString();
	return in;
}

} // namespace ohmydj
