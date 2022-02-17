#include "browsercontent.h"
#include "ui_browsercontent.h"

BrowserContent::BrowserContent(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::BrowserContent)
{
	ui->setupUi(this);
}

BrowserContent::~BrowserContent()
{
	delete ui;
}
