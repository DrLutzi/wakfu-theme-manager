#ifndef FORMPARAMETERS_H
#define FORMPARAMETERS_H

#include <QDialog>
#include <QDir>
#include <QUrl>
#include <QFileDialog>
#include "types.h"

namespace Ui {
class FormParameters;
}

class FormParameters : public QDialog
{
	Q_OBJECT

public:
	explicit FormParameters(AppParameters &parametersRef, QWidget *parent = nullptr);
	~FormParameters();

	void closeEvent(QCloseEvent *e);

private slots:

	void on_pushButton_themes_path_released();
	void on_pushButton_output_path_released();

	void on_lineEdit_json_url_textChanged(const QString &arg1);
	void on_lineEdit_themes_path_textChanged(const QString &arg1);
	void on_lineEdit_output_path_textChanged(const QString &arg1);

	void on_pushButton_reset_released();

	void on_pushButton_ok_released();

	void on_FormParameters_destroyed();

private:

	bool checkValidityOfJsonUrl() const;
	bool checkValidityOfThemesPath() const;
	bool checkValidityOfOutputPath() const;

	void updateStyleSheetOfJsonUrl();
	void updateStyleSheetOfThemesPath();
	void updateStyleSheetOfOutputPath();

	void updateAllLineEdits();

	Ui::FormParameters *ui;
	AppParameters &m_parametersRef;
	AppParameters m_parametersNotSaved;
	AppParameters m_copyParameters;

	static const QString styleSheetLineEditOkay;
	static const QString styleSheetLineEditWarning;
};

#endif // FORMPARAMETERS_H
