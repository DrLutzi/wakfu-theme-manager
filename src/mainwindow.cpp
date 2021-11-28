#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

QJsonDocument _jsonThemes;
int nbThreads;
std::mutex nbThreads_mutex;


MainWindow::MainWindow(QWidget *parent)
	:QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_configFile("./config.json")
{
	ui->setupUi(this);
	loadConfigurationFile();
}

MainWindow::~MainWindow()
{
	delete ui;
	delete m_fd;
}

void MainWindow::loadConfigurationFile()
{
	m_configFile.open(QIODevice::ReadOnly | QIODevice::Text); //todo erreurs
	QString configFileContent = m_configFile.readAll();
	m_configFile.close();
	QJsonDocument configFileJson = QJsonDocument::fromJson(configFileContent.toUtf8());
	m_jsonUrl.setUrl(configFileJson["json_url"].toString()); //todo erreurs
	m_themesPath.setPath(configFileJson["output_path"].toString()); //todo erreurs
	return;
}

void MainWindow::loadJsonFromInternet(QUrl url)
{
	_jsonThemes = QJsonDocument::fromJson(m_fd->downloadedData());
    //create useful folders
	QDir defaultThemePath(m_themesPath.absolutePath() + "/images/");
	m_themesPath.mkpath(defaultThemePath.absolutePath());

	//save the json file
	QFile jsonFile(m_themesPath.absolutePath() + "/theme.json", this);
	jsonFile.open(QIODevice::WriteOnly);	//todo erreurs
	jsonFile.write(_jsonThemes.toJson());
	jsonFile.close();

	const QJsonValue &texturesValue = _jsonThemes["textures"]; //todo erreurs
    const QJsonArray &textures = texturesValue.toArray();
	QFile textureFile;
	//create directory if it does not exist
	nbThreads = textures.size();
	for(QJsonArray::ConstIterator cit = textures.constBegin(); cit!=textures.constEnd(); ++cit)
	{
        const QJsonValue &value = (*cit);
		QString texturePath = value["path"].toString();
		QFileInfo fileInfo(texturePath);
		texturePath = fileInfo.baseName() + ".png";
		FileDownloader *fd = new FileDownloader("https://wakfu.cdn.ankama.com/gamedata/theme/images/" + texturePath);
		connect(fd, &FileDownloader::downloaded, this, [&, fd](QUrl url2)
		{
			QPixmap buttonImage;
			buttonImage.loadFromData(fd->downloadedData());
			QFile file(m_themesPath.absolutePath() + "/images/" + url2.fileName(), this);
			if(!buttonImage.save(file.fileName(), "PNG"))
			{
				qDebug() << QString("Failed to save Image ") + file.fileName();
			}
			fd->deleteLater();
			std::lock_guard<std::mutex> guard(nbThreads_mutex);
			--nbThreads;
			if(nbThreads == 0)
			{
				//Load default theme
				QDir defaultTheme(m_themesPath.absolutePath() + "/images");
				if(defaultTheme.exists())
				{
					m_defaultTheme.load(defaultTheme);
				}
			}
		});
	}
	return;
}

void MainWindow::on_actionDownload_triggered()
{
	QUrl imageUrl("https://wakfu.cdn.ankama.com/gamedata/theme/theme.json");
	m_fd = new FileDownloader(imageUrl, this);
	connect(m_fd, SIGNAL (downloaded(QUrl)), this, SLOT (loadJsonFromInternet(QUrl)));
}


void MainWindow::on_actionOpen_triggered()
{
}

