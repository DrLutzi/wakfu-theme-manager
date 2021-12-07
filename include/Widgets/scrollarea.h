#ifndef SCROLLAREA_H
#define SCROLLAREA_H

#include <QScrollArea>
#include <QResizeEvent>
#include "scrollareacontent.h"

class ScrollArea : public QScrollArea
{
	Q_OBJECT
public:
	explicit ScrollArea(QWidget *parent = nullptr);

	void resizeEvent(QResizeEvent *event);
};

#endif // SCROLLAREA_H
