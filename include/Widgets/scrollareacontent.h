#ifndef SCROLLAREACONTENT_H
#define SCROLLAREACONTENT_H

#include <QWidget>
#include <QLayout>
#include "themewidget.h"

class ScrollAreaContent : public QWidget
{
	Q_OBJECT
public:
	explicit ScrollAreaContent(QWidget *parent = nullptr);

	void dropEvent(QDropEvent *event);
	void resizeEvent(QResizeEvent *event);

signals:

};

#endif // SCROLLAREACONTENT_H
