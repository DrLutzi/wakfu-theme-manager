#include "color.h"

Color::Color(const QString &id, const QString &color) :
	m_id(id),
	m_color(color)
{}

const QString &Color::id() const
{
	return m_id;
}

const QColor &Color::color() const
{
	return m_color;
}


CompareColor::CompareColor()
{}

bool CompareColor::operator()(const Color &color, const Color &other)
{
	std::less<QString> comparator;
	return comparator(color.id(), other.id());
}
