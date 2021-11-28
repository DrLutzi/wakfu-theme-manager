#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <mutex>
#include "filedownloader.h"
#include "themewidget.h"
#include "theme.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void downloadAllImages();
	void downloadAllColours();
	void downloadAll();

	void loadConfigurationFile();

private:
	Ui::MainWindow *ui;
	FileDownloader *m_fd;
	QJsonDocument	m_configJson;
	QUrl			m_jsonUrl;
	QDir			m_themesPath;
	QFile			m_configFile;

	Theme			m_defaultTheme;

private slots:
	void loadJsonFromInternet(QUrl url);
	void on_actionDownload_triggered();
	void on_actionOpen_triggered();
};
#endif // MAINWINDOW_H
