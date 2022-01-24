#ifndef COLOR_H
#define COLOR_H

#include <QColor>

class Color
{
public:
	Color(const QString &id, const QString &colorStr);
	Color(const QString &id, const QColor &color);

	const QString &id() const;
	const QColor &qcolor() const;
	QString colorName() const;

	static QString writeRGBA(const QColor &color);
	static QColor readRGBA(const QString &colorStr);

private:

	QString m_id;
	QColor m_color;
};

class CompareColor
{
public:
	CompareColor();

	bool operator()(const Color &color, const Color &other) const;
};

#endif // COLOR_H
