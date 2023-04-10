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
		m_pixmap = m_pixmap.scaled(64, 64, Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation);
		ui->label_pix->setPixmap(m_pixmap);
		m_dragPixmap = m_pixmap.scaled(32, 32, Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation);
	}
	return b;
}

void ThemeWidget::createOrUpdateStyle()
{
	if(m_theme)
	{
		ui->name->setText(m_theme->name());
		ui->author->setText(m_theme->author());

		//TODO: download and set image, or load it if it was already pre-downloaded.

		//Old version, I need to delete this later.

//		QColor qcolor;
//		if(m_theme->colors().size()>0)
//		{
//			const Theme *t = theme();
//			const Theme::ColorMapType &colors = t->colors();
//			Color c("defaultDarkGreyColor", QString(""));
//			Theme::ColorMapType::const_iterator cit = colors.find(c);
//			if(cit == colors.end())
//			{
//				const Color &color = *m_theme->colors().begin();
//				qcolor = color.qcolor();
//			}
//			else
//			{
//				const Color &color = *cit;
//				qcolor = color.qcolor();
//			}
//			QString style = "border-radius : 1px; border-width: 1.3px; border-color: rgb(%1, %2, %3);";
//			ui->label_pix->setStyleSheet(style.arg(std::min(qcolor.red(), 220)).arg(std::min(qcolor.green(), 220)).arg(std::min(qcolor.blue(), 220)));
//		}
//		else
//		{
//			QString style = "border-radius : 1px; border-width: 1.3px; border-color: rgb(%1, %2, %3);";
//			ui->label_pix->setStyleSheet(style.arg(0).arg(0).arg(0));
//		}
//        if(!setImage(QFile(m_theme->path().absolutePath() + "/theme.png")))
//		{
//			if(!setImage(QFile(m_theme->path().absolutePath() + "/images/dungeon.png")))
//			{
//				QPixmap pixmapFillColor(1, 1);
//				qcolor.setRedF(qcolor.redF() + (1.0-qcolor.redF())/2.0);
//				qcolor.setGreenF(qcolor.greenF() + (1.0-qcolor.greenF())/2.0);
//				qcolor.setBlueF(qcolor.blueF() + (1.0-qcolor.blueF())/2.0);
//				pixmapFillColor.fill(qcolor);
//				m_pixmap = pixmapFillColor;
//				m_dragPixmap = m_pixmap.scaled(32, 32, Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::FastTransformation);
//				ui->label_pix->setPixmap(m_pixmap);
//			}
//		}
	}
}

void ThemeWidget::mousePressEvent(QMouseEvent *event)
{
	assert(m_theme);
	if (event->button() == Qt::LeftButton)
	{
		QDrag *drag = new QDrag(this);

        QMimeData *mimeData = new QMimeData;
		mimeData->setText(m_theme->name());
		drag->setMimeData(mimeData);
		drag->setPixmap(m_dragPixmap);
		setTransparentAspect(true);
        Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
        if(dropAction == Qt::IgnoreAction)
        {
            drag->deleteLater();
            mimeData->deleteLater();
        }
		setTransparentAspect(false);
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
//					m_theme->saveRemote();
					moveAndReplaceFolderContents(m_theme->path().absolutePath(), appParameters.outputPath.absolutePath());
					createOrUpdateStyle();
				}
			}
			fd->deleteLater();
		}
		emit downloadInProcess(false);
	});
	fd->launchDownload();
}

void ThemeWidget::on_lineEdit_url_editingFinished()
{
}

void ThemeWidget::on_pushButton_pressed()
{
	downloadTheme();
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

void ThemeWidget::moveAndReplaceFolderContents(const QString &fromDir, const QString &toDir)
{
	QDirIterator it(fromDir, QDirIterator::Subdirectories);
	QDir dir(fromDir);
	const int absSourcePathLength = dir.absoluteFilePath(fromDir).length();
	while (it.hasNext())
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
				QFile::copy(fileInfo.absoluteFilePath(), constructedAbsolutePath);
				QFile::remove(fileInfo.absoluteFilePath());
			}
		}
	}
	dir.removeRecursively();
}
