#include "theme.h"

extern QJsonDocument _jsonThemes;

Theme::Theme() :
	m_name(),
	m_path()
{

}

void Theme::save(const QDir &dir) const
{
	QDir imagesDir(dir.absolutePath() + "/images");
	imagesDir.mkpath(imagesDir.absolutePath());
	for(const std::pair<const QString, Texture> &pair : m_textures)
	{
		pair.second.save(imagesDir.absolutePath() + "/" + pair.second.pathId() + ".png");
	}
}

void Theme::load(const QDir &dir)
{
	m_textures.clear();
	m_name = dir.dirName();
	m_path = dir;
	QDir imageDir = dir.absolutePath() + "/images";
	QStringList ls = imageDir.entryList(QStringList() << "*.png", QDir::Files);
	const QJsonValue &textureValue = _jsonThemes["textures"];
	const QJsonArray &textureArray = textureValue.toArray();
	for(QStringList::ConstIterator cit = ls.constBegin(); cit != ls.constEnd(); ++cit)
	{
		QFile file(imageDir.absolutePath() + "/" + (*cit));
		QFileInfo fileInfo(file);
		QString baseName = fileInfo.baseName();
		Texture texture(baseName);
		texture.load(file);
		bool foundInJson = false;
		unsigned int index = 0;
		for(QJsonArray::ConstIterator cit2 = textureArray.constBegin(); cit2 != textureArray.constEnd() && !foundInJson; ++cit2)
		{
            const QJsonValue &value = (*cit2);
			if( value["path"].toString() == QString("theme/images/") + baseName + ".tga" )
				foundInJson = true;
			else
				++index;
		}
		if(foundInJson)
			m_textures.emplace(std::make_pair(baseName, texture));
	}
}

void Theme::pack(const Theme *model)
{
	for(std::pair<const QString, Texture> &pair : m_textures)
	{
		if(model != nullptr)
		{
			MapType::const_iterator cit = model->textures().find(pair.first);
			if(cit != model->textures().end())
			{
				MapType::const_reference texturePair = (*cit);
				const Texture &modelTexture = texturePair.second;
				pair.second.pack(&modelTexture);
			}
		}
		else
			pair.second.pack(nullptr);
	}
	return;
}

void Theme::unpack(const Theme *model)
{
	for(MapType::reference pair : m_textures)
	{
		if(model != nullptr)
		{
			MapType::const_iterator cit = model->textures().find(pair.first);
			if(cit != model->textures().end())
			{
				MapType::const_reference texturePair = (*cit);
				const Texture &modelTexture = texturePair.second;
				pair.second.unpack(&modelTexture);
			}
		}
		else
			pair.second.unpack(nullptr);
	}
	return;
}

void Theme::savePixmaps(const QDir &dir)
{
	for(std::pair<const QString, Texture> &pair : m_textures)
	{
		QDir path = dir.absolutePath() + "/" + pair.first;
		dir.mkpath(path.absolutePath());
		pair.second.savePixmaps(path);
	}
}

void Theme::loadPixmaps(const QDir &dir)
{
	QFileInfoList ls = dir.entryInfoList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot);
	m_name = dir.dirName();
	m_path = dir;
	for(QFileInfoList::ConstIterator cit = ls.constBegin(); cit != ls.constEnd(); ++cit)
	{
		QString pathString = (*cit).absolutePath();
		QString id = pathString; //.section(QString("_pixmaps"), 0, 0); //when pixmap folders ended with _pixmap
		Texture texture(id);
		texture.loadPixmaps(pathString);
	}
}

const QDir &Theme::path() const
{
	return m_path;
}

const QString &Theme::name() const
{
	return m_name;
}

bool Theme::isInitialized() const
{
    return (!m_name.isEmpty() || m_textures.size()>0);
}

bool Theme::isOpened() const
{
	return m_textures.size()>0;
}

bool Theme::isUnpacked() const
{
	return isOpened() && m_textures.size()>0 && (*m_textures.begin()).second.isUnpacked();
}

void Theme::copyTextures(const Theme &other)
{
	m_textures = other.textures();
}

const Theme::MapType &Theme::textures() const
{
	return m_textures;
}
