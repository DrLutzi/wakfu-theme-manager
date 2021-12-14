#ifndef COLOR_H
#define COLOR_H

#include <QColor>

class Color
{
public:
	Color(const QString &id, const QString &color);

	const QString &id() const;
	const QColor &color() const;

private:
	QString m_id;
	QColor m_color;
};

class CompareColor
{
public:
	CompareColor();

	bool operator()(const Color &color, const Color &other);
};

#endif // COLOR_H
