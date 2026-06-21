#include "about-tab.hpp"

#include <obs-module.h>

#include <QDesktopServices>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#ifndef PLUGIN_VERSION
#define PLUGIN_VERSION "0.0.0"
#endif

namespace ohmydj {

namespace {

QString T(const char *key)
{
	return QString::fromUtf8(obs_module_text(key));
}

const char *kRepoUrl = "https://github.com/vigosan/oh-my-dj";
const char *kDonateUrl = "https://www.paypal.com/donate/?hosted_button_id=FPP74JCGWCPWS";

QLabel *Link(const QString &html, QWidget *parent)
{
	auto *label = new QLabel(html, parent);
	label->setTextFormat(Qt::RichText);
	label->setOpenExternalLinks(true);
	return label;
}

} // namespace

QWidget *CreateAboutTab()
{
	auto *widget = new QWidget();
	auto *layout = new QVBoxLayout(widget);
	layout->setAlignment(Qt::AlignTop);

	auto *title = new QLabel("<h2>Oh My DJ</h2>", widget);

	auto *version = new QLabel(T("OhMyDj.About.Version").arg(PLUGIN_VERSION), widget);
	version->setEnabled(false);

	auto *author = new QLabel("Vicent Gozalbes", widget);
	auto *email = Link("<a href=\"mailto:hello@vicent.io\">hello@vicent.io</a>", widget);
	auto *repo = Link(QString("<a href=\"%1\">%1</a>").arg(kRepoUrl), widget);

	auto *donate = new QPushButton(T("OhMyDj.About.Donate"), widget);
	QObject::connect(donate, &QPushButton::clicked, donate,
			 []() { QDesktopServices::openUrl(QUrl(kDonateUrl)); });

	layout->addWidget(title);
	layout->addWidget(version);
	layout->addSpacing(8);
	layout->addWidget(author);
	layout->addWidget(email);
	layout->addWidget(repo);
	layout->addSpacing(12);
	layout->addWidget(donate);
	layout->addStretch();

	return widget;
}

} // namespace ohmydj
