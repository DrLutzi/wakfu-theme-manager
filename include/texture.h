#ifndef TEXTURE_H
#define TEXTURE_H

#include "pixmap.h"
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class Texture
{
public:
	typedef std::map<QString, Pixmap, std::less<QString>> MapType;

	Texture(const QString &id = QString());

	bool save(const QFile &file) const;
	bool load(const QFile &file);

	bool savePixmaps(const QDir &dir) const;
	bool loadPixmaps(const QDir &dir);

	void pack();
	void unpack();

	void smartPack();
	void smartUnpack();

	const QString &id() const;

private:
	QString m_id;
	QImage	m_image;
	MapType m_pixmaps;
};

#endif // TEXTURE_H
