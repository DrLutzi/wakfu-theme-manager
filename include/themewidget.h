#ifndef THEMEWIDGET_H
#define THEMEWIDGET_H

#include <QFrame>

namespace Ui {
class ThemeWidget;
}

class ThemeWidget : public QFrame
{
	Q_OBJECT

public:
	explicit ThemeWidget(QWidget *parent = nullptr);
	~ThemeWidget();

private:
	Ui::ThemeWidget *ui;
};

#endif // THEMEWIDGET_H
