#ifndef DIALOGIMPORTURL_H
#define DIALOGIMPORTURL_H

#include <QDialog>
#include <QUrl>

namespace Ui {
class DialogImportUrl;
}

class DialogImportUrl : public QDialog
{
	Q_OBJECT

public:
	explicit DialogImportUrl(QWidget *parent = nullptr);
	~DialogImportUrl();

signals:
	void urlProvided(QString name, QString url);

private slots:
	void on_buttonBox_accepted();

private:
	Ui::DialogImportUrl *ui;
};

#endif // DIALOGIMPORTURL_H
