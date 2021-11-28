#include "themewidget.h"
#include "ui_themewidget.h"

ThemeWidget::ThemeWidget(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::ThemeWidget)
{
	ui->setupUi(this);
}

ThemeWidget::~ThemeWidget()
{
	delete ui;
}
