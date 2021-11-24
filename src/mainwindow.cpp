#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
	:QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_configFile("./config.json")
{
	ui->setupUi(this);
	loadConfiguration();
	QUrl imageUrl("https://wakfu.cdn.ankama.com/gamedata/theme/theme.json");
	m_fd = new FileDownloader(imageUrl, this);
	connect(m_fd, SIGNAL (downloaded(QUrl)), this, SLOT (loadJsonFromInternet(QUrl)));
}

MainWindow::~MainWindow()
{
	delete ui;
	delete m_fd;
}

void MainWindow::loadConfiguration()
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
	m_wakfuJson = QJsonDocument::fromJson(m_fd->downloadedData());
    //create useful folders
    m_themesPath.mkpath(m_themesPath.absolutePath() + "/images/");

	//save the json file
    QFile file(m_themesPath.absolutePath() + "/theme.json", this);
    QFile jsonFile(file.fileName());
	jsonFile.open(QIODevice::WriteOnly);	//todo erreurs
	jsonFile.write(m_wakfuJson.toJson());
	jsonFile.close();

    const QJsonValue &texturesValue = m_wakfuJson["textures"]; //todo erreurs
    const QJsonArray &textures = texturesValue.toArray();
	QFile textureFile;
	//create directory if it does not exist
	for(QJsonArray::ConstIterator cit = textures.constBegin(); cit!=textures.constEnd(); ++cit)
	{
        const QJsonValue &value = (*cit);
        QString textureId = value["id"].toString();
		FileDownloader *fd = new FileDownloader("https://wakfu.cdn.ankama.com/gamedata/theme/images/" + textureId + ".png");
        connect(fd, &FileDownloader::downloaded, this, [&, fd](QUrl url2)
		{
			QPixmap buttonImage;
			buttonImage.loadFromData(fd->downloadedData());
            QFile file(m_themesPath.absolutePath() + "/images/" + url2.fileName(), this);
            if(!buttonImage.save(file.fileName(), "PNG", 100))
            {
                qDebug() << QString("Failed to save Image ") + file.fileName();
            }
			fd->deleteLater();
		});
	}
	return;
}
