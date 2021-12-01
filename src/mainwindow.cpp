#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

QJsonDocument _jsonThemes;
int nbThreads;
std::mutex nbThreads_mutex;


MainWindow::MainWindow(QWidget *parent)
	:QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_fd(nullptr),
	m_configFile("./config.json"),
	m_pixmapRelativePath("/pixmaps/")
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
	m_themesPath.setPath(configFileJson["themes_path"].toString()); //todo erreurs
	m_defaultThemePath.setPath(m_themesPath.absolutePath() + "/default");
	return;
}

void MainWindow::loadJsonFromInternet(QUrl url)
{
	_jsonThemes = QJsonDocument::fromJson(m_fd->downloadedData());
    //create useful folders
	QDir defaultThemeImagePath(m_defaultThemePath.absolutePath() + "/images");
	m_defaultThemePath.mkpath(defaultThemeImagePath.absolutePath());

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
			QDir defaultThemeImagePath(m_defaultThemePath.absolutePath() + "/images");
			QPixmap buttonImage;
			buttonImage.loadFromData(fd->downloadedData());
			QFile file(defaultThemeImagePath.absolutePath() + "/" + url2.fileName(), this);
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
				QDir defaultTheme(defaultThemeImagePath.absolutePath());
				if(defaultTheme.exists())
				{
					m_defaultTheme.load(defaultTheme);
					m_defaultTheme.unpack();
				}
			}
		});
	}
	const QJsonArray &textureArray = _jsonThemes["textures"].toArray();
	Texture::initPathToIdMapFromJson(textureArray);
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
	QFile jsonThemeFile(m_themesPath.absolutePath() + "/theme.json");
	jsonThemeFile.open(QIODevice::ReadOnly);
	_jsonThemes = QJsonDocument::fromJson(jsonThemeFile.readAll());
	const QJsonArray &textureArray = _jsonThemes["textures"].toArray();
	Texture::initPathToIdMapFromJson(textureArray);
	QDir defaultTheme(m_defaultThemePath.absolutePath() + "/images");
	if(defaultTheme.exists())
	{
		m_defaultTheme.load(defaultTheme);
		m_defaultTheme.unpack();
	}
}


void MainWindow::on_actionExport_triggered()
{
	QDir defaultThemeImages(m_defaultThemePath.absolutePath() + "/images");
	if(defaultThemeImages.exists())
	{
		m_defaultTheme.savePixmaps(m_defaultThemePath.absolutePath() + m_pixmapRelativePath);
	}
}

