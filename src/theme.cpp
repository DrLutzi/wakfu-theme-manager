#include "theme.h"

extern QJsonDocument _jsonThemes;

Theme::Theme()
{

}

void Theme::save(QDir dir) const
{
	for(const std::pair<QString, Texture> &pair : m_textures)
	{
		pair.second.save(dir.absolutePath() + "/" + pair.second.id() + ".png");
	}
}

void Theme::load(QDir dir)
{
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
			if( (*cit2)["path"].toString() == QString("theme/images/") + baseName + ".tga" )
				foundInJson = true;
			else
				++index;
		}
		if(foundInJson)
			m_textures.emplace(std::make_pair(baseName, texture));
	}
}
