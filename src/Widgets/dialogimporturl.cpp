#include "dialogimporturl.h"
#include "ui_dialogimporturl.h"

DialogImportUrl::DialogImportUrl(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogImportUrl)
{
	ui->setupUi(this);
}

DialogImportUrl::~DialogImportUrl()
{
	delete ui;
}

void DialogImportUrl::on_buttonBox_accepted()
{
	emit urlProvided(ui->lineEdit_name->text(), ui->lineEdit_url->text());
}

