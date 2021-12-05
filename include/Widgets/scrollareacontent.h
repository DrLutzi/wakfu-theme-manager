#ifndef SCROLLAREACONTENT_H
#define SCROLLAREACONTENT_H

#include <QWidget>
#include <QLayout>
#include <QDrag>
#include "themewidget.h"

class ScrollAreaContent : public QWidget
{
	Q_OBJECT
public:
	explicit ScrollAreaContent(QWidget *parent = nullptr);

	void dropEvent(QDropEvent *event);
	void resizeEvent(QResizeEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);

signals:

};

#endif // SCROLLAREACONTENT_H
