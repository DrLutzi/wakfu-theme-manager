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
	m_extraThemeWidgets(),
	m_progressBar(nullptr)
{
	ui->setupUi(this);
	makeProgressBar();
	setActionIcons();
	loadConfigurationFile();
	initJson();
	loadAllThemes();
    createScrollAreas();
	createAllExtraThemeWidgets();
}

MainWindow::~MainWindow()
{
	delete ui;
	delete m_fd;
}

void MainWindow::makeProgressBar()
{
	m_progressBar = new QProgressBar(ui->statusbar);
	m_progressBar->setBaseSize(32, 10);
	m_progressBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_progressBar->setMinimum(0);
	m_progressBar->setMaximum(100);
	m_progressBar->setTextVisible(false);
	ui->statusbar->addPermanentWidget(m_progressBar);
}

bool MainWindow::setActionIcons()
{
	ui->actionDownload->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowDown));
	ui->actionReset->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogResetButton));
	ui->actionMake_theme->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
	ui->actionOpen->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirIcon));
	return true;
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
		m_parameters.jsonUrl.setUrl(configFileJson["json_url"].toString());
		m_parameters.themesPath.setPath(configFileJson["themes_path"].toString());
		m_parameters.outputPath.setPath(configFileJson["output_path"].toString());
		m_defaultThemePath.setPath(m_parameters.themesPath.absolutePath() + "/default");
	}
	return b;
}

void MainWindow::checkOutputExistence()
{
	if(!m_parameters.outputPath.exists())
	{
		QMessageBox::information(this, tr("Information"), tr("The program could not find the \"theme\" folder of Wakfu at its usual place. Please provide it to continue."));
		QDir str = QFileDialog::getExistingDirectory(this, tr("Choose the \"theme\" or \"zaap\" folder of your game"), QDir::homePath());
		if(str.dirName() == "zaap")
		{
			str.setPath(str.absolutePath() + "/wakfu/theme");
		}
		else if(str.dirName() != "theme")
		{
			ui->statusbar->showMessage(tr("Warning: the provided folder is not \"theme\" or \"zaap\"."));
		}
		m_parameters.outputPath.setPath(str.absolutePath());
		saveConfigurationFile();
	}
}

bool MainWindow::saveConfigurationFile()
{
	m_configFile.open(QIODevice::ReadWrite | QIODevice::Text);
	bool b;
	if((b = m_configFile.isOpen()))
	{
		QString configFileContent = m_configFile.readAll();
		QJsonDocument configFileJson = QJsonDocument::fromJson(configFileContent.toUtf8());
		QJsonObject objectJson = configFileJson.object();
		objectJson["json_url"] = m_parameters.jsonUrl.url();
		objectJson["themes_path"] = m_parameters.themesPath.absolutePath();
		objectJson["output_path"] = m_parameters.outputPath.absolutePath();
		configFileJson.setObject(objectJson);
		m_configFile.write(configFileJson.toJson());
		m_configFile.close();
		ui->statusbar->showMessage(tr("Configuration file saved successfully."));
	}
	else
	{
		ui->statusbar->showMessage(tr("Warning: unable to save settings (unknown error)."));
	}
	return b;
}

void MainWindow::resetDefaultThemeWidget()
{
	if(m_defaultThemeWidget != nullptr)
	{
		if(ui->scrollAreaWidgetContents_stash && ui->scrollAreaWidgetContents_stash->layout())
		{
			ui->scrollAreaWidgetContents_stash->layout()->removeWidget(m_defaultThemeWidget);
		}
		if(ui->scrollAreaWidgetContents_used && ui->scrollAreaWidgetContents_used->layout())
		{
			ui->scrollAreaWidgetContents_used->layout()->removeWidget(m_defaultThemeWidget);
		}
		delete m_defaultThemeWidget;
		m_defaultThemeWidget = nullptr;
	}
	if(m_defaultTheme.isInitialized())
	{
		m_defaultThemeWidget = new ThemeWidget(&m_defaultTheme, ui->scrollAreaWidgetContents_stash);
		m_defaultThemeWidget->setTransparentAspect(true);
		m_defaultThemeWidget->setEnabled(false);
	}
	if(m_defaultThemeWidget != nullptr)
	{
		ui->scrollAreaWidgetContents_stash->layout()->addWidget(m_defaultThemeWidget);
	}
}

void MainWindow::loadAllThemes()
{
	ui->actionDownload->setEnabled(false);
	ui->actionMake_theme->setEnabled(false);
	ui->actionReset->setEnabled(false);
	QFileInfoList ls = m_parameters.themesPath.entryInfoList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot);
    m_extraThemes.reserve(std::max(0, int(ls.size())-1));
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
				createOneTheme(dir);
			}
		}
	}
	ui->actionDownload->setEnabled(true);
	ui->actionMake_theme->setEnabled(true);
	ui->actionReset->setEnabled(true);
    return;
}

void MainWindow::createScrollAreas()
{
    QWidget *centralWidget = ui->centralwidget;
	QLayout *l1 = ui->widget_scrollArea_stash->layout();
	QLayout *l2 = ui->widget_scrollArea_used->layout();

	l1->removeWidget(ui->scrollArea_stash);
	delete(ui->scrollArea_stash);
	l2->removeWidget(ui->scrollArea_used);
	delete(ui->scrollArea_used);

	ui->scrollArea_stash = new ScrollArea(centralWidget);
	ui->scrollArea_used = new ScrollArea(centralWidget);

    ui->scrollArea_stash->setObjectName(QString::fromUtf8("scrollArea_stash"));
    ui->scrollArea_stash->setMinimumSize(QSize(128, 0));
    ui->scrollArea_stash->setAcceptDrops(true);
    ui->scrollArea_stash->setWidgetResizable(true);
    ui->scrollArea_stash->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	l1->addWidget(ui->scrollArea_stash);

    ui->scrollArea_used->setObjectName(QString::fromUtf8("scrollArea_used"));
    ui->scrollArea_used->setMinimumSize(QSize(128, 0));
    ui->scrollArea_used->setAcceptDrops(true);
    ui->scrollArea_used->setWidgetResizable(true);
    ui->scrollArea_used->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	l2->addWidget(ui->scrollArea_used);

    //custom scroll area content widget
	ScrollAreaContent *stashContent = new ScrollAreaContent(ui->scrollArea_stash);
	ui->scrollAreaWidgetContents_stash = stashContent;
	ScrollAreaContent *usedContent = new ScrollAreaContent(ui->scrollArea_used);
	ui->scrollAreaWidgetContents_used = usedContent;

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
    ui->scrollAreaWidgetContents_used->layout()->setContentsMargins(4, 4, 4, 4);

	stashContent->setTwin(usedContent);
	usedContent->setTwin(stashContent);

    return;
}

void MainWindow::createAllExtraThemeWidgets()
{
	delete m_defaultThemeWidget;
	m_defaultThemeWidget = nullptr;
	clearLayout(ui->scrollAreaWidgetContents_stash->layout());
	clearLayout(ui->scrollAreaWidgetContents_used->layout());
	for(std::vector<ThemeWidget *>::iterator it = m_extraThemeWidgets.begin(); it != m_extraThemeWidgets.end(); ++it)
	{
        delete (*it);
    }
    if(m_defaultTheme.isInitialized())
    {
        m_defaultThemeWidget = new ThemeWidget(&m_defaultTheme, ui->scrollAreaWidgetContents_stash);
		m_defaultThemeWidget->setTransparentAspect(true);
		m_defaultThemeWidget->setEnabled(false);
    }
	m_extraThemeWidgets.reserve(m_extraThemes.size());
    if(m_defaultThemeWidget != nullptr)
    {
        ui->scrollAreaWidgetContents_stash->layout()->addWidget(m_defaultThemeWidget);
    }
	for(std::vector<Theme>::iterator it = m_extraThemes.begin(); it != m_extraThemes.end(); ++it)
	{
		Theme &theme = *it;
		createOneExtraThemeWidget(theme);
	}
	return;
}

Theme &MainWindow::createOneTheme(const QDir &dir)
{
	m_extraThemes.emplace_back();
	Theme &back = m_extraThemes.back();
	back.load(dir.absolutePath());
	return back;
}

void MainWindow::createOneExtraThemeWidget(Theme &theme)
{
	ThemeWidget *extraTheme = new ThemeWidget(&theme, ui->scrollAreaWidgetContents_stash);
	m_extraThemeWidgets.push_back(extraTheme);
	ui->scrollAreaWidgetContents_stash->layout()->addWidget(extraTheme);
}

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
    return;
}

void MainWindow::initJson(bool forceReset)
{
	if(_jsonThemes.isEmpty() || forceReset)
	{
		QFile jsonThemeFile(m_parameters.themesPath.absolutePath() + "/theme.json");
		bool b = jsonThemeFile.open(QIODevice::ReadOnly);
		if(b)
		{
			_jsonThemes = QJsonDocument::fromJson(jsonThemeFile.readAll());
			const QJsonArray &textureArray = _jsonThemes["textures"].toArray();
			Texture::initPathToIdMapFromJson(textureArray);
			jsonThemeFile.close();
		}
		else
		{
			on_actionDownload_triggered();
		}
	}
}

//SLOTS

void MainWindow::downloadDefault()
{
	m_progressBar->setValue(0);
	QJsonParseError *jsonPE = new QJsonParseError;
	_jsonThemes = QJsonDocument::fromJson(m_fd->downloadedData(), jsonPE);
	//create useful folders
	QDir defaultThemeImagePath(m_defaultThemePath.absolutePath() + "/images");
	defaultThemeImagePath.mkpath(defaultThemeImagePath.absolutePath());

	//save the json file
	QFile jsonFile(m_parameters.themesPath.absolutePath() + "/theme.json", this);
	jsonFile.open(QIODevice::WriteOnly);	//todo erreurs
	jsonFile.write(_jsonThemes.toJson());
	jsonFile.close();

	const QJsonValue &texturesValue = _jsonThemes["textures"]; //todo erreurs
	const QJsonArray &textures = texturesValue.toArray();
	QFile textureFile;
	//create directory if it does not exist
	nbThreads = textures.size();
	unsigned int maxProgress = nbThreads;
	if(nbThreads == 0)
	{
		ui->statusbar->showMessage(QString(tr("Error while attempting to download files!")));
	}
	else
	{
		ui->statusbar->showMessage(QString(tr("Downloading files... ")) + QString::number(nbThreads) + tr(" left"));
		for(QJsonArray::ConstIterator cit = textures.constBegin(); cit!=textures.constEnd(); ++cit)
		{
			const QJsonValue &value = (*cit);
			QString texturePath = value["path"].toString();
			QFileInfo fileInfo(texturePath);
			texturePath = fileInfo.baseName() + ".png";
			FileDownloader *fd = new FileDownloader("https://wakfu.cdn.ankama.com/gamedata/theme/images/" + texturePath);
			this->connect(fd, &FileDownloader::downloaded, this, [&, fd, maxProgress](QUrl url2)
			{
				QDir defaultThemeImagePath(m_defaultThemePath.absolutePath() + "/images");
				QPixmap textureImage;
				textureImage.loadFromData(fd->downloadedData());
				QFile file(defaultThemeImagePath.absolutePath() + "/" + url2.fileName(), this);
				if(!textureImage.save(file.fileName(), "PNG"))
				{
					qDebug() << QString(tr("Failed to save Image ")) + file.fileName();
				}
				fd->deleteLater();
				std::lock_guard<std::mutex> guard(nbThreads_mutex);
				--nbThreads;
				m_progressBar->setValue(float(maxProgress-nbThreads)/maxProgress * 80);
				ui->statusbar->showMessage(QString(tr("Downloading files... ")) + QString::number(nbThreads) + " left");
				if(nbThreads == 0)
				{
					//Load default theme
					QDir defaultTheme(m_defaultThemePath.absolutePath());
					if(defaultTheme.exists())
					{
						m_defaultTheme.load(defaultTheme);
						//m_defaultTheme.unpack();
					}
					const QJsonArray &textureArray = _jsonThemes["textures"].toArray();
					Texture::initPathToIdMapFromJson(textureArray);
					ui->statusbar->showMessage(QString(tr("All files were downloaded successfully.")));
					resetDefaultThemeWidget();
					m_progressBar->setValue(100);
					setAllWidgetsEnabled(true);
				}
			});
			fd->launchDownload();
		}
	}
}

void MainWindow::loadJsonFromInternet(QUrl url)
{
	(void) url;

	ui->actionDownload->setEnabled(false);
	ui->actionMake_theme->setEnabled(false);
	ui->actionReset->setEnabled(false);
	if(m_defaultThemeWidget != nullptr)
	{
		m_defaultThemeWidget->setEnabled(false);
	}

	downloadDefault();

	return;
}

void MainWindow::on_actionDownload_triggered()
{
	setAllWidgetsEnabled(false);

	QUrl imageUrl("https://wakfu.cdn.ankama.com/gamedata/theme/theme.json");
	m_fd = new FileDownloader(imageUrl, this);
	connect(m_fd, SIGNAL (downloaded(QUrl)), this, SLOT (loadJsonFromInternet(QUrl)));
	m_fd->launchDownload();
}


void MainWindow::on_actionOpen_triggered()
{
	auto findAppropriateSaveName = [] (QDir &directory)
	{
		QDir directory_copy(directory);
		bool canSave = false;
		int i = 0;
		while(!canSave)
		{
			if((canSave = !directory_copy.exists()))
			{
				directory = directory_copy;
			}
			else
			{
				directory_copy.setPath(directory.absolutePath() + " (" + QString::number(++i) + ")");
			}
		}
	};

	QString str = QFileDialog::getExistingDirectory(this, tr("Open a theme folder/a folder containing theme folders"), QDir::homePath());
	if(!str.isEmpty() && !str.isNull())
	{
		QDir dir(str);
		if(dir.exists())
		{
			//First check if this is a theme folder
			QDir dirColors(dir.absolutePath() + "/colors");
			QDir dirImages(dir.absolutePath() + "/images");
			if(dirColors.exists() || dirImages.exists())
			{
				//Case this is a theme folder
				QString dirEntryName(dir.dirName());
				Theme tmpTheme;
				tmpTheme.load(dir);
				QDir dirSaveTheme(m_parameters.themesPath.absolutePath() + "/" + dirEntryName);
				findAppropriateSaveName(dirSaveTheme);
				tmpTheme.save(dirSaveTheme);
				Theme &theme = createOneTheme(dirSaveTheme);
				createOneExtraThemeWidget(theme);
			}
			else
			{
				//Case this is a themes folder
				const QStringList lsDir = dir.entryList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot);
				for(const QString &strEntry : lsDir)
				{
					QDir dirEntry(dir.absolutePath() + "/" + strEntry);
					QDir dirColors(dirEntry.absolutePath() + "/colors");
					QDir dirImages(dirEntry.absolutePath() + "/images");
					if(dirColors.exists() || dirImages.exists())
					{
						//Case this is a theme folder
						QString dirEntryName(dirEntry.dirName());
						Theme tmpTheme;
						tmpTheme.load(QDir(dirEntry));
						QDir dirSaveTheme(m_parameters.themesPath.absolutePath() + "/" + dirEntryName);
						findAppropriateSaveName(dirSaveTheme);
						tmpTheme.save(dirSaveTheme);
						Theme &theme = createOneTheme(dirSaveTheme);
						createOneExtraThemeWidget(theme);
					}
				}
			}
		}
	}
    return;
}


void MainWindow::on_actionExport_triggered()
{
	QDir defaultThemeImages(m_defaultThemePath.absolutePath() + "/images");
	if(defaultThemeImages.exists())
	{
		m_defaultTheme.savePixmaps(m_defaultThemePath.absolutePath() + m_pixmapRelativePath);
	}
    return;
}

void MainWindow::on_actionSave_triggered()
{
    ThemeWidget *tw = new ThemeWidget(&m_defaultTheme, ui->scrollAreaWidgetContents_stash);
    QScrollArea *scrollArea = ui->scrollArea_stash;
    QWidget *scrollAreaContent = scrollArea->widget();
    QLayout *layout = scrollAreaContent->layout();
    layout->addWidget(tw);
    return;
}

void MainWindow::makeTheme()
{
	if(!m_defaultTheme.isOpened())
	{
		m_defaultTheme.load(m_defaultThemePath);
	}
	if(!m_defaultTheme.isUnpacked())
	{
		m_defaultTheme.unpack();
	}
	m_outputTheme.copyTextures(m_defaultTheme);
	QString messageMakingTheme = tr("Making theme... ");
	ui->statusbar->showMessage(messageMakingTheme);
	//v most likely not in the right order, look for widgets in layout
	int layoutCount = ui->scrollAreaWidgetContents_used->layout()->count();

	Theme taggerTheme;
	taggerTheme.copyTextures(m_defaultTheme);
	taggerTheme.resetTextures();

	for(int i=layoutCount-1; i>=0; --i)
	{
		QWidget *widget = ui->scrollAreaWidgetContents_used->layout()->itemAt(i)->widget();
		ThemeWidget *tw = dynamic_cast<ThemeWidget *>(widget);
		if(tw != nullptr)
		{
			Theme *theme = tw->theme();
			assert(theme != nullptr);
			ui->statusbar->showMessage(QString(tr("Loading ")) + tw->name() + "...");
			if(!theme->isOpened())
			{
				theme->load(theme->path());
			}
			ui->statusbar->showMessage(QString(tr("Extracting ")) + tw->name() + "...");
			if(!theme->isUnpacked())
			{
				theme->unpack(&m_defaultTheme);
			}
			ui->statusbar->showMessage(QString(tr("Applying pixmaps of ")) + tw->name() + "...");
			m_outputTheme.pack(theme, &taggerTheme, true);
		}
		m_progressBar->setValue(int( float(layoutCount-i)/(layoutCount) * 50));
	}
	for(int i=0; i<layoutCount; ++i)
	{
		QWidget *widget = ui->scrollAreaWidgetContents_used->layout()->itemAt(i)->widget();
		ThemeWidget *tw = dynamic_cast<ThemeWidget *>(widget);
		if(tw != nullptr)
		{
			Theme *theme = tw->theme();
			ui->statusbar->showMessage(QString(tr("Applying orphan pixmaps of ")) + tw->name() + "...");
			m_outputTheme.pack(theme, &taggerTheme, false);
		}
		if(layoutCount == 1)
			m_progressBar->setValue(int(100));
		else
			m_progressBar->setValue(50 + int( float(i)/(layoutCount-1) * 50));
	}
	m_outputTheme.save(m_parameters.outputPath);
	ui->statusbar->showMessage(QString(tr("Theme compiled.")));
	setAllWidgetsEnabled(true);
}

void MainWindow::on_actionMake_theme_triggered()
{
	m_progressBar->setValue(0);
	setAllWidgetsEnabled(false);
	initJson();
	if(_jsonThemes.isEmpty())
	{
		ui->statusbar->showMessage(QString(tr("Error: theme json file could not be opened (did you download it first?).")));
	}
	else
	{
		QThread *thread = QThread::create([this]() {makeTheme();});
		thread->setParent(this);
		connect(thread, &QThread::finished, thread, &QObject::deleteLater);
		thread->start();
	}
}

void MainWindow::resetTheme()
{
	ui->statusbar->showMessage(QString(tr("Restoring default theme...")));
	if(!m_defaultTheme.isOpened())
	{
		m_defaultTheme.load(m_defaultThemePath);
	}
	if(m_defaultTheme.isOpened())
	{
		m_defaultTheme.save(m_parameters.outputPath);
		ui->statusbar->showMessage(QString(tr("Finished restoring default theme.")));
	}
	else
	{
		ui->statusbar->showMessage(QString(tr("Unable to restore default theme (did you download it first?).")));
	}
	setAllWidgetsEnabled(true);
}

void MainWindow::on_actionReset_triggered()
{
	setAllWidgetsEnabled(false);

	QThread *thread = QThread::create([this]() {resetTheme();});
	thread->setParent(this);
	connect(thread, &QThread::started, m_progressBar, [this]{m_progressBar->setValue(20);});
	connect(thread, &QThread::finished, m_progressBar, [this]{m_progressBar->setValue(100);});
	connect(thread, &QThread::finished, thread, &QObject::deleteLater);
	thread->start();
}

void MainWindow::setAllWidgetsEnabled(bool b)
{
	ui->actionDownload->setEnabled(b);
	ui->actionMake_theme->setEnabled(b);
	ui->actionReset->setEnabled(b);
	ui->actionParameters->setEnabled(b);
	ui->widget_scrollArea_stash->setEnabled(b);
	ui->widget_scrollArea_used->setEnabled(b);
}

void MainWindow::on_actionParameters_triggered()
{
	FormParameters *formParameters = new FormParameters(m_parameters, this);
	formParameters->show();
}

AppParameters *MainWindow::parameters()
{
	return &m_parameters;
}

void MainWindow::notifyThemesPathChanged()
{
	for(auto &tw : m_extraThemeWidgets)
	{
		tw->deleteLater();
	}
	m_extraThemeWidgets.clear();
	m_extraThemes.clear();
	if(!m_defaultTheme.isInitialized())
	{
		m_defaultThemePath.setPath(m_parameters.themesPath.absolutePath() + "/default");
		on_actionDownload_triggered();
	}
	loadAllThemes();
	createAllExtraThemeWidgets();
}

void MainWindow::notifyParametersChanged()
{
	saveConfigurationFile();
}
