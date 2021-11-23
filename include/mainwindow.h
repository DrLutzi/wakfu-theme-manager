#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include "filedownloader.h"

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

	void loadConfiguration();

private:
	Ui::MainWindow *ui;
	FileDownloader *m_fd;
	QJsonDocument	m_wakfuJson;
	QJsonDocument	m_configJson;
	QUrl			m_jsonUrl;
	QDir			m_themesPath;
	QFile			m_configFile;

private slots:
	void loadJsonFromInternet(QUrl url);
};
#endif // MAINWINDOW_H
