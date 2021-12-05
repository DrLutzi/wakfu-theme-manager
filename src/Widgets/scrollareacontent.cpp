#include "scrollareacontent.h"

ScrollAreaContent::ScrollAreaContent(QWidget *parent) : QWidget(parent)
{
    setAcceptDrops(true);
}

void ScrollAreaContent::dropEvent(QDropEvent *event)
{
	assert(m_twin && event != nullptr);
	QBoxLayout *l = dynamic_cast<QBoxLayout *>(layout());
	assert(l != nullptr);
	event->acceptProposedAction();
	const QMimeData *mimeData = event->mimeData();
	const QPoint &pos = event->pos();
	if(mimeData->hasText())
	{
		QString text = mimeData->text(); //text should be a name
		ThemeWidget* tw;
		bool isInSelf = false;
		if((tw = m_twin->find(text)))
		{
			m_twin->remove(tw);
		}
		else if((tw = find(text)))
		{
			isInSelf = true;
		}
		if(tw != nullptr)
		{
			ThemeWidget *twUnder = nullptr, *twAbove = nullptr;
			getNeighbors(pos, twUnder, twAbove);
			if(isInSelf)
				remove(tw);
			tw->setParent(this);
			if(twAbove == nullptr)
			{
				l->addWidget(tw);
			}
			else if(twUnder == nullptr)
			{
				l->insertWidget(0, tw);
			}
			else
			{
				int layoutIndex = layout()->indexOf(twUnder);
				assert(layoutIndex != -1);
				l->insertWidget(layoutIndex, tw);
			}
		}
	}
}

void ScrollAreaContent::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("text/plain"))
	{

		event->acceptProposedAction();
	}
}

void ScrollAreaContent::resizeEvent(QResizeEvent *event)
{
	const QLayout *l = layout();
	if(l != nullptr && event != nullptr)
	{
		QList<ThemeWidget *> listThemeWidget = this->findChildren<ThemeWidget *>();
		foreach(ThemeWidget *tw, listThemeWidget)
		{
            tw->resize(event->size().width()-4, tw->height());
		}
	}
}

void ScrollAreaContent::setTwin(ScrollAreaContent *twin)
{
	m_twin = twin;
}

ThemeWidget* ScrollAreaContent::find(const QString &name)
{
	QList<ThemeWidget *> listThemeWidget = this->findChildren<ThemeWidget *>();
	ThemeWidget *retTW = nullptr;
	for(QList<ThemeWidget *>::iterator it = listThemeWidget.begin(); it != listThemeWidget.end() && retTW==nullptr; ++it)
	{
		ThemeWidget *tw = *it;
		if(tw != nullptr && tw->name() == name)
		{
			retTW = tw;
		}
	}
	return retTW;
}

bool ScrollAreaContent::remove(ThemeWidget *tw)
{
	bool b = false;
	if(tw != nullptr)
	{
		QList<ThemeWidget *> listThemeWidget = this->findChildren<ThemeWidget *>();
		b = listThemeWidget.removeOne(tw);
	}
	return b;
}

void ScrollAreaContent::getNeighbors(const QPoint &pos, ThemeWidget *&twUnder, ThemeWidget *&twAbove) const
{
	twUnder = nullptr;
	twAbove = nullptr;
	int yHighMin=std::numeric_limits<int>::max(), yLowMax=0;
	QList<ThemeWidget *> listThemeWidget = this->findChildren<ThemeWidget *>();
	for(QList<ThemeWidget *>::iterator it = listThemeWidget.begin(); it != listThemeWidget.end(); ++it)
	{
		ThemeWidget *tw = *it;
		if(tw != nullptr)
		{
			const QRect &rectTw = tw->geometry();
			QPoint bottomLeftTw = rectTw.bottomLeft();
			QPoint topLeftTw = rectTw.topLeft();
			QPoint centerTwMapped = tw->mapToParent((bottomLeftTw - topLeftTw)/2.0);
			int yCenter = centerTwMapped.y();
			if(yCenter < pos.y() && yCenter>yLowMax)
			{
				twUnder = tw;
				yLowMax = yCenter;
			}
			else if(yCenter<yHighMin)
			{
				twAbove = tw;
				yHighMin = yCenter;
			}
		}
	}
}
