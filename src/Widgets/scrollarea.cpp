#include "scrollarea.h"

#include <iostream>

ScrollArea::ScrollArea(QWidget *parent) :
	QScrollArea(parent)
{
    setAcceptDrops(true);
}

void ScrollArea::resizeEvent(QResizeEvent *event)
{
	assert(event != nullptr);
	QWidget *w = widget();
	ScrollAreaContent *scrollAreaContent = dynamic_cast<ScrollAreaContent *>(w);
	if(scrollAreaContent != nullptr)
	{
		scrollAreaContent->resize(event->size().width()-4, scrollAreaContent->size().height());
	}
}

void ScrollArea::dropEvent(QDropEvent *event)
{
}
