#include "themewidget.h"
#include "ui_themewidget.h"

ThemeWidget::ThemeWidget(Theme *theme, QWidget *parent) :
	QFrame(parent),
	ui(new Ui::ThemeWidget),
	m_theme(theme)
{
	ui->setupUi(this);
	ui->label_pix->setScaledContents(true);
	createPixmap();
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

void ThemeWidget::createPixmap()
{
	if(m_theme)
	{
		ui->label->setText(m_theme->name());
		QColor qcolor;
		if(m_theme->colors().size()>0)
		{
			const Theme *t = theme();
			const Theme::ColorMapType &colors = t->colors();
			Color c("defaultDarkGreyColor", "");
			Theme::ColorMapType::const_iterator cit = colors.find(c);
			if(cit == colors.end())
			{
				const Color &color = *m_theme->colors().begin();
				qcolor = color.qcolor();
			}
			else
			{
				const Color &color = *cit;
				qcolor = color.qcolor();
			}
			QString style = "border-radius : 1px; border-width: 1.3px; border-color: rgb(%1, %2, %3);";
			ui->label_pix->setStyleSheet(style.arg(std::min(qcolor.red(), 220)).arg(std::min(qcolor.green(), 220)).arg(std::min(qcolor.blue(), 220)));
		}
		else
		{
			QString style = "border-radius : 1px; border-width: 1.3px; border-color: rgb(%1, %2, %3);";
			ui->label_pix->setStyleSheet(style.arg(0).arg(0).arg(0));
		}
        if(!setImage(QFile(m_theme->path().absolutePath() + "/theme.png")))
		{
			if(!setImage(QFile(m_theme->path().absolutePath() + "/images/dungeon.png")))
			{
				QPixmap pixmapFillColor(1, 1);
				qcolor.setRedF(qcolor.redF() + (1.0-qcolor.redF())/2.0);
				qcolor.setGreenF(qcolor.greenF() + (1.0-qcolor.greenF())/2.0);
				qcolor.setBlueF(qcolor.blueF() + (1.0-qcolor.blueF())/2.0);
				pixmapFillColor.fill(qcolor);
				m_pixmap = pixmapFillColor;
				m_dragPixmap = m_pixmap.scaled(32, 32, Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::FastTransformation);
				ui->label_pix->setPixmap(m_pixmap);
			}
		}
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
