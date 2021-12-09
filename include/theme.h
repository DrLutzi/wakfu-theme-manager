#ifndef THEME_H
#define THEME_H

#include "texture.h"
#include <QDir>
#include <map>

class Theme
{
public:

	typedef std::map<QString, Texture, std::less<QString>> MapType;
	Theme();

	void save(const QDir &dir) const;
	void load(const QDir &dir);

	void pack(const Theme *model = nullptr);
	void unpack(const Theme *model=nullptr);

	void savePixmaps(const QDir &dir);
	void loadPixmaps(const QDir &dir);

	const QDir &path() const;
	const QString &name() const;

    bool isInitialized() const;

	bool isOpened() const;
	bool isUnpacked() const;

	void copyTextures(const Theme &other);

	const MapType &textures() const;

private:
	QString	m_name;
	QDir m_path;
	MapType m_textures;
};

#endif // THEME_H
