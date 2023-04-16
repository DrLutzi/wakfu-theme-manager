#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

QJsonDocument jsonThemes;
int nbThreads;
std::mutex nbThreads_mutex;
QJsonDocument jsonExternalThemes;
QUrl jsonExternalThemesLink;
AppParameters appParameters;

MainWindow::MainWindow(QWidget *parent)
	:QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_fd(nullptr),
	m_fdWTM(nullptr),
	m_pixmapRelativePath("/pixmaps/"),
	m_configFile("./config.json"),
	m_extraThemes(),
	m_defaultThemeWidget(nullptr),
	m_extraThemeWidgets(),
	m_progressBar(nullptr)
{
	ui->setupUi(this);
	jsonExternalThemesLink = QUrl("https://drive.google.com/uc?export=download&id=1_36tA1LPK2Y3vCdlUKrI5r09CE1XDuao");
	connect(this, &MainWindow::messageUpdateRequired, ui->statusbar, &QStatusBar::showMessage);
	connect(this, &MainWindow::extraThemeWidgetCreationOrUpdateRequired, this, &MainWindow::createOneThemeWidget);
	connect(this, &MainWindow::openThemeRequired, this, &MainWindow::on_openThemeRequired);
	connect(this, &MainWindow::defaultThemeWidgetCreationRequired, this, &MainWindow::createDefaultThemeWidget);
	connect(this, &MainWindow::updateFromThemesDirRequired, this, &MainWindow::updateFromThemesDir);
	makeProgressBar();
	createScrollAreas();
	loadConfigurationFile();
	initWTMJson();
	checkOutputExistence();
	//initAnkamaJson();
}

MainWindow::~MainWindow()
{
	delete ui;
	delete m_fd;
	delete m_fdWTM;
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
		appParameters.jsonUrl.setUrl(configFileJson["json_url"].toString());
		appParameters.themesPath.setPath(configFileJson["themes_path"].toString());
		appParameters.themesPath.mkdir(".");
		QString output_path(configFileJson["output_path"].toString());
		if(output_path.isNull() || output_path.isEmpty())
		{
			output_path = _defaultThemeDirectory;
			appParameters.outputPath.setPath(output_path);
			saveConfigurationFile();
		}
		else
		{
			appParameters.outputPath.setPath(output_path);
		}
		m_defaultThemePath.setPath(appParameters.themesPath.absolutePath() + "/ankama");
	}
	return b;
}

void MainWindow::checkOutputExistence()
{
	if(!appParameters.outputPath.exists())
	{
		QMessageBox::information(this, tr("Information"), tr("The program could not find the \"theme\" folder of Wakfu at its usual place. Please provide it to continue."));
		QString dirStr = QFileDialog::getExistingDirectory(this, tr("Choose the \"theme\" or \"zaap\" folder of your game"), QDir::homePath());
		QDir dir(dirStr);
		if(dirStr.isNull() || dirStr.isEmpty())
		{
			return;
		}
		if(dir.dirName() == "zaap")
		{
			dir.setPath(dir.absolutePath() + "/wakfu/theme");
		}
		else if(dir.dirName() != "theme")
		{
			emit messageUpdateRequired(tr("Warning: the provided folder is not \"theme\" or \"zaap\"."));
		}
		appParameters.outputPath.setPath(dir.absolutePath());
		saveConfigurationFile();
	}
	else
	{
		emit messageUpdateRequired(tr("Found Wakfu theme folder: ") + appParameters.outputPath.absolutePath());
	}
}

void MainWindow::importAllThemes()
{
	setAllWidgetsEnabled(false);
	QFileInfoList ls = appParameters.themesPath.entryInfoList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot);
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
	QLayout *layout = ui->widget_scrollArea_used->layout();

	layout->removeWidget(ui->scrollArea_used);
	delete(ui->scrollArea_used);

	ui->scrollArea_used = new ScrollArea(centralWidget);

	ui->scrollArea_used->setObjectName(QString::fromUtf8("scrollArea_used"));
	ui->scrollArea_used->setMinimumSize(QSize(128, 0));
	ui->scrollArea_used->setAcceptDrops(true);
	ui->scrollArea_used->setWidgetResizable(true);
	ui->scrollArea_used->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	layout->addWidget(ui->scrollArea_used);

	//custom scroll area content widget
	ScrollAreaContent *content = new ScrollAreaContent(ui->scrollArea_used);
	ui->scrollAreaWidgetContents_used = content;

	//Layout in scroll areas
	ui->scrollAreaWidgetContents_used->setLayout(new QBoxLayout(QBoxLayout::TopToBottom, ui->scrollAreaWidgetContents_used));
	ui->scrollAreaWidgetContents_used->layout()->setAlignment(Qt::AlignTop);

	//set scroll area custom widget
	ui->scrollArea_used->setWidget(ui->scrollAreaWidgetContents_used);

	ui->scrollArea_used->horizontalScrollBar()->setEnabled(false);

	ui->scrollAreaWidgetContents_used->layout()->setContentsMargins(4, 4, 4, 4);

	return;
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
				{"json_url", appParameters.jsonUrl.url()},
				{"themes_path", appParameters.themesPath.absolutePath()},
				{"output_path", appParameters.outputPath.absolutePath()}
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

void MainWindow::openThemeFolderOrFile(const QString &str)
{
	bool opIsSuccess=false;

	if(str.isEmpty() || str.isNull())
		return;
	QFileInfo fileInfo(str);
	if(fileInfo.isDir())
	{
		QDir dir(str);
		opIsSuccess = openThemeFolder(dir);
	}
	else if(fileInfo.isFile() && fileInfo.suffix() == "zip")
	{
		emit progressUpdateRequired(0);
		emit messageUpdateRequired(tr("Extracting theme..."));
		QFile file(str);
		opIsSuccess = openThemeZip(file);
	}
	else
	{
		emit messageUpdateRequired(QString(tr("Error while attempting to open theme(s): no .zip file or folder selected.")));
	}
	(void) opIsSuccess;
	emit progressUpdateRequired(100);
	return;
}

bool MainWindow::openThemeFolder(QDir &dir)
{
	bool opIsSuccess = true;
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

	if(dir.exists())
	{
		if(correctIfDirIsImagesOrColors(dir) && !themeFolderIsValid(dir))
		{
			emit progressUpdateRequired(0);
			const QStringList lsDir = dir.entryList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot);
			for(const QString &strEntry : lsDir)
			{
				QDir dirEntry(dir.absolutePath() + "/" + strEntry);
				if(themeFolderIsValid(dirEntry))
					opIsSuccess &= ThemeWidget::moveAndReplaceFolderContents(dirEntry.absolutePath(), appParameters.outputPath.absolutePath(), false);
			}
		}
		else if (themeFolderIsValid(dir))
		{
			opIsSuccess &= ThemeWidget::moveAndReplaceFolderContents(dir.absolutePath(), appParameters.outputPath.absolutePath(), false);
		}
	}
	if(!opIsSuccess)
	{
		emit messageUpdateRequired(QString(tr("Error while attempting to open theme(s): no valid theme folder found.")));
	}
	return opIsSuccess;
}

bool MainWindow::openThemeZip(QFile &file)
{
	bool opIsSuccess = file.open(QIODevice::ReadOnly);
	QFileInfo fileInfo(file);
	if(opIsSuccess)
	{
		file.close();
		if(opIsSuccess)
		{
			QDir themeDir = appParameters.themesPath.absolutePath() + "/" + fileInfo.baseName();
			emit progressUpdateRequired(33);
			//the following function works only if colors or images exists
			Theme theme;
			theme.setPath(themeDir);
			opIsSuccess = theme.unzip(file);
			if(opIsSuccess)
			{
				emit messageUpdateRequired(tr("Exporting theme..."));
				emit progressUpdateRequired(66);
				QDir themePath(theme.path());
				opIsSuccess &= openThemeFolder(themePath);
				themePath.removeRecursively();
				if(opIsSuccess)
				{
					emit messageUpdateRequired(tr("Theme extracted and exported with success."));
					emit progressUpdateRequired(100);
				}
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
	return opIsSuccess;
}

AppParameters *MainWindow::parameters()
{
	return &appParameters;
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

void MainWindow::initWTMJson()
{
//	setAllWidgetsEnabled(false);

	m_fdWTM = new FileDownloader(jsonExternalThemesLink, this);
	connect(m_fdWTM, &FileDownloader::downloaded, this, &MainWindow::loadWTMJsonFromWeb);
	connect(m_fdWTM, &FileDownloader::errorMsg, ui->statusbar, &QStatusBar::showMessage);
	m_fdWTM->launchDownload();
}

void MainWindow::initAnkamaJson(bool forceReset)
{
	if(jsonThemes.isEmpty() || forceReset)
	{
		QFile jsonThemeFile(appParameters.themesPath.absolutePath() + "/theme.json");
		bool b = jsonThemeFile.open(QIODevice::ReadOnly) && m_defaultThemePath.exists()
				&& Theme::colorsDir(m_defaultThemePath).exists()
				&& Theme::imagesDir(m_defaultThemePath).exists();
		if(b)
		{
			jsonThemes = QJsonDocument::fromJson(jsonThemeFile.readAll());
			const QJsonArray &textureArray = jsonThemes["textures"].toArray();
			Texture::initPathToIdMapFromJson(textureArray);
			jsonThemeFile.close();
		}
		else
		{
			on_actionDownload_triggered();
		}
	}
}

void MainWindow::downloadWTMThemes()
{
	emit progressUpdateRequired(0);
	QJsonParseError *jsonPE = new QJsonParseError;
	jsonExternalThemes = QJsonDocument::fromJson(m_fdWTM->downloadedData(), jsonPE);

	//construct the theme widgets
	QJsonArray jsonThemes = jsonExternalThemes.array();
	unsigned int i = 0;
	for(QJsonArray::ConstIterator cit = jsonThemes.constBegin(); cit!=jsonThemes.constEnd(); ++cit)
	{
		const QJsonValue &value = (*cit);
		//take all json values
		QJsonValue jsonName = value["name"];
		QJsonValue jsonAuthor = value["author"];
		QJsonValue jsonVersion = value["version"];
		QJsonValue jsonLink = value["link"];
		QJsonValue jsonImage = value["image"];
		QJsonValue jsonForum = value["forum"];

		QString name = jsonName.toString(tr("Undefined name") + QString::number(i++));
		QString author = jsonAuthor.toString(tr("Unknown author"));
		int version = jsonVersion.toInt(0);
		QUrl link = QUrl(jsonLink.toString(""));
		QUrl image = QUrl(jsonImage.toString(""));
		QUrl forum = QUrl(jsonForum.toString(""));

		Theme *theme = new Theme();
		theme->setName(name);
		theme->setAuthor(author);
		theme->setVersion(version);
		theme->setRemote(link);
		theme->setImageRemote(image);
		theme->setForumURL(forum);

		theme->setPath(appParameters.themesPath.absolutePath() + "/" + name);

		m_extraThemes.push_back(theme);
		createOneThemeWidget(theme);
	}
	setAllWidgetsEnabled(true);
}

void MainWindow::downloadDefault()
{
	emit progressUpdateRequired(0);
	QJsonParseError *jsonPE = new QJsonParseError;
	jsonThemes = QJsonDocument::fromJson(m_fd->downloadedData(), jsonPE);
	//create useful folders
	QDir defaultThemeImagePath(Theme::imagesDir(m_defaultThemePath));
	defaultThemeImagePath.mkpath(defaultThemeImagePath.absolutePath());

	//save the json file
	QFile jsonFile(appParameters.themesPath.absolutePath() + "/theme.json", this);
	jsonFile.open(QIODevice::WriteOnly);	//todo erreurs
	jsonFile.write(jsonThemes.toJson());
	jsonFile.close();

	const QJsonValue &texturesValue = jsonThemes["textures"]; //todo erreurs
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
			QString originalPath = fileInfo.filePath();

			// Replace "theme" with an empty string to remove it from the path
			QString modifiedPath = originalPath.replace("theme/", "");

			// Replace ".tga" with ".png" to change the file extension
			modifiedPath.replace(".tga", ".png");

			FileDownloader *fd = new FileDownloader("https://wakfu.cdn.ankama.com/gamedata/theme/" + modifiedPath);
			connect(fd, &FileDownloader::errorMsg, ui->statusbar, &QStatusBar::showMessage);
			connect(fd, &FileDownloader::downloaded, this, [&, fd, maxProgress, modifiedPath](QUrl url2)
			{
				(void) url2;
				QFile file(m_defaultThemePath.absolutePath() + "/" + modifiedPath, this);
				QFileInfo fileInfo(file.fileName());
				QDir parentDir = fileInfo.dir();
				if(!parentDir.exists())
				{
					parentDir.mkpath(parentDir.absolutePath());
				}
				QPixmap textureImage;
				textureImage.loadFromData(fd->downloadedData());
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
						//Create colors
						m_defaultTheme.extractColorsFromJsonThemes();
						m_defaultTheme.saveColors(defaultTheme);
						//Load theme
						m_defaultTheme.load(defaultTheme);
					}
					const QJsonArray &textureArray = jsonThemes["textures"].toArray();
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
	m_outputTheme.save(appParameters.outputPath);
	m_outputTheme.unload();
	emit messageUpdateRequired(QString(tr("Theme compiled.")));
}

void MainWindow::resetTheme()
{
	emit messageUpdateRequired(tr("Removing theme in ") + appParameters.outputPath.absolutePath() + " ...");
	emit progressUpdateRequired(0);
	QDir images(Theme::imagesDir(appParameters.outputPath));
	QDir colors(Theme::colorsDir(appParameters.outputPath));
	if(images.exists())
	{
		images.removeRecursively();
	}
	emit progressUpdateRequired(50);
	if(colors.exists())
	{
		colors.removeRecursively();
	}
	emit messageUpdateRequired(tr("Successfully reset theme in ") + appParameters.outputPath.absolutePath());
	emit progressUpdateRequired(100);
}

std::vector<Theme *>::iterator MainWindow::findTheme(const QString &name)
{
	std::vector<Theme *>::iterator it = m_extraThemes.end();
	if(!name.isEmpty() && !name.isNull() && name != "ankama")
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
	return ((imagesDir.exists() && !imagesDir.isEmpty()) || (colorsDir.exists() && !colorsDir.isEmpty()));
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
	ui->widget_scrollArea_used->setEnabled(b);
}

void MainWindow::updateFromThemesDir()
{
	QStringList ls = appParameters.themesPath.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
	int i=0;
	for(QStringList::ConstIterator cit = ls.constBegin(); cit != ls.constEnd(); ++cit, ++i)
	{
		const QString &dirStr = (*cit);
		if(!dirStr.isEmpty() && !dirStr.isNull())
		{
			QDir dir(dirStr);
			QString dirName(dir.dirName());
			if(dirName == "ankama")
			{
				m_defaultTheme.load(m_defaultThemePath);
			}
			else
			{
				on_openThemeRequired(appParameters.themesPath.absolutePath() + "/" + dirName);
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
			createOneThemeWidget(theme);
		}
	}
}

void MainWindow::createDefaultThemeWidget()
{
	m_defaultThemeWidget = new ThemeWidget(&m_defaultTheme, ui->scrollAreaWidgetContents_used);
	m_defaultThemeWidget->setTransparentAspect(true);
	m_defaultThemeWidget->setEnabled(false);
	m_defaultThemeWidget->setToolTip(tr("Default theme downloaded and imported correctly."));
}

ThemeWidget *MainWindow::createOneThemeWidget(Theme *theme)
{
	ThemeWidget *extraTheme = nullptr;
	if(theme == nullptr)
	{
		return extraTheme;
	}
	extraTheme = new ThemeWidget(theme, ui->scrollAreaWidgetContents_used);
	connect(extraTheme, &ThemeWidget::downloadInProcess, this, &MainWindow::on_downloadInProcess);
	connect(extraTheme, &ThemeWidget::progressUpdateRequired, m_progressBar, &QProgressBar::setValue);
	connect(extraTheme, &ThemeWidget::messageUpdateRequired, ui->statusbar, &QStatusBar::showMessage);
	m_extraThemeWidgets.push_back(extraTheme);
	ui->scrollAreaWidgetContents_used->layout()->addWidget(extraTheme);
	return extraTheme;
}

void MainWindow::createAllThemeWidgets()
{
	delete m_defaultThemeWidget;
	m_defaultThemeWidget = nullptr;
	clearLayout(ui->scrollAreaWidgetContents_used->layout());
	for(std::vector<ThemeWidget *>::iterator it = m_extraThemeWidgets.begin(); it != m_extraThemeWidgets.end(); ++it)
	{
		delete (*it);
	}
	m_extraThemeWidgets.reserve(m_extraThemes.size());
	for(std::vector<Theme *>::iterator it = m_extraThemes.begin(); it != m_extraThemes.end(); ++it) //TODO : should be const iterator
	{
		Theme *&theme = *it;
		emit extraThemeWidgetCreationOrUpdateRequired(theme);
	}
	return;
}

void MainWindow::loadWTMJsonFromWeb()
{
	ui->actionDownload->setEnabled(false);
	ui->actionMake_theme->setEnabled(false);
	ui->actionReset->setEnabled(false);
	if(m_defaultThemeWidget != nullptr)
	{
		m_defaultThemeWidget->setEnabled(false);
	}
	downloadWTMThemes();
}

void MainWindow::loadAnkamaJsonFromWeb()
{
	ui->actionDownload->setEnabled(false);
	ui->actionMake_theme->setEnabled(false);
	if(m_defaultThemeWidget != nullptr)
	{
		m_defaultThemeWidget->setEnabled(false);
	}
	downloadDefault();
}

void MainWindow::on_actionDownload_triggered()
{
	setAllWidgetsEnabled(false);

	QUrl jsonUrl(appParameters.jsonUrl);
	m_fd = new FileDownloader(jsonUrl, this);
	connect(m_fd, &FileDownloader::downloaded, this, &MainWindow::loadAnkamaJsonFromWeb); //TODO : connect to deleteLater or make a setUrl function
	connect(m_fd, &FileDownloader::errorMsg, ui->statusbar, &QStatusBar::showMessage);
	m_fd->launchDownload();
}


void MainWindow::on_actionOpen_triggered()
{
	setAllWidgetsEnabled(false);
	QString str = QFileDialog::getExistingDirectory(this, tr("Open a theme folder/a folder containing theme folders"), QDir::homePath());
	openThemeFolderOrFile(str);
	setAllWidgetsEnabled(true);
}

void MainWindow::on_actionOpen_Zip_triggered()
{
	setAllWidgetsEnabled(false);
	QString str = QFileDialog::getOpenFileName(this, tr("Open a .zip archive containing a theme"), QDir::homePath(), "*.zip");
	openThemeFolderOrFile(str);
	setAllWidgetsEnabled(true);
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
		initAnkamaJson();
		if(jsonThemes.isEmpty())
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
	QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Warning"),
		tr("This action will remove theme folders and files within ") + appParameters.outputPath.absolutePath() + tr(". Do you want to continue?"));
	if(reply == QMessageBox::Yes)
		resetTheme();
	setAllWidgetsEnabled(true);
}

void MainWindow::on_actionParameters_triggered()
{
	FormParameters *formParameters = new FormParameters(appParameters, this);
	connect(formParameters, &FormParameters::notifyParametersChanged, this, &MainWindow::on_parametersChanged);
	formParameters->show();
}

void MainWindow::on_parametersChanged(AppParameters newParameters)
{
	setAllWidgetsEnabled(false);
	bool themesPathChanged = newParameters.themesPath != appParameters.themesPath;
	appParameters = newParameters;
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
			m_defaultThemePath.setPath(appParameters.themesPath.absolutePath() + "/ankama");
			on_actionDownload_triggered();
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
	QDir dir(appParameters.themesPath.absolutePath() + "/" + name);
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

void MainWindow::on_downloadInProcess(bool b)
{
	setAllWidgetsEnabled(!b);
}
