#include "color.h"

Color::Color(const QString &id, const QString &color) :
	m_id(id),
	m_color(readRGBA(color))
{}

const QString &Color::id() const
{
	return m_id;
}

const QColor &Color::qcolor() const
{
	return m_color;
}

QString Color::colorName() const
{
	return writeRGBA(m_color);
}

QString Color::writeRGBA(const QColor &color)
{
	//it is assumed the string is in the form RRGGBBAA, which is an issue because DUMBASS Qt only writes AARRGGBB,
	//So we need to swap colors out
	//RGBA -> ARGB
	int a = color.red();
	int r = color.green();
	int g = color.blue();
	int b = color.alpha();
	QColor colorCorrected(r, g, b, a);
	QString colorStr = colorCorrected.name(QColor::NameFormat::HexArgb);
	return colorStr;
}

QColor Color::readRGBA(const QString &colorStr)
{
	if(colorStr.size()>=9)
	{
		//same issue as writeRGBA.
		//ARGB -> RGBA
		QColor colorCorrected(colorStr);
		int r = colorCorrected.alpha();
		int g = colorCorrected.red();
		int b = colorCorrected.green();
		int a = colorCorrected.blue();
		colorCorrected.setRgb(r, g, b, a);
		return colorCorrected;
	}
	else
	{
		//otherwise it's easier, we read RGB
		QColor color(colorStr);
		return color;
	}
}

CompareColor::CompareColor()
{}

bool CompareColor::operator()(const Color &color, const Color &other) const
{
	std::less<QString> comparator;
	return comparator(color.id(), other.id());
}
