#ifndef BROWSERCONTENT_H
#define BROWSERCONTENT_H

#include <QWidget>

namespace Ui {
class BrowserContent;
}

class BrowserContent : public QWidget
{
	Q_OBJECT

public:
	explicit BrowserContent(QWidget *parent = nullptr);
	~BrowserContent();

private:
	Ui::BrowserContent *ui;
};

#endif // BROWSERCONTENT_H
