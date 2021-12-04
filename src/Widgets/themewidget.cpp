#include "themewidget.h"
#include "ui_themewidget.h"

ThemeWidget::ThemeWidget(const Theme *theme, QWidget *parent) :
	QFrame(parent),
	ui(new Ui::ThemeWidget),
	m_theme(theme)
{
	ui->setupUi(this);
	if(m_theme)
	{
		ui->label->setText(theme->name());
		setImage(QDir(m_theme->path().absolutePath()));
	}
}

bool ThemeWidget::setImage(const QFile &file)
{
	bool b = m_pixmap.load(file.fileName());
	ui->label_pix->setScaledContents(true);
	ui->label_pix->setPixmap(m_pixmap);
	m_dragPixmap = m_pixmap.scaled(64, 64, Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation);
	return b;
}

bool ThemeWidget::setImage(const QDir &dir)
{
	QFile file(dir.absolutePath() + "/theme.png");
	return setImage(file);
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
		Qt::DropAction dropAction = drag->exec();
	}
}

ThemeWidget::~ThemeWidget()
{
	delete ui;
}
