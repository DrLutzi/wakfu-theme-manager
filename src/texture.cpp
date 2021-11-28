#include "texture.h"

extern QJsonDocument _jsonThemes;

Texture::Texture(const QString &id) :
	m_id(id)
{}

bool Texture::save(const QFile &file) const
{
	return m_image.save(file.fileName());
}

bool Texture::load(const QFile &file)
{
	QFileInfo fileInfo(file);
	m_id = fileInfo.baseName();
	return m_image.load(file.fileName());
}

bool Texture::savePixmaps(const QDir &dir) const
{
	for(MapType::const_iterator cit = m_pixmaps.begin(); cit != m_pixmaps.end(); ++cit)
	{
		const Pixmap &pixmap = (*cit).second;
		pixmap.save(dir);
	}
	return true;
}

bool Texture::loadPixmaps(const QDir &dir)
{
	QStringList ls = dir.entryList(QStringList() << "*.png", QDir::Files);
	const QJsonValue &pixmapValues = _jsonThemes["pixmaps"];
	for(QStringList::ConstIterator cit = ls.constBegin(); cit != ls.constEnd(); ++cit)
	{
		QFile file(dir.absolutePath() + "/" + (*cit));
		QFileInfo fileInfo(file);
		QString baseName = fileInfo.baseName();
		Pixmap pixmap;
		pixmap.load(file);
		int index = pixmap.index();
		if(index == -1)
		{
			//search for it
		}
		else
		{
			const QJsonValue &pixmapJson(pixmapValues[index]);
			const QJsonObject &objectJson = pixmapJson.toObject();
			//QString id = objectJson["id"].toString(); //todo: erreurs
			QSize size;
			size.setWidth(objectJson["width"].toInt());
			size.setHeight(objectJson["height"].toInt());
			Pixmap::Position position = Pixmap::positionToEnum(objectJson["position"].toString());
			Pixmap::Rotation rotation = Pixmap::rotationToEnum(objectJson["rotation"].toString());
			QSize xy;
			xy.setWidth(objectJson["x"].toInt());
			xy.setHeight(objectJson["y"].toInt());
			bool flipHorizontally;
			flipHorizontally = objectJson["flipHorizontally"].toBool();
			bool flipVertically;
			flipVertically = objectJson["flipVertically"].toBool();
			pixmap.set(baseName, size, position, rotation, xy, flipHorizontally, flipVertically);
		}
		m_pixmaps.emplace(std::make_pair(baseName, pixmap));
	}
	return true;
}

void Texture::pack()
{
	assert(!m_image.isNull());
	for(MapType::const_iterator cit = m_pixmaps.begin(); cit != m_pixmaps.end(); ++cit)
	{
		const Pixmap &pixmap = (*cit).second;
		unsigned int xEnd = std::min(std::min(pixmap.image().width(), m_image.width()-pixmap.xy().width()), pixmap.size().width());
		unsigned int yEnd = std::min(std::min(pixmap.image().height(), m_image.height()-pixmap.xy().height()), pixmap.size().height());
		for(unsigned int y=0; y<yEnd; ++y)
		{
			unsigned int dy = y+pixmap.xy().height();
			for(unsigned int x=0; x<xEnd; ++x)
			{
				unsigned int dx = x+pixmap.xy().width();
				m_image.setPixelColor(dx, dy, pixmap.image().pixelColor(x, y));
			}
		}
	}
}

void Texture::unpack()
{
	assert(!m_image.isNull());
	m_pixmaps.clear();
	const QJsonValue &pixmapValues = _jsonThemes["pixmaps"];
	QJsonArray jsonArray = pixmapValues.toArray();
	unsigned int index=0;
	for(QJsonArray::ConstIterator cit = jsonArray.constBegin(); cit != jsonArray.constEnd(); ++cit, ++index)
	{
		const QJsonValue &pixmapJson = (*cit);
		const QString &textureId = pixmapJson["texture"].toString();
		if(textureId == m_id)
		{
			Pixmap pixmap(index);
			const QJsonValue &pixmapJson(pixmapValues[index]);
			const QJsonObject &objectJson = pixmapJson.toObject();
			QString id = objectJson["id"].toString();
			QSize size;
			size.setWidth(objectJson["width"].toInt());
			size.setHeight(objectJson["height"].toInt());
			Pixmap::Position position = Pixmap::positionToEnum(objectJson["position"].toString());
			//Pixmap::Rotation rotation = Pixmap::rotationToEnum(objectJson["rotation"].toString());
			Pixmap::Rotation rotation = Pixmap::Normal;
			QSize xy;
			xy.setWidth(objectJson["x"].toInt());
			xy.setHeight(objectJson["y"].toInt());
			bool flipHorizontally;
			flipHorizontally = objectJson["flipHorizontally"].toBool();
			bool flipVertically;
			flipVertically = objectJson["flipVertically"].toBool();
			QImage pixmapImage = m_image.copy(xy.width(), xy.height(), size.width(), size.height());
			pixmap.setImage(pixmapImage);
			pixmap.set(id, size, position, rotation, xy, flipHorizontally, flipVertically);
			m_pixmaps.emplace(std::make_pair(id, pixmap));
		}
	}
}

const QString &Texture::id() const
{
	return m_id;
}
