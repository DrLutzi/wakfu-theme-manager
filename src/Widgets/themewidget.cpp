#include "themewidget.h"
#include "ui_themewidget.h"

ThemeWidget::ThemeWidget(Theme *theme, QWidget *parent) :
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


ThemeWidget::~ThemeWidget()
{
	delete ui;
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
		setTransparentAspect(true);
		drag->exec(Qt::MoveAction);
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
