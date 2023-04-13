#include "formparameters.h"
#include "ui_formparameters.h"
#include "mainwindow.h"

#ifdef Q_OS_WIN
AppParameters FormParameters::m_defaultParameters {QDir(_defaultThemeDirectory), QDir("./themes"), QUrl(_defaultJsonThemeUrl)};
#else
AppParameters FormParameters::m_defaultParameters {QDir(_defaultThemeDirectory), QDir("./themes"), QUrl(_defaultJsonThemeUrl)};
#endif

const QString FormParameters::styleSheetLineEditOkay("background: rgba(230, 250, 230, 255);");
const QString FormParameters::styleSheetLineEditWarning("background: rgba(250, 250, 210, 255);");

FormParameters::FormParameters(AppParameters &parametersRef, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::FormParameters),
	m_parametersRef(parametersRef),
	m_parametersNotSaved(parametersRef),
	m_copyParameters(parametersRef)
{
	ui->setupUi(this);
	setWindowTitle(tr("Parameters"));

	updateAllLineEdits();
}

FormParameters::~FormParameters()
{
	delete ui;
}

void FormParameters::closeEvent(QCloseEvent *e)
{
	(void) e;
	if(m_parametersNotSaved != m_parametersRef)
	{
		emit notifyParametersChanged(m_parametersNotSaved);
	}
}

void FormParameters::on_pushButton_themes_path_released()
{
	QString str = QFileDialog::getExistingDirectory(this, tr("Open the folder where themes should be stored"), QDir::homePath());
	if(!str.isEmpty() && !str.isNull())
	{
		QDir dir(str);
		m_parametersNotSaved.themesPath = dir;
//		ui->lineEdit_themes_path->setText(m_parametersNotSaved.themesPath.absolutePath());
	}
}


void FormParameters::on_pushButton_output_path_released()
{
	QString str = QFileDialog::getExistingDirectory(this, tr("Choose the \"theme\" or \"zaap\" folder of your game"), QDir::homePath());
	if(!str.isEmpty())
	{
		QDir dir(str);
		if(dir.dirName() == "zaap")
		{
			dir.setPath(dir.absolutePath() + "/wakfu/theme");
		}
		m_parametersNotSaved.outputPath.setPath(dir.absolutePath());
		ui->lineEdit_output_path->setText(m_parametersNotSaved.outputPath.absolutePath());
	}
}

void FormParameters::on_lineEdit_json_url_textChanged(const QString &arg1)
{
	m_parametersNotSaved.jsonUrl = QUrl(arg1);
	updateStyleSheetOfJsonUrl();
}

void FormParameters::on_lineEdit_themes_path_textChanged(const QString &arg1)
{
	m_parametersNotSaved.themesPath = QDir(arg1);
	updateStyleSheetOfThemesPath();
}

void FormParameters::on_lineEdit_output_path_textChanged(const QString &arg1)
{
	m_parametersNotSaved.outputPath = QDir(arg1);
	updateStyleSheetOfOutputPath();
}

bool FormParameters::checkValidityOfJsonUrl() const
{
	return AppParameters::checkValidityOfJsonUrl(m_parametersNotSaved.jsonUrl.url());
}

bool FormParameters::checkValidityOfThemesPath() const
{
	return true;
}

bool FormParameters::checkValidityOfOutputPath() const
{
	return(m_parametersNotSaved.outputPath.exists() && m_parametersNotSaved.outputPath.dirName() == "theme");
}

void FormParameters::updateStyleSheetOfJsonUrl()
{
	if(checkValidityOfJsonUrl())
	{
		ui->lineEdit_json_url->setStyleSheet(styleSheetLineEditOkay);
	}
	else
	{
		ui->lineEdit_json_url->setStyleSheet(styleSheetLineEditWarning);
	}
}

void FormParameters::updateStyleSheetOfThemesPath()
{
//	if(checkValidityOfThemesPath())
//	{
//		ui->lineEdit_themes_path->setStyleSheet(styleSheetLineEditOkay);
//	}
//	else
//	{
//		ui->lineEdit_themes_path->setStyleSheet(styleSheetLineEditWarning);
//	}
}

void FormParameters::updateStyleSheetOfOutputPath()
{
	if(checkValidityOfOutputPath())
	{
		ui->lineEdit_output_path->setStyleSheet(styleSheetLineEditOkay);
	}
	else
	{
		ui->lineEdit_output_path->setStyleSheet(styleSheetLineEditWarning);
	}
}

void FormParameters::updateAllLineEdits()
{
	ui->lineEdit_json_url->setText(m_parametersNotSaved.jsonUrl.toString());
	ui->lineEdit_output_path->setText(m_parametersNotSaved.outputPath.absolutePath());
//	ui->lineEdit_themes_path->setText(m_parametersNotSaved.themesPath.absolutePath());
}

void FormParameters::on_pushButton_reset_released()
{
	m_parametersNotSaved = m_copyParameters;
	updateAllLineEdits();
	return;
}

void FormParameters::on_pushButton_ok_released()
{
	closeEvent(nullptr);
	deleteLater();
}


void FormParameters::on_FormParameters_destroyed()
{
	return;
}

void FormParameters::on_pushButton_default_released()
{
	m_parametersNotSaved = m_defaultParameters;
	updateAllLineEdits();
}

