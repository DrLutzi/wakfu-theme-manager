#include "scrollareacontent.h"

ScrollAreaContent::ScrollAreaContent(QWidget *parent) : QWidget(parent)
{}

void ScrollAreaContent::dropEvent(QDropEvent *event)
{

}

void ScrollAreaContent::resizeEvent(QResizeEvent *event)
{
	const QLayout *l = layout();
	if(l != nullptr && event != nullptr)
	{
		QList<ThemeWidget *> listThemeWidget = this->findChildren<ThemeWidget *>();
		foreach(ThemeWidget *tw, listThemeWidget)
		{
			tw->resize(event->size().width()-16, tw->height());
		}
	}
}
