#include "texture.h"

extern QJsonDocument _jsonThemes;
std::map<QString, QString, std::less<QString>> Texture::ms_pathToIdMap;

Texture::Texture(const QString &id) :
	m_pathId(id)
{}

bool Texture::save(const QFile &file) const
{
	return m_image.save(file.fileName());
}

bool Texture::load(const QFile &file)
{
	QFileInfo fileInfo(file);
	m_pathId = fileInfo.baseName();
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
	m_pixmaps.clear();
	QFileInfoList ls = dir.entryInfoList(QStringList() << "*.png", QDir::Files);
	const QJsonValue &pixmapValues = _jsonThemes["pixmaps"];
	for(QFileInfoList::ConstIterator cit = ls.constBegin(); cit != ls.constEnd(); ++cit)
	{
		const QFileInfo &fileInfo = (*cit);
		QString baseName = fileInfo.baseName();
		Pixmap pixmap;
		pixmap.load(fileInfo.absoluteFilePath());
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
	const QJsonArray &jsonArray = pixmapValues.toArray();
	unsigned int index=0;
	bool isSpecific = false;
	for(QJsonArray::ConstIterator cit = jsonArray.constBegin(); cit != jsonArray.constEnd(); ++cit, ++index)
	{
		const QJsonValue &pixmapJson = (*cit);
		const QString &textureId = pixmapJson["texture"].toString();
		if(textureId == ms_pathToIdMap[m_pathId])
		{
			const QJsonValue &pixmapJson(pixmapValues[index]);
			const QJsonObject &objectJson = pixmapJson.toObject();
			const QString &id = objectJson["id"].toString();
			createAndAppendPixmap(objectJson, id, index, isSpecific, -1, -1);
		}
	}
	unpackSpecific();
}

void Texture::initPathToIdMapFromJson(const QJsonArray &textureJsonArray)
{
	for(QJsonArray::ConstIterator cit = textureJsonArray.constBegin(); cit !=textureJsonArray.constEnd(); ++cit)
	{
        const QJsonValue &value = (*cit);
        const QString &fileString = value["path"].toString();
        const QString &idString = value["id"].toString();
        QFileInfo fileInfo(fileString);
        ms_pathToIdMap.insert(std::make_pair(fileInfo.baseName(), idString));
	}
}

void Texture::unpackSpecific()
{
	assert(!m_image.isNull());

	const QJsonValue &themeElementValues = _jsonThemes["themeElement"];
	const QJsonArray &jsonArray = themeElementValues.toArray();
	unsigned int index=0;
	bool isSpecific = true;
	std::function<void (const QJsonObject &, const QString &, int)> lmbd_rec_unpackAllSpecificPixmaps = [&](const QJsonObject &jsonObject, const QString &id, int depth)
	{
		const QJsonValue &specificPixmapsValue = jsonObject["specificPixmaps"];
		const QJsonArray &specificPixmapsArray = specificPixmapsValue.toArray();
		int specificIndex = 0;
		for(QJsonArray::ConstIterator cit = specificPixmapsArray.constBegin(); cit != specificPixmapsArray.constEnd(); ++cit, ++specificIndex)
		{
			const QJsonObject &specificPixmapObject = (*cit).toObject();
			const QString &textureId = specificPixmapObject["texture"].toString();
			if(textureId == ms_pathToIdMap[m_pathId])
			{
				createAndAppendPixmap(specificPixmapObject, id, index, isSpecific, depth, specificIndex);
			}
		}
		const QJsonValue &subThemeElements = jsonObject["childrenThemeElements"];
		if(!subThemeElements.isNull() && !subThemeElements.isUndefined())
		{
			const QJsonArray &subThemeElementsArray = subThemeElements.toArray();
			for(QJsonArray::ConstIterator cit = subThemeElementsArray.constBegin(); cit != subThemeElementsArray.constEnd(); ++cit)
			{
				lmbd_rec_unpackAllSpecificPixmaps((*cit).toObject(), id, depth+1);
			}
		}
	};

	for(QJsonArray::ConstIterator cit = jsonArray.constBegin(); cit != jsonArray.constEnd(); ++cit, ++index)
	{
		const QJsonValue &themeElementJson = (*cit);
		const QJsonObject &jsonObject = themeElementJson.toObject();
		const QString &id = jsonObject["id"].toString();
		lmbd_rec_unpackAllSpecificPixmaps(jsonObject, id, 0);
	}
}

void Texture::createAndAppendPixmap(const QJsonObject &objectJson, const QString &id, int index, bool isSpecific, int depth, int specificIndex)
{
	Pixmap pixmap(index, isSpecific, depth, specificIndex);
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
	QImage pixmapImage = m_image.copy(xy.width(), xy.height(), size.width(), size.height());
	pixmap.setImage(pixmapImage);
	pixmap.set(id, size, position, rotation, xy, flipHorizontally, flipVertically);
	m_pixmaps.emplace(std::make_pair(id, pixmap));
	return;
}

const QString &Texture::id() const
{
	return m_pathId;
}

bool Texture::isUnpacked() const
{
	return m_pixmaps.size()>0;
}
