#ifndef THEME_H
#define THEME_H

#include "texture.h"
#include <QDir>
#include <map>

class Theme
{
public:
	Theme();

	void save(const QDir &dir) const;
	void load(const QDir &dir);

	void pack();
	void unpack();

	void savePixmaps(const QDir &dir);
	void loadPixmaps(const QDir &dir);


private:
	QString	m_name;
	std::map<QString, Texture, std::less<QString>> m_textures;
};

#endif // THEME_H
