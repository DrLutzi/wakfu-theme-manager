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
#include "unzipper.h"

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

	void createOrUpdateStyle();

	const QString &name() const;
	const Theme *theme() const;
	Theme *theme();

	void setTransparentAspect(bool b);
	void downloadTheme();

signals:

	void downloadInProcess(bool);
	void downloadProgressUpdate(int);

private slots:
	void on_lineEdit_url_editingFinished();
	void on_pushButton_pressed();
	void on_pushButton_forumURL_pressed();

private:

	void moveAndReplaceFolderContents(const QString &fromDir, const QString &toDir);

	Ui::ThemeWidget *ui;
	Theme *m_theme;
	QPixmap m_pixmap;
	QPixmap m_dragPixmap;
};

#endif // THEMEWIDGET_H
