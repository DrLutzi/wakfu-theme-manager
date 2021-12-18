#include "theme.h"

extern QJsonDocument _jsonThemes;

Theme::Theme() :
	m_name(),
	m_path(),
	m_textures(),
	m_colors()
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

	QDir colorsDir(dir.absolutePath() + "/colors");
	saveColors(dir);
}

void Theme::load(const QDir &dir)
{
	m_colors.clear();
	loadColors(dir);

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
	fuseColors(model);
	for(std::pair<const QString, Texture> &pair : m_textures)
	{
		if(model != nullptr)
		{
			TextureMapType::const_iterator cit = model->textures().find(pair.first);
			if(cit != model->textures().end())
			{
				TextureMapType::const_reference texturePair = (*cit);
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
	for(TextureMapType::reference pair : m_textures)
	{
		if(model != nullptr)
		{
			TextureMapType::const_iterator cit = model->textures().find(pair.first);
			if(cit != model->textures().end())
			{
				TextureMapType::const_reference texturePair = (*cit);
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
		QDir path(dir.absolutePath() + "/" + pair.first);
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

bool Theme::loadColors(const QDir &dir)
{
	QDir colorsPath(dir.absolutePath() + "/colors");
	QDomDocument xmlColorsDocument;
	QFile colorsFile(colorsPath.absolutePath() + "/colors.xml");
	bool b = colorsFile.open(QIODevice::ReadOnly);
	if(b)
	{
		xmlColorsDocument.setContent(&colorsFile);
		QDomElement root = xmlColorsDocument.documentElement();
		QString type = root.tagName();
		if(type != QString("colors"))
		{
			b = false;
		}
		if(b)
		{
			QDomNodeList list(root.childNodes());
			for(int i=0; i<list.size(); ++i)
			{
				QDomNode node = list.item(i);
				if(!node.isNull() && node.isElement())
				{
					 QDomElement element = node.toElement();
					 if(element.tagName() == "color")
					 {
						 QDomNamedNodeMap attributes = element.attributes();
						 QDomNode nodeId = attributes.namedItem("id");
						 QString stringId = nodeId.nodeValue();
						 QDomNode nodeColor = attributes.namedItem("color");
						 QString stringColor = nodeColor.nodeValue();
						 Color c(stringId, stringColor);
						 m_colors.emplace(c);
					 }
				}
			}
		}
		colorsFile.close();
	}
	return b;
}

bool Theme::saveColors(const QDir &dir) const
{
	QDir colorsPath(dir.absolutePath() + "/colors");
	colorsPath.mkpath(colorsPath.absolutePath());
	QDomDocument xmlColorsDocument;
	QFile colorsFile(colorsPath.absolutePath() + "/colors.xml");
	bool b = colorsFile.open(QIODevice::WriteOnly);
	if(b)
	{
		QDomElement colorsElement(xmlColorsDocument.createElement(QString("colors")));
		xmlColorsDocument.appendChild(colorsElement);
		for(ColorMapType::const_iterator cit = m_colors.begin(); cit != m_colors.end(); ++cit)
		{
			const Color &color = (*cit);
			QDomElement colorElement(xmlColorsDocument.createElement(QString("color")));
			colorElement.setAttribute("id", color.id());
			colorElement.setAttribute("color", color.colorName());
			colorsElement.appendChild(colorElement);
		}
		QTextStream stream(&colorsFile);
		stream << xmlColorsDocument.toString();

		colorsFile.close();
	}
	return b;
}

void Theme::fuseColors(const Theme *model)
{
	if(model != nullptr)
	{
		const ColorMapType &colors = model->colors();
		for(ColorMapType::const_iterator cit = colors.begin(); cit != colors.end(); ++cit)
		{
			const Color &color = (*cit);
			ColorMapType::iterator oldColorIt = m_colors.find(color);
			if(oldColorIt != m_colors.end())
			{
				m_colors.erase(oldColorIt);
			}
			m_colors.insert(color);
		}
	}
	return;
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

const Theme::TextureMapType &Theme::textures() const
{
	return m_textures;
}

const Theme::ColorMapType &Theme::colors() const
{
	return m_colors;
}
