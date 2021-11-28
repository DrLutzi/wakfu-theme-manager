#ifndef THEME_H
#define THEME_H

#include "texture.h"
#include <QDir>
#include <map>

class Theme
{
public:
	Theme();

	void save(QDir dir) const;
	void load(QDir dir);

private:
	QString	m_name;
	std::map<QString, Texture, std::less<QString>> m_textures;
};

#endif // THEME_H
