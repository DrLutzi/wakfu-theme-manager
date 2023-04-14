#include "themewidget.h"
#include "types.h"
#include "ui_themewidget.h"
#include <QDesktopServices>

extern AppParameters appParameters;

ThemeWidget::ThemeWidget(Theme *theme, QWidget *parent) :
	QFrame(parent),
	ui(new Ui::ThemeWidget),
	m_theme(theme)
{
	ui->setupUi(this);
	ui->label_pix->setScaledContents(true);
	connect(this, &ThemeWidget::downloadInProcess, this, &QWidget::setDisabled);
	createOrUpdateStyle();
}


ThemeWidget::~ThemeWidget()
{
	delete ui;
}

bool ThemeWidget::setImage(const QFile &file)
{
	bool b = m_pixmap.load(file.fileName());
	if(b)
	{
		m_pixmap = m_pixmap.scaled(128, 128, Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation);
		ui->label_pix->setPixmap(m_pixmap);
		m_dragPixmap = m_pixmap.scaled(32, 32, Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation);
	}
	return b;
}

void ThemeWidget::createOrUpdateStyle()
{
	if(m_theme)
	{
		//Texts + tooltips sets
		ui->name->setText(m_theme->name());
		ui->author->setText(m_theme->author());
		ui->pushButton_forumURL->setToolTip(tr("Forum:") + " " + m_theme->forumURL().url());
		ui->pushButton->setToolTip(tr("Download:") + " " + m_theme->remote().url());

		//Image download
		if(m_theme->imageRemote().isValid())
		{
			downloadImage();
		}
	}
}

const QString &ThemeWidget::name() const
{
	assert(m_theme != nullptr);
	return m_theme->name();
}

const Theme *ThemeWidget::theme() const
{
	return m_theme;
}

Theme *ThemeWidget::theme()
{
	return m_theme;
}

void ThemeWidget::setTransparentAspect(bool b)
{
	static QString defaultStyleSheet("border: 2px solid black;\nborder-radius: 8px;\nbackground-color: white;");
	static QString transparentStyleSheet("border: 2px solid rgba(0, 0, 0, 30%);\nborder-radius: 8px;\nbackground-color: rgba(255, 255, 255, 30%);");
	setStyleSheet(b ? transparentStyleSheet : defaultStyleSheet);
}

void ThemeWidget::downloadTheme()
{
	if(m_theme == nullptr)
		return;
	QUrl remote = m_theme->remote();
	if(!remote.isValid())
		return;
	emit downloadInProcess(true);
	FileDownloader *fd = new FileDownloader(remote, this);
	fd->connect(fd, &FileDownloader::errorMsg, this, [this] (QString errorMsg, int timeout)
	{
		(void) timeout;
		qDebug() << errorMsg;
		emit downloadInProcess(false);
	});
	fd->connect(fd, &FileDownloader::downloaded, this, [this, fd] ()
	{
		bool opIsSuccess;
		QByteArray downloadedData = fd->downloadedData();
		if((opIsSuccess = !downloadedData.isEmpty() && !downloadedData.isNull()))
		{
			emit messageUpdateRequired(tr("Saving theme..."));
			emit progressUpdateRequired(66);
			QDir root = m_theme->path();
			root.cdUp();
			QFile tmpZipFile(root.absolutePath() + "/_tmpTheme_" + m_theme->name() + ".zip");
			tmpZipFile.open(QIODevice::WriteOnly);
			if(opIsSuccess)
			{
				tmpZipFile.write(downloadedData);
				tmpZipFile.close();
				opIsSuccess = m_theme->unzip(tmpZipFile);
				tmpZipFile.remove();
				if(opIsSuccess)
				{
					emit messageUpdateRequired(tr("Exporting theme..."));
					emit progressUpdateRequired(88);
					moveAndReplaceFolderContents(m_theme->path().absolutePath(), appParameters.outputPath.absolutePath());
					createOrUpdateStyle();
					emit messageUpdateRequired(tr("Theme downloaded and exported with success."));
					emit progressUpdateRequired(100);
				}
			}
			fd->deleteLater();
		}
		emit downloadInProcess(false);
	});
	fd->connect(fd, &FileDownloader::updateProgress, this, &ThemeWidget::on_updateProgress);
	emit messageUpdateRequired(tr("Downloading theme..."));
	emit progressUpdateRequired(0);
	fd->launchDownload();
}

void ThemeWidget::downloadImage()
{
	FileDownloader *fd = new FileDownloader(m_theme->imageRemote(), this);
	fd->connect(fd, &FileDownloader::errorMsg, this, [] (QString errorMsg, int timeout)
	{
		(void) timeout;
		qDebug() << errorMsg;
	});
	fd->connect(fd, &FileDownloader::downloaded, this, [this, fd] ()
	{
		bool opIsSuccess;
		QByteArray downloadedData = fd->downloadedData();
		if((opIsSuccess = !downloadedData.isEmpty() && !downloadedData.isNull()))
		{
			QPixmap image;
			image.loadFromData(downloadedData);
			ui->label_pix->setPixmap(image);
		}
	});
	fd->launchDownload();
}

void ThemeWidget::on_pushButton_pressed()
{
	QTimer *timer = new QTimer;
	ui->pushButton->setEnabled(false);
	timer->setInterval(30000);
	connect(timer, &QTimer::timeout, this, [this]()
	{
		ui->pushButton->setEnabled(true); // Re-enable the button
	});
	connect(timer, &QTimer::timeout, &QObject::deleteLater);
	downloadTheme();
	timer->start();
}


void ThemeWidget::on_pushButton_forumURL_pressed()
{
	if(m_theme != nullptr)
	{
		QUrl forumURL = m_theme->forumURL();
		if(forumURL.isValid() && !forumURL.isLocalFile())
			QDesktopServices::openUrl(forumURL);
	}
}

void ThemeWidget::on_updateProgress(quint64 bytesReceived, quint64 bytesTotal)
{
	double ratio = double(bytesReceived)/bytesTotal;
	emit progressUpdateRequired(ratio*100);
}

bool ThemeWidget::moveAndReplaceFolderContents(const QString &fromDir, const QString &toDir, bool removeOrigin)
{
	QDirIterator it(fromDir, QDirIterator::Subdirectories);
	QDir dir(fromDir);
	const int absSourcePathLength = dir.absoluteFilePath(fromDir).length();
	bool b = true;
	while (it.hasNext() && b)
	{
		it.next();
		const auto fileInfo = it.fileInfo();
		if(!fileInfo.isHidden())
		{ //filters dot and dotdot
			const QString subPathStructure = fileInfo.absoluteFilePath().mid(absSourcePathLength);
			const QString constructedAbsolutePath = toDir + subPathStructure;

			if(fileInfo.isDir())
			{
				//Create directory in target folder
				dir.mkpath(constructedAbsolutePath);
			}
			else if(fileInfo.isFile())
			{
				//Copy File to target directory
				//Remove file at target location, if it exists, or QFile::copy will fail
				QFile::remove(constructedAbsolutePath);
				b = QFile::copy(fileInfo.absoluteFilePath(), constructedAbsolutePath);
				if(removeOrigin && b)
					QFile::remove(fileInfo.absoluteFilePath());
			}
		}
	}
	if(removeOrigin && b)
		dir.removeRecursively();
	return b;
}
