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

	const QDir &path() const;
	const QString &name() const;

    bool isInitialized() const;

	bool isOpened() const;
	bool isUnpacked() const;

private:
	QString	m_name;
	QDir m_path;
	std::map<QString, Texture, std::less<QString>> m_textures;
};

#endif // THEME_H
