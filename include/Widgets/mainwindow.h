#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QScrollBar>
#include <QFileDialog>
#include <QMessageBox>
#include <mutex>
#include <QtConcurrent/QtConcurrent>
#include "filedownloader.h"
#include "themewidget.h"
#include "theme.h"
#include "scrollareacontent.h"
#include "scrollarea.h"

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

	bool setActionIcons();
	bool loadConfigurationFile();
	void checkOutputExistence();
	void loadAllThemes();
    void createScrollAreas();
	void createAllThemeWidgets();

	bool saveConfigurationFile();
	void resetDefaultThemeWidget();

private:
	Ui::MainWindow *ui;
	FileDownloader *m_fd;
	QJsonDocument	m_configJson;
	QUrl			m_jsonUrl;
	QString			m_pixmapRelativePath;
	QDir			m_themesPath;
	QDir			m_outputPath;
	QDir			m_defaultThemePath;
	QFile			m_configFile;

	Theme			m_defaultTheme;
	Theme			m_outputTheme;
	std::vector<Theme> m_extraThemes;

	ThemeWidget *m_defaultThemeWidget;
	std::vector<ThemeWidget *> m_extraThemeWidgets;

	static void clearLayout(QLayout *layout);
	void initJson(bool forceReset = false);
	void downloadDefault();

private slots:
	void loadJsonFromInternet(QUrl url);
	void on_actionDownload_triggered();
	void on_actionOpen_triggered();
	void on_actionExport_triggered();
	void on_actionSave_triggered();
	void on_actionMake_theme_triggered();
	void on_actionReset_triggered();
};
#endif // MAINWINDOW_H
