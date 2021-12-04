#include "theme.h"

extern QJsonDocument _jsonThemes;

Theme::Theme() :
	m_name(),
	m_path()
{

}

void Theme::save(const QDir &dir) const
{
	for(const std::pair<const QString, Texture> &pair : m_textures)
	{
		pair.second.save(dir.absolutePath() + "/" + pair.second.id() + ".png");
	}
}

void Theme::load(const QDir &dir)
{
	m_textures.clear();
	m_name = dir.dirName();
	m_path = dir;
	QStringList ls = dir.entryList(QStringList() << "*.png", QDir::Files);
	const QJsonValue &textureValue = _jsonThemes["textures"];
	const QJsonArray &textureArray = textureValue.toArray();
	for(QStringList::ConstIterator cit = ls.constBegin(); cit != ls.constEnd(); ++cit)
	{
		QFile file(dir.absolutePath() + "/" + (*cit));
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

void Theme::pack()
{
	for(std::pair<const QString, Texture> &pair : m_textures)
	{
		pair.second.pack();
	}
	return;
}

void Theme::unpack()
{
	for(std::pair<const QString, Texture> &pair : m_textures)
	{
		pair.second.unpack();
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
