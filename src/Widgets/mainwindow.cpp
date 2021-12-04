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
	m_pixmapRelativePath("/pixmaps/"),
	m_configFile("./config.json"),
	m_extraThemes(),
	m_defaultThemeWidget(nullptr),
	m_extraThemeWidgets()
{
	ui->setupUi(this);
	loadConfigurationFile();
	loadAllThemes();

	QWidget *centralWidget = ui->centralwidget;
	QLayout *l = centralWidget->layout();
	clearLayout(l);

	ui->scrollArea_stash = new ScrollArea(centralWidget);
	ui->scrollArea_used = new ScrollArea(centralWidget);

	ui->scrollArea_stash->setObjectName(QString::fromUtf8("scrollArea_stash"));
	ui->scrollArea_stash->setMinimumSize(QSize(128, 0));
	ui->scrollArea_stash->setAcceptDrops(true);
	ui->scrollArea_stash->setWidgetResizable(true);
	ui->scrollArea_stash->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	l->addWidget(ui->scrollArea_stash);

	ui->scrollArea_used->setObjectName(QString::fromUtf8("scrollArea_used"));
	ui->scrollArea_used->setMinimumSize(QSize(128, 0));
	ui->scrollArea_used->setAcceptDrops(true);
	ui->scrollArea_used->setWidgetResizable(true);
	ui->scrollArea_used->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	l->addWidget(ui->scrollArea_used);

	//custom scroll area content widget
	ui->scrollAreaWidgetContents_stash = new ScrollAreaContent(ui->scrollArea_stash);
	ui->scrollAreaWidgetContents_used = new ScrollAreaContent(ui->scrollArea_used);

	//Layout in scroll areas
	ui->scrollAreaWidgetContents_stash->setLayout(new QBoxLayout(QBoxLayout::TopToBottom, ui->scrollAreaWidgetContents_stash));
	ui->scrollAreaWidgetContents_used->setLayout(new QBoxLayout(QBoxLayout::TopToBottom, ui->scrollAreaWidgetContents_used));
	ui->scrollAreaWidgetContents_stash->layout()->setAlignment(Qt::AlignTop);
	ui->scrollAreaWidgetContents_used->layout()->setAlignment(Qt::AlignTop);

	//set scroll area custom widget
	ui->scrollArea_stash->setWidget(ui->scrollAreaWidgetContents_stash);
	ui->scrollArea_used->setWidget(ui->scrollAreaWidgetContents_used);

	ui->scrollArea_stash->horizontalScrollBar()->setEnabled(false);
	ui->scrollArea_used->horizontalScrollBar()->setEnabled(false);

	ui->scrollAreaWidgetContents_stash->layout()->setContentsMargins(4, 4, 4, 4);

	ui->scrollAreaWidgetContents_stash->setAcceptDrops(true);
	ui->scrollAreaWidgetContents_used->setAcceptDrops(true);
	createAllThemeWidgets();
}

MainWindow::~MainWindow()
{
	delete ui;
	delete m_fd;
}

bool MainWindow::loadConfigurationFile()
{
	m_configFile.open(QIODevice::ReadOnly | QIODevice::Text);
	bool b;
	if((b = m_configFile.isOpen()))
	{
		QString configFileContent = m_configFile.readAll();
		m_configFile.close();
		QJsonDocument configFileJson = QJsonDocument::fromJson(configFileContent.toUtf8());
		m_jsonUrl.setUrl(configFileJson["json_url"].toString());
		m_themesPath.setPath(configFileJson["themes_path"].toString());
		m_defaultThemePath.setPath(m_themesPath.absolutePath() + "/default");
	}
	return b;
}

void MainWindow::loadAllThemes()
{
	QFileInfoList ls = m_themesPath.entryInfoList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot);
	m_extraThemes.reserve(ls.size()-1);
	for(QFileInfoList::ConstIterator cit = ls.constBegin(); cit != ls.constEnd(); ++cit)
	{
		QDir dir;
		dir.setPath((*cit).absoluteFilePath());
		if(dir.exists())
		{
			if(dir == m_defaultThemePath)
			{
				m_defaultTheme.load(m_defaultThemePath);
			}
			else
			{
				Theme theme;
				theme.load(dir.absolutePath());
				m_extraThemes.push_back(theme);
			}
		}
	}
}

void MainWindow::createAllThemeWidgets()
{
	clearLayout(ui->scrollAreaWidgetContents_stash->layout());
	clearLayout(ui->scrollAreaWidgetContents_used->layout());
	delete m_defaultThemeWidget;
	for(std::vector<ThemeWidget *>::iterator it = m_extraThemeWidgets.begin(); it != m_extraThemeWidgets.end(); ++it)
	{
		delete (*it);
	}
	m_defaultThemeWidget = new ThemeWidget(&m_defaultTheme, ui->scrollAreaWidgetContents_stash);
	m_extraThemeWidgets.reserve(m_extraThemes.size());
	ui->scrollAreaWidgetContents_stash->layout()->addWidget(m_defaultThemeWidget);
	for(std::vector<Theme>::const_iterator cit = m_extraThemes.begin(); cit != m_extraThemes.end(); ++cit)
	{
		ThemeWidget *extraTheme = new ThemeWidget(&(*cit), ui->scrollAreaWidgetContents_stash);
		m_extraThemeWidgets.push_back(extraTheme);
		ui->scrollAreaWidgetContents_stash->layout()->addWidget(extraTheme);
	}
	return;
}

//STATIC

void MainWindow::clearLayout(QLayout *layout)
{
	if (layout == nullptr)
		return;
	QLayoutItem *item;
	while((item = layout->takeAt(0)))
	{
		if (item->layout())
		{
			clearLayout(item->layout());
			delete item->layout();
		}
		if (item->widget())
		{
		   delete item->widget();
		}
		delete item;
	}
}

//SLOTS

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
					//m_defaultTheme.unpack();
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

void MainWindow::on_actionSave_triggered()
{
	ThemeWidget *tw = new ThemeWidget(&m_defaultTheme, ui->scrollAreaWidgetContents_stash);
	QScrollArea *scrollArea = ui->scrollArea_stash;
	QWidget *scrollAreaContent = scrollArea->widget();
	QLayout *layout = scrollAreaContent->layout();
	layout->addWidget(tw);
}

