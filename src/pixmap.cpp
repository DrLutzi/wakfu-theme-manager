#include "pixmap.h"

std::map<QString, Pixmap::Position> Pixmap::ms_positionMap;
std::map<QString, Pixmap::Rotation> Pixmap::ms_rotationMap;
bool Pixmap::ms_uninitialized = true;

Pixmap::Pixmap(int index, bool isSpecific, int depth, int specificIndex) :
	m_index(index),
	m_isSpecific(isSpecific),
	m_depth(depth),
	m_specificIndex(specificIndex)
{
	if(ms_uninitialized)
	{ //I learned later that there is a better way to build a static map but f it
		ms_uninitialized = false;
		ms_positionMap.emplace(std::make_pair("center", Center));
		ms_positionMap.emplace(std::make_pair("north", N));
		ms_positionMap.emplace(std::make_pair("south", S));
		ms_positionMap.emplace(std::make_pair("west", W));
		ms_positionMap.emplace(std::make_pair("east", E));
		ms_positionMap.emplace(std::make_pair("north_west", NW));
		ms_positionMap.emplace(std::make_pair("north_east", NE));
		ms_positionMap.emplace(std::make_pair("south_west", SW));
		ms_positionMap.emplace(std::make_pair("south_east", SE));
		ms_positionMap.emplace(std::make_pair(QString(), PositionInvalid));

		ms_rotationMap.emplace(std::make_pair(QString(), Normal));
		ms_rotationMap.emplace(std::make_pair("quarter_clockwise", QuarterClockwise));
		ms_rotationMap.emplace(std::make_pair("quarter_counter_clockwise", QuarterCounterClockwise));
		ms_rotationMap.emplace(std::make_pair("half", Half));
	}
}

Pixmap::Position Pixmap::positionToEnum(QString key)
{
	std::map<QString, Pixmap::Position>::const_iterator cit = ms_positionMap.find(key);
	return cit != ms_positionMap.end() ? (*cit).second : PositionInvalid;
}

Pixmap::Rotation Pixmap::rotationToEnum(QString key)
{
	std::map<QString, Pixmap::Rotation>::const_iterator cit = ms_rotationMap.find(key);
	return cit != ms_rotationMap.end() ? (*cit).second : Normal;
}

bool Pixmap::load(const QFile &file, const QFile &infoFile)
{
	QFileInfo fileInfo(file);
	QFile effectiveInfoFile;
	if(!infoFile.fileName().isEmpty() || !infoFile.fileName().isNull())
	{
		effectiveInfoFile.setFileName(infoFile.fileName());
	}
	else
	{
		QFileInfo IFile(file);
		effectiveInfoFile.setFileName(IFile.absoluteDir().absolutePath() + IFile.baseName() + ".info");
	}
	m_id = fileInfo.baseName();
	if(effectiveInfoFile.open(QIODevice::ReadOnly))
	{
		m_index = QString(effectiveInfoFile.readLine()).toInt();
		m_isSpecific = bool(QString(effectiveInfoFile.readLine()).toInt());
	}
	else
	{
		m_index = -1;
		m_isSpecific = false;
	}
	return m_image.load(file.fileName());
}

bool Pixmap::save(const QFile &file, const QFile &infoFile) const
{
	QFileInfo fileInfo(file);
	QFile effectiveInfoFile;
	if(!infoFile.fileName().isEmpty() || !infoFile.fileName().isNull())
	{
		effectiveInfoFile.setFileName(infoFile.fileName());
	}
	else
	{
		effectiveInfoFile.setFileName(fileInfo.absoluteDir().absolutePath() + "/" + m_id + ".info");
	}
	effectiveInfoFile.open(QIODevice::WriteOnly | QIODevice::Text);
	effectiveInfoFile.write((QString::number(m_index) + '\n').toStdString().c_str());
	effectiveInfoFile.write(QString::number(m_isSpecific).toStdString().c_str());
	effectiveInfoFile.close();
	return m_image.save(file.fileName());
}

bool Pixmap::save(const QDir &directory) const
{
	QFile file;
	if(m_isSpecific)
	{
		QDir substituteDir(directory.absolutePath() + "/" + m_id + "/" + QString::number(m_depth));
		substituteDir.mkpath(substituteDir.absolutePath());
		file.setFileName(substituteDir.absolutePath() + "/" + QString::number(m_specificIndex) + ".png");
	}
	else
	{
		file.setFileName(directory.absolutePath() + "/" + QString(m_id) + ".png");
	}
	return save(file);
}

void Pixmap::set(const QString &id, const QSize &size, Position position, Rotation rotation, const QSize &xy, bool flipH, bool flipV)
{
	m_id = id;
	m_size = size;
	m_position = position;
	m_rotation = rotation;
	m_xy = xy;
	m_flipHorizontally = flipH;
	m_flipVertically = flipV;
}

void Pixmap::setImage(const QImage &image)
{
	m_image = image;
}

int Pixmap::index() const
{
	return m_index;
}

const QImage &Pixmap::image() const
{
	return m_image;
}

const QString &Pixmap::id() const
{
	return m_id;
}

const QSize &Pixmap::size() const
{
	return m_size;
}

Pixmap::Position Pixmap::position() const
{
	return m_position;
}

Pixmap::Rotation Pixmap::rotation() const
{
	return m_rotation;
}

const QSize &Pixmap::xy() const
{
	return m_xy;
}

bool Pixmap::flipHorizontally() const
{
	return m_flipHorizontally;
}

bool Pixmap::flipVertically() const
{
	return m_flipVertically;
}

int Pixmap::depth() const
{
	return m_depth;
}

int Pixmap::specificIndex() const
{
	return m_specificIndex;
}

bool Pixmap::operator==(const Pixmap &other) const
{
	bool b;
	b = m_index == other.index() && m_specificIndex == other.specificIndex() && m_depth == other.depth(); //quick check

	//I put that aside in case it becomes relevant one day (compare image norm2 instead of pixel/pixel norm2)
//	auto norm2Images = [&] (const QImage &image1, const QImage &image2) -> float
//	{
//		float norm = 0;
//		for(int y=0; y<image1.height(); ++y)
//		{
//			for(int x=0; x<image1.width(); ++x)
//			{
//				//b = image.pixelColor(x, y) == m_image.pixelColor(x, y);
//				const QColor &pix1 = image1.pixelColor(x, y);
//				const QColor &pix2 = image2.pixelColor(x, y);
//				norm += norm2Diff(pix1, pix2);
//			}
//		}
//		return norm/(image1.width()*image1.height());
//	};
	if(b)
	{
		const QImage &image = other.image();
		if(image.size() == m_image.size())
		{
			b = true;
			for(int y=0; y<image.height() && b; ++y)
			{
				for(int x=0; x<image.width() && b; ++x)
				{
					//b = image.pixelColor(x, y) == m_image.pixelColor(x, y);
					QColor c1 = image.pixelColor(x, y);
					QColor c2 = m_image.pixelColor(x, y);
					float n = norm2Diff(c1, c2);
					b = n < 0.008f; //tolerates a difference of 2/255 (small accidental changes).
				}
			}
//			float n = norm2Images(m_image, image); //try comparing images if fail cases? (longer)
//			b = n < 0.02f;
		}
		else
		{
			b = false;
		}
	}
	return b;
}

bool Pixmap::norm2Diff(const QColor &color1, const QColor &color2)
{
	float r = color1.alphaF()*color1.redF() - color2.alphaF()*color2.redF();
	float g = color1.alphaF()*color1.greenF() - color2.alphaF()*color2.greenF();
	float b = color1.alphaF()*color1.blueF() - color2.alphaF()*color2.blueF();
	return sqrt(r*r + g*g + b*b)/3.0f;
}
