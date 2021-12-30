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
	connect(this, &MainWindow::messageUpdateRequired, ui->statusbar, &QStatusBar::showMessage);
	connect(this, &MainWindow::extraThemeWidgetCreationOrUpdateRequired, this, &MainWindow::createOrUpdateOneExThemeWidget);
	connect(this, &MainWindow::openThemeRequired, this, &MainWindow::on_openThemeRequired);
	connect(this, &MainWindow::defaultThemeWidgetCreationRequired, this, &MainWindow::createDefaultThemeWidget);
	connect(this, &MainWindow::updateFromThemesDirRequired, this, &MainWindow::updateFromThemesDir);
	makeProgressBar();
    createScrollAreas();
	loadConfigurationFile();
	initJson();
	importAllThemesThreaded();
}

MainWindow::~MainWindow()
{
	delete ui;
	delete m_fd;
    for(Theme *t : m_extraThemes)
    {
        delete t;
    }
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
	connect(this, &MainWindow::progressUpdateRequired, m_progressBar, &QProgressBar::setValue);
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
        QString output_path(configFileJson["output_path"].toString());
        if(output_path.isNull() || output_path.isEmpty())
        {
			output_path = _defaultThemeDirectory;
            m_parameters.outputPath.setPath(output_path);
            saveConfigurationFile();
        }
        else
        {
            m_parameters.outputPath.setPath(output_path);
        }
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
			emit messageUpdateRequired(tr("Warning: the provided folder is not \"theme\" or \"zaap\"."));
		}
		m_parameters.outputPath.setPath(str.absolutePath());
		saveConfigurationFile();
	}
}

void MainWindow::importAllThemes()
{
	setAllWidgetsEnabled(false);
	QFileInfoList ls = m_parameters.themesPath.entryInfoList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot);
	m_extraThemes.reserve(std::max(0, int(ls.size())-1)); //TODO: add more space? Change to std::list
	int i=0;
	for(QFileInfoList::ConstIterator cit = ls.constBegin(); cit != ls.constEnd(); ++cit, ++i)
	{
		QDir dir;
		dir.setPath((*cit).absoluteFilePath());
		if(dir.exists())
		{
			if(dir == m_defaultThemePath)
			{
				m_defaultTheme.load(m_defaultThemePath);
				emit progressUpdateRequired(float(i+1)/ls.size() * 100);
			}
			else
			{
				if(themeFolderIsValid(dir))
					importOneTheme(dir);
			}
		}
	}
	setAllWidgetsEnabled(false);
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

void MainWindow::importAllThemesThreaded()
{
	QThread *threadWidgets = QThread::create([this]() {createAllExtraThemeWidgets();});
	connect(threadWidgets, &QThread::started, m_progressBar, [this]{m_progressBar->setValue(0);});
	connect(threadWidgets, &QThread::finished, m_progressBar, [this]{m_progressBar->setValue(100);});
	connect(threadWidgets, &QThread::finished, this, &MainWindow::enableAllWidgets);
	connect(threadWidgets, &QThread::finished, threadWidgets, &QObject::deleteLater);
	connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, threadWidgets, &QThread::quit);

	QThread *threadThemes = QThread::create([this]() {importAllThemes();});
	connect(threadThemes, &QThread::started, this, &MainWindow::disableAllWidgets);
	connect(threadThemes, &QThread::started, m_progressBar, [this]{m_progressBar->setValue(0);});
	connect(threadThemes, &QThread::finished, m_progressBar, [this]{m_progressBar->setValue(100);});
	connect(threadThemes, &QThread::finished, this, [threadWidgets]{
		threadWidgets->start();
	});
	connect(threadThemes, &QThread::finished, threadThemes, &QObject::deleteLater);
	connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, threadThemes, &QThread::quit);
	threadThemes->start();
}

Theme *MainWindow::importOneTheme(const QDir &dir)
{
	Theme *theme = nullptr;
	std::vector<Theme *>::iterator it;
	if((it = findTheme(dir.dirName())) == m_extraThemes.end())
	{
		theme = new Theme;
		m_extraThemes.push_back(theme);
		theme->import(dir);
	}
	else
	{
		Theme *theme = (*it);
		theme->import(dir);
	}
    return theme;
}

bool MainWindow::saveConfigurationFile()
{
    m_configFile.open(QIODevice::WriteOnly | QIODevice::Text);
    bool b;
    if((b = m_configFile.isOpen()))
    {
        QJsonObject objectJson
        {
            {"json_url", m_parameters.jsonUrl.url()},
            {"themes_path", m_parameters.themesPath.absolutePath()},
            {"output_path", m_parameters.outputPath.absolutePath()}
        };
        QJsonDocument configFileJson(objectJson);
        m_configFile.write(configFileJson.toJson());
        m_configFile.close();
		emit messageUpdateRequired(tr("Configuration file saved successfully."));
    }
    else
    {
		emit messageUpdateRequired(tr("Warning: unable to save settings (unknown error)."));
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
	if(m_defaultTheme.isImported())
    {
        createDefaultThemeWidget();
    }
}

void MainWindow::openTheme(QString str)
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

	//moves one up if the directory is "images" or "colors" (misunderstanding from the user).
	auto correctIfDirIsImagesOrColors = [&] (QDir &themeDir) -> bool
	{
		bool dirIsValid = true;
		if(themeDir.dirName() == "colors" || themeDir.dirName() == "images")
		{
			QDir dirColors(Theme::colorsDir(themeDir));
			QDir dirImages(Theme::imagesDir(themeDir));
			if(!dirColors.exists() && !dirImages.exists())
			{
				dirIsValid = themeDir.cdUp();
			}
		}
		return dirIsValid;
	};

	bool opIsSuccess=false;

	auto openAndImportExistingThemeDir = [&] (QDir &themeDir) -> bool
	{
		bool isThemeDir = false;
		if(themeFolderIsValid(themeDir))
		{
			isThemeDir = true;
			opIsSuccess=true;
			//Case this is a theme folder
			QString dirEntryName(themeDir.dirName());
			Theme tmpTheme;
			tmpTheme.load(themeDir);
			QDir dirSaveTheme(m_parameters.themesPath.absolutePath() + "/" + dirEntryName);
			findAppropriateSaveName(dirSaveTheme);
			tmpTheme.save(dirSaveTheme);
			Theme *theme = importOneTheme(dirSaveTheme);
			if(theme != nullptr)
			{
				emit extraThemeWidgetCreationOrUpdateRequired(theme);
			}
		}
		return isThemeDir;
	};

    if(!str.isEmpty() && !str.isNull())
    {
		QFileInfo fileInfo(str);
		if(fileInfo.isDir())
		{
			QDir dir(str);
			if(dir.exists())
			{
				//First check if this is a theme folder
				if(correctIfDirIsImagesOrColors(dir) && !openAndImportExistingThemeDir(dir))
				{
					emit progressUpdateRequired(0);
					const QStringList lsDir = dir.entryList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot);
					int i = 0;
					for(const QString &strEntry : lsDir)
					{
						QDir dirEntry(dir.absolutePath() + "/" + strEntry);
						openAndImportExistingThemeDir(dirEntry);
						emit progressUpdateRequired((float(++i)/lsDir.size())*100);
					}
				}
			}
			if(!opIsSuccess)
			{
				emit messageUpdateRequired(QString(tr("Error while attempting to open theme(s): no valid theme folder found.")));
			}
		}
		else if(fileInfo.isFile() && fileInfo.suffix() == "zip")
		{
			emit progressUpdateRequired(0);
			QFile file(str);
			opIsSuccess = file.open(QIODevice::ReadOnly);
			if(opIsSuccess)
			{
				file.close();
				emit progressUpdateRequired(20);
				if(opIsSuccess)
				{
					std::vector<Theme *>::iterator it;
					if((it = findTheme(fileInfo.baseName())) != m_extraThemes.end())
					{ //Theme already exists
						Theme *theme = (*it);
						opIsSuccess = theme->unzip(QFile(str));
						emit extraThemeWidgetCreationOrUpdateRequired(theme);
					}
					else
					{
						QDir themeDir = m_parameters.themesPath.absolutePath() + "/" + fileInfo.baseName();
						QDir colorsDirTrick(Theme::colorsDir(themeDir));
						colorsDirTrick.mkpath(colorsDirTrick.absolutePath());
						//the following function works only if colors or images exists
						Theme *theme = importOneTheme(themeDir);
						opIsSuccess = theme->unzip(QFile(str));
						emit extraThemeWidgetCreationOrUpdateRequired(theme);
					}
					emit progressUpdateRequired(100);
					if(opIsSuccess)
					{
						emit updateFromThemesDirRequired();
						emit messageUpdateRequired(tr("Successfully unzipped and imported theme archive."));
					}
					else
					{
						emit messageUpdateRequired(QString(tr("Error while attempting to open theme: unable to find or use an unzip program.")));
					}
				}
			}
			if(!opIsSuccess)
			{
				emit messageUpdateRequired(QString(tr("Error while attempting to open theme: unable to unzip file.")));
			}
		}
		else
		{
			emit messageUpdateRequired(QString(tr("Error while attempting to open theme(s): no .zip file or folder selected.")));
		}
    }
	emit progressUpdateRequired(100);
    return;
}

AppParameters *MainWindow::parameters()
{
	return &m_parameters;
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
		bool b = jsonThemeFile.open(QIODevice::ReadOnly) && m_defaultThemePath.exists();
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

void MainWindow::downloadDefault()
{
	emit progressUpdateRequired(0);
    QJsonParseError *jsonPE = new QJsonParseError;
    _jsonThemes = QJsonDocument::fromJson(m_fd->downloadedData(), jsonPE);
	//create useful folders
	QDir defaultThemeImagePath(Theme::imagesDir(m_defaultThemePath));
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
		emit messageUpdateRequired(QString(tr("Error while attempting to download files!")));
    }
    else
    {
		emit messageUpdateRequired(QString(tr("Downloading files... ")) + QString::number(nbThreads) + tr(" left"));
        for(QJsonArray::ConstIterator cit = textures.constBegin(); cit!=textures.constEnd(); ++cit)
        {
            const QJsonValue &value = (*cit);
            QString texturePath = value["path"].toString();
            QFileInfo fileInfo(texturePath);
            texturePath = fileInfo.baseName() + ".png";
            FileDownloader *fd = new FileDownloader("https://wakfu.cdn.ankama.com/gamedata/theme/images/" + texturePath);
			connect(fd, &FileDownloader::errorMsg, ui->statusbar, &QStatusBar::showMessage);
			connect(fd, &FileDownloader::downloaded, this, [&, fd, maxProgress](QUrl url2)
            {
				QDir defaultThemeImagePath(Theme::imagesDir(m_defaultThemePath));
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
				emit progressUpdateRequired(int(float(maxProgress-nbThreads)/maxProgress * 80));
				emit messageUpdateRequired(QString(tr("Downloading files... ")) + QString::number(nbThreads) + " left");
                if(nbThreads == 0)
                {
                    //Load default theme
                    QDir defaultTheme(m_defaultThemePath.absolutePath());
                    if(defaultTheme.exists())
                    {
                        m_defaultTheme.load(defaultTheme);
                    }
                    const QJsonArray &textureArray = _jsonThemes["textures"].toArray();
                    Texture::initPathToIdMapFromJson(textureArray);
					emit messageUpdateRequired(QString(tr("All files were downloaded successfully.")));
                    resetDefaultThemeWidget();
					emit progressUpdateRequired(100);
                    setAllWidgetsEnabled(true);
                }
            });
            fd->launchDownload();
        }
    }
}

void MainWindow::makeTheme()
{
	if(!m_defaultTheme.isLoaded())
    {
        m_defaultTheme.load(m_defaultThemePath);
    }
    if(!m_defaultTheme.isUnpacked())
    {
        m_defaultTheme.unpack();
    }
	resetTheme();
    m_outputTheme.copyTextures(m_defaultTheme);
	emit messageUpdateRequired(tr("Making theme... "));
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
			emit messageUpdateRequired(QString(tr("Loading ")) + tw->name() + "...");
			if(!theme->isLoaded())
            {
                theme->load(theme->path());
            }
			emit messageUpdateRequired(QString(tr("Extracting ")) + tw->name() + "...");
            if(!theme->isUnpacked())
            {
                theme->unpack(&m_defaultTheme);
            }
			emit messageUpdateRequired(QString(tr("Applying pixmaps of ")) + tw->name() + "...");
            m_outputTheme.pack(theme, &taggerTheme, true);
			theme->unload();
        }
		emit progressUpdateRequired(int( float(layoutCount-i)/(layoutCount) * 50));
    }
    for(int i=0; i<layoutCount; ++i)
    {
        QWidget *widget = ui->scrollAreaWidgetContents_used->layout()->itemAt(i)->widget();
        ThemeWidget *tw = dynamic_cast<ThemeWidget *>(widget);
        if(tw != nullptr)
        {
            Theme *theme = tw->theme();
			theme->load(theme->path());
			emit messageUpdateRequired(QString(tr("Applying orphan pixmaps of ")) + tw->name() + "...");
            m_outputTheme.pack(theme, &taggerTheme, false);
			theme->unload();
        }
        if(layoutCount == 1)
			emit progressUpdateRequired(100);
        else
			emit progressUpdateRequired(50 + int( float(i)/(layoutCount-1) * 50));
    }
    m_outputTheme.save(m_parameters.outputPath);
	m_outputTheme.unload();
	emit messageUpdateRequired(QString(tr("Theme compiled.")));
}

void MainWindow::resetTheme()
{
	emit messageUpdateRequired(QString(tr("Restoring default theme...")));
	if(!m_defaultTheme.isLoaded())
    {
        m_defaultTheme.load(m_defaultThemePath);
    }
	if(m_defaultTheme.isLoaded())
    {
		//m_defaultTheme.save(m_parameters.outputPath);
		m_defaultTheme.useToRemoveImagesIn(m_parameters.outputPath);
		emit messageUpdateRequired(QString(tr("Finished restoring default theme.")));
    }
    else
    {
		emit messageUpdateRequired(QString(tr("Unable to restore default theme (did you download it first?).")));
    }
}

std::vector<Theme *>::iterator MainWindow::findTheme(const QString &name)
{
	std::vector<Theme *>::iterator it = m_extraThemes.end();
	if(!name.isEmpty() && !name.isNull() && name != "default")
	{
		it = std::find_if(m_extraThemes.begin(), m_extraThemes.end(), [&] (Theme *theme) -> bool
		{
			return theme != nullptr && theme->name() == name;
		});
	}
	return it;
}

bool MainWindow::themeFolderIsValid(const QDir &dir)
{
	QDir colorsDir(Theme::colorsDir(dir));
	QDir imagesDir(Theme::imagesDir(dir));
	QFile remoteFile(Theme::remoteFile(dir));
	return ((imagesDir.exists() && !imagesDir.isEmpty()) || (colorsDir.exists() && !colorsDir.isEmpty()) || remoteFile.exists());
}

///SLOTS

void MainWindow::enableAllWidgets()
{
	setAllWidgetsEnabled(true);
}

void MainWindow::disableAllWidgets()
{
	setAllWidgetsEnabled(false);
}

void MainWindow::setAllWidgetsEnabled(bool b)
{
	ui->actionDownload->setEnabled(b);
	ui->actionMake_theme->setEnabled(b);
	ui->actionReset->setEnabled(b);
	ui->actionParameters->setEnabled(b);
	ui->actionOpen->setEnabled(b);
	ui->actionOpen_Zip->setEnabled(b);
	ui->widget_scrollArea_stash->setEnabled(b);
	ui->widget_scrollArea_used->setEnabled(b);
}

void MainWindow::updateFromThemesDir()
{
	QStringList ls = m_parameters.themesPath.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
	int i=0;
	for(QStringList::ConstIterator cit = ls.constBegin(); cit != ls.constEnd(); ++cit, ++i)
	{
		const QString &dirStr = (*cit);
		if(!dirStr.isEmpty() && !dirStr.isNull())
		{
			QDir dir(dirStr);
			QString dirName(dir.dirName());
			if(dirName == "default")
			{
				m_defaultTheme.load(m_defaultThemePath);
			}
			else
			{
				on_openThemeRequired(m_parameters.themesPath.absolutePath() + "/" + dirName);
			}
		}
		emit progressUpdateRequired(float(i+1)/ls.size() * 100);
	}
	emit messageUpdateRequired(tr("Successfully updated themes."));
}

void MainWindow::on_openThemeRequired(QDir dir)
{
	if(themeFolderIsValid(dir))
	{
		Theme *theme = importOneTheme(dir);
		if(theme != nullptr)
		{
			createOrUpdateOneExThemeWidget(theme);
		}
	}
}

void MainWindow::createDefaultThemeWidget()
{
	m_defaultThemeWidget = new ThemeWidget(&m_defaultTheme, ui->scrollAreaWidgetContents_stash);
	m_defaultThemeWidget->setTransparentAspect(true);
	m_defaultThemeWidget->setEnabled(false);
    m_defaultThemeWidget->setToolTip(tr("Default theme downloaded and imported correctly."));
	ui->scrollAreaWidgetContents_stash->layout()->addWidget(m_defaultThemeWidget);
}

ThemeWidget *MainWindow::createOrUpdateOneExThemeWidget(Theme *theme)
{
	ThemeWidget *extraTheme = nullptr;
    if(theme != nullptr)
    {
		std::vector<ThemeWidget *>::iterator it = std::find_if(m_extraThemeWidgets.begin(), m_extraThemeWidgets.end(), [&] (ThemeWidget *tw) -> bool
		{
				return tw && tw->theme() == theme;
	});
		if(it == m_extraThemeWidgets.end())
		{
			extraTheme = new ThemeWidget(theme, ui->scrollAreaWidgetContents_stash);
			m_extraThemeWidgets.push_back(extraTheme);
			ui->scrollAreaWidgetContents_stash->layout()->addWidget(extraTheme);
			if(!theme->pathHasContent() && theme->remoteIsValid())
			{ //case where the path does not have any content, but a remote is provided.
				extraTheme->downloadTheme();
			}
		}
		else
		{
			extraTheme = (*it);
			extraTheme->createOrUpdateStyle();
		}
    }
	return extraTheme;
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
	if(m_defaultTheme.isImported())
	{
		emit defaultThemeWidgetCreationRequired();
	}
	m_extraThemeWidgets.reserve(m_extraThemes.size());
	for(std::vector<Theme *>::iterator it = m_extraThemes.begin(); it != m_extraThemes.end(); ++it) //TODO : should be const iterator
	{
		Theme *&theme = *it;
		emit extraThemeWidgetCreationOrUpdateRequired(theme);
	}
	return;
}

void MainWindow::loadJsonFromInternet()
{
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

	QUrl jsonUrl(m_parameters.jsonUrl);
	m_fd = new FileDownloader(jsonUrl, this);
	connect(m_fd, &FileDownloader::downloaded, this, &MainWindow::loadJsonFromInternet); //TODO : connect to deleteLater or make a setUrl function
	connect(m_fd, &FileDownloader::errorMsg, ui->statusbar, &QStatusBar::showMessage);
    m_fd->launchDownload();
}


void MainWindow::on_actionOpen_triggered()
{
    setAllWidgetsEnabled(false);
	QString str = QFileDialog::getExistingDirectory(this, tr("Open a theme folder/a folder containing theme folders"), QDir::homePath());
	QThread *thread = QThread::create([this, str]() {openTheme(str);});
	connect(thread, &QThread::finished, this, &MainWindow::enableAllWidgets);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
	connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, thread, &QThread::quit);
    thread->start();
}

void MainWindow::on_actionOpen_Zip_triggered()
{
	setAllWidgetsEnabled(false);
	QString str = QFileDialog::getOpenFileName(this, tr("Open a .zip archive containing a theme"), QDir::homePath(), "*.zip");
	QThread *thread = QThread::create([this, str]() {openTheme(str);});
	connect(thread, &QThread::finished, this, &MainWindow::enableAllWidgets);
	connect(thread, &QThread::finished, thread, &QObject::deleteLater);
	connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, thread, &QThread::quit);
	thread->start();
}

void MainWindow::on_actionExport_triggered()
{
	QDir defaultThemeImages(Theme::imagesDir(m_defaultThemePath));
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

void MainWindow::on_actionMake_theme_triggered()
{
	int layoutCount = ui->scrollAreaWidgetContents_used->layout()->count();
	if(layoutCount == 0)
	{
		on_actionReset_triggered();
	}
	else
	{
		emit progressUpdateRequired(0);
		setAllWidgetsEnabled(false);
		initJson();
		if(_jsonThemes.isEmpty())
		{
			emit messageUpdateRequired(QString(tr("Error: theme json file could not be opened (did you download it first?).")));
		}
		else
		{
			QThread *thread = QThread::create([this]() {makeTheme();});
			connect(thread, &QThread::finished, this, &MainWindow::enableAllWidgets);
			connect(thread, &QThread::finished, thread, &QObject::deleteLater);
			connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, thread, &QThread::quit);
			thread->start();
		}
	}
}

void MainWindow::on_actionReset_triggered()
{
    setAllWidgetsEnabled(false);

    QThread *thread = QThread::create([this]() {resetTheme();});
    connect(thread, &QThread::started, m_progressBar, [this]{m_progressBar->setValue(20);});
    connect(thread, &QThread::finished, m_progressBar, [this]{m_progressBar->setValue(100);});
	connect(thread, &QThread::finished, this, &MainWindow::enableAllWidgets);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
	connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, thread, &QThread::quit);
    thread->start();
}

void MainWindow::on_actionParameters_triggered()
{
    FormParameters *formParameters = new FormParameters(m_parameters, this);
	connect(formParameters, &FormParameters::notifyParametersChanged, this, &MainWindow::on_parametersChanged);
    formParameters->show();
}

void MainWindow::on_parametersChanged(AppParameters newParameters)
{
	setAllWidgetsEnabled(false);
	bool themesPathChanged = newParameters.themesPath != m_parameters.themesPath;
	m_parameters = newParameters;
	saveConfigurationFile();
	if(themesPathChanged)
	{
		for(auto &tw : m_extraThemeWidgets)
		{
			tw->deleteLater();
		}
		m_extraThemeWidgets.clear();
		for(Theme *t : m_extraThemes)
		{
			delete t;
		}
		m_extraThemes.clear();
		if(!m_defaultTheme.isImported())
		{
			m_defaultThemePath.setPath(m_parameters.themesPath.absolutePath() + "/default");
			on_actionDownload_triggered();
		}
		else
		{
			importAllThemesThreaded();
		}
	}
	setAllWidgetsEnabled(true);
}

void MainWindow::on_actionImport_From_Url_triggered()
{
	DialogImportUrl *diu = new DialogImportUrl(this);
	connect(diu, &DialogImportUrl::urlProvided, this, &MainWindow::on_urlProvided);
	diu->show();
}

void MainWindow::on_urlProvided(QString name, QString urlStr)
{
	bool opIsSuccess = false;
	QUrl url(urlStr);
	QDir dir(m_parameters.themesPath.absolutePath() + "/" + name);
	if(url.isValid() && !(name.isEmpty() || name.isNull() || name == ".." || name == "."))
	{
		std::vector<Theme *>::iterator it = findTheme(name);
		if(it == m_extraThemes.end())
		{
			dir.mkpath(dir.absolutePath());
			QFile remoteFile(Theme::remoteFile(dir));
			opIsSuccess = remoteFile.open(QIODevice::WriteOnly);
			if(opIsSuccess)
			{
				remoteFile.write(urlStr.toUtf8());
				remoteFile.close();
				emit openThemeRequired(dir);
			}
		}
		else
		{
			emit messageUpdateRequired("Error: cannot import url: theme with identical name exists.");
		}
	}
	else
	{
		emit messageUpdateRequired("Error: cannot import url: url or name not valid.");
	}
}
