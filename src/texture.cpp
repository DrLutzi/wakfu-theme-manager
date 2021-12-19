#include "texture.h"

extern QJsonDocument _jsonThemes;
std::map<QString, QString, std::less<QString>> Texture::ms_pathToIdMap;

Texture::Texture(const QString &id) :
	m_pathId(id)
{}

Texture::Texture(const Texture &other) :
	m_pathId(other.pathId()),
	m_image(other.image()),
	m_pixmaps() //pixmaps are personnal. I could change my mind later.
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

void Texture::pack(const Texture *other, Texture *tagger, bool usePixmaps)
{
	assert(other != nullptr);
	assert(other->pathId() == m_pathId);
	//merge anything contained in the pixmaps.
	if(usePixmaps)
	{
		if(other == nullptr)
		{
			pack(m_pixmaps, tagger); //for testing purposes only.
		}
		else
		{
			const MapType &otherPixmaps = other->pixmaps();
			pack(otherPixmaps, tagger);
		}
	}
	//merge anything not contained in the pixmaps (Ankama's json does not seem to contain all necessary infos).
	if(!usePixmaps && other != nullptr && other->image().size() == m_image.size() && tagger != nullptr)
	{
		int yMax = m_image.size().height();
		int xMax = m_image.size().width();
		QColor color;
		for(int y=0; y<yMax; ++y)
		{
			for(int x=0; x<xMax; ++x)
			{
				color = other->image().pixelColor(x, y);
				if(tagger->image().pixel(x, y) < 1 && Pixmap::norm2Diff(m_image.pixelColor(x, y), color) > 0.016f)
				{
					tagger->image().setPixel(x, y, QRgb(0xFF00FF00));
					m_image.setPixelColor(x, y, color);
				}
			}
		}
	}
	return;
}

void Texture::pack(const MapType &pixmaps, Texture * const tagger)
{
	for(MapType::const_iterator cit = pixmaps.begin(); cit != pixmaps.end(); ++cit)
	{
		const Pixmap &pixmap = (*cit).second;
		int xEnd = std::min(std::min(pixmap.image().width(), m_image.width()-pixmap.xy().width()), pixmap.size().width());
		int yEnd = std::min(std::min(pixmap.image().height(), m_image.height()-pixmap.xy().height()), pixmap.size().height());
		for(int y=0; y<yEnd; ++y)
		{
			int dy = y+pixmap.xy().height();
			for(int x=0; x<xEnd; ++x)
			{
				int dx = x+pixmap.xy().width();
				m_image.setPixel(dx, dy, pixmap.image().pixel(x, y));
				if(tagger != nullptr)
				{
					tagger->image().setPixel(dx, dy, QRgb(0xFF0000FF));
				}
			}
		}
	}
	return;
}

void Texture::unpack(const Texture *model)
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
			createAndAppendPixmap(objectJson, id, index, isSpecific, -1, -1, model);
		}
	}
	unpackSpecific(model);
}

void Texture::initPathToIdMapFromJson(const QJsonArray &textureJsonArray)
{
	ms_pathToIdMap.clear();
	for(QJsonArray::ConstIterator cit = textureJsonArray.constBegin(); cit !=textureJsonArray.constEnd(); ++cit)
	{
        const QJsonValue &value = (*cit);
        const QString &fileString = value["path"].toString();
        const QString &idString = value["id"].toString();
        QFileInfo fileInfo(fileString);
        ms_pathToIdMap.insert(std::make_pair(fileInfo.baseName(), idString));
	}
}

void Texture::unpackSpecific(const Texture *model)
{
	assert(!m_image.isNull());

	const QJsonValue &themeElementValues = _jsonThemes["themeElement"];
	const QJsonArray &jsonArray = themeElementValues.toArray();
	unsigned int index=0;
	bool isSpecific = true;
	std::function<void (const QJsonObject &, const QString &, int)> lmbd_rec_unpackAllSpecificPixmaps = [&](const QJsonObject &jsonObject, const QString &pathId, int depth)
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
				createAndAppendPixmap(specificPixmapObject, pathId, index, isSpecific, depth, specificIndex, model);
			}
		}
		const QJsonValue &subThemeElements = jsonObject["childrenThemeElements"];
		if(!subThemeElements.isNull() && !subThemeElements.isUndefined())
		{
			const QJsonArray &subThemeElementsArray = subThemeElements.toArray();
			for(QJsonArray::ConstIterator cit = subThemeElementsArray.constBegin(); cit != subThemeElementsArray.constEnd(); ++cit)
			{
				lmbd_rec_unpackAllSpecificPixmaps((*cit).toObject(), pathId, depth+1);
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

void Texture::createAndAppendPixmap(const QJsonObject &objectJson, const QString &id,
									int index, bool isSpecific, int depth, int specificIndex,
									const Texture *model)
{
	Pixmap pixmap(index, isSpecific, depth, specificIndex);
	QSize size;
	size.setWidth(objectJson["width"].toInt());
	size.setHeight(objectJson["height"].toInt());
	QSize xy;
	xy.setWidth(objectJson["x"].toInt());
	xy.setHeight(objectJson["y"].toInt());
	bool foundExactCopy = false;
	QImage pixmapImage;
	if(size.width() != 0 && size.height() != 0)
	{
		pixmapImage = m_image.copy(xy.width(), xy.height(), size.width(), size.height());
	}
	else
	{
		foundExactCopy = true;
	}
	pixmap.setImage(pixmapImage);


//	//DEBUG ONLY
//	Pixmap::Position position = Pixmap::positionToEnum(objectJson["position"].toString());
//	Pixmap::Rotation rotation = Pixmap::rotationToEnum(objectJson["rotation"].toString());
//	bool flipHorizontally;
//	flipHorizontally = objectJson["flipHorizontally"].toBool();
//	bool flipVertically;
//	flipVertically = objectJson["flipVertically"].toBool();
//	pixmap.set(id, size, position, rotation, xy, flipHorizontally, flipVertically);
//	//

	if(model != nullptr && model->isUnpacked()) //if a model is set, do not create a pixmap if the same pixmap exists
	{
		//search for a pixmap having the same look

		std::pair<MapType::const_iterator, MapType::const_iterator> range = (model->pixmaps().equal_range(id));
		for(MapType::const_iterator cit = range.first; cit != range.second && !foundExactCopy; ++cit)
		{
			const std::pair<QString, Pixmap> &pair = (*cit);
			if(pair.second == pixmap)
			{
				foundExactCopy = true;
			}
		}
	}
	if(!foundExactCopy) //no need to store the pixmap if one was found.
	{
		Pixmap::Position position = Pixmap::positionToEnum(objectJson["position"].toString());
		Pixmap::Rotation rotation = Pixmap::rotationToEnum(objectJson["rotation"].toString());
		bool flipHorizontally;
		flipHorizontally = objectJson["flipHorizontally"].toBool();
		bool flipVertically;
		flipVertically = objectJson["flipVertically"].toBool();
		pixmap.set(id, size, position, rotation, xy, flipHorizontally, flipVertically);
		m_pixmaps.emplace(std::make_pair(id, pixmap));
	}
	return;
}

const QString &Texture::pathId() const
{
	return m_pathId;
}

const QImage &Texture::image() const
{
	return m_image;
}

QImage &Texture::image()
{
	return m_image;
}

bool Texture::isUnpacked() const
{
	return m_pixmaps.size()>0;
}

const Texture::MapType &Texture::pixmaps() const
{
	return m_pixmaps;
}

Texture &Texture::operator=(const Texture &other)
{
	m_pathId = other.pathId();
	m_image = other.image();
	return *this;
}
