#ifndef THEMEWIDGET_H
#define THEMEWIDGET_H

#include <QFrame>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QCursor>
#include <QDropEvent>
#include <QCommonStyle>
#include "theme.h"

namespace Ui {
class ThemeWidget;
}

class ThemeWidget : public QFrame
{
	Q_OBJECT

public:
	explicit ThemeWidget(Theme *theme, QWidget *parent = nullptr);
	~ThemeWidget();

	void mousePressEvent(QMouseEvent *event);
	bool setImage(const QFile &file);
	bool setImage(const QDir &dir);

	void createPixmap();

	const QString &name() const;
	const Theme *theme() const;
	Theme *theme();

	void setTransparentAspect(bool b);

private:
	Ui::ThemeWidget *ui;
	Theme *m_theme;
	QPixmap m_pixmap;
	QPixmap m_dragPixmap;
};

#endif // THEMEWIDGET_H
