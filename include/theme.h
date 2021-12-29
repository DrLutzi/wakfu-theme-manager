#ifndef THEME_H
#define THEME_H

#include "texture.h"
#include <QDir>
#include <map>
#include <set>
#include "color.h"
#include <QtXml>
#include "filedownloader.h"
#include "unzipper.h"

class Theme
{
public:

	typedef std::map<QString, Texture, std::less<QString>> TextureMapType;
	typedef std::set<Color, CompareColor> ColorMapType;
	Theme();

	void save(const QDir &dir) const;
	void import(const QDir &dir); //soft load
	void load(const QDir &dir);
	void unload();

	void pack(const Theme *model = nullptr, Theme *taggerTheme = nullptr, bool usePixmaps = false);
	void unpack(const Theme *model=nullptr);

	void savePixmaps(const QDir &dir);
	void loadPixmaps(const QDir &dir);

	bool loadColors(const QDir &dir);
	bool saveColors(const QDir &dir) const;
	void fuseColors(const Theme *model);

	void setPath(const QDir &path);
	const QDir &path() const;
	const QString &name() const;

	bool isImported() const;
	bool isLoaded() const;
	bool isUnpacked() const;

	void copyTextures(const Theme &other);

	///\brief resetTextures sets all textures at value 0.
	void resetTextures();

	///\brief useToRemoveImagesIn removes all files in dir for which the name can be found in the map.
	void useToRemoveImagesIn(const QDir &dir);

	const TextureMapType &textures() const;
	TextureMapType &textures();
	const ColorMapType &colors() const;

	bool saveRemote() const;
	bool loadRemote();
	void setRemote(const QUrl &url);
	const QUrl &remote() const;
	bool unzip(const QFile &zipFile);

private:
	QString	m_name;
	QDir m_path;
	TextureMapType m_textures;
	ColorMapType m_colors;
	QUrl m_remote;
};

#endif // THEME_H
