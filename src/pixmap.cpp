#include "pixmap.h"

std::map<QString, Pixmap::Position> Pixmap::ms_positionMap;
std::map<QString, Pixmap::Rotation> Pixmap::ms_rotationMap;
bool Pixmap::ms_uninitialized = true;

Pixmap::Pixmap(int index) :
	m_index(index)
{
	if(ms_uninitialized)
	{
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

bool Pixmap::load(const QFile &file)
{
	QFileInfo fileInfo(file);
	m_id = fileInfo.baseName();
	QFile txtFile(fileInfo.absoluteDir().absolutePath() + fileInfo.baseName() + ".info");
	if(txtFile.open(QIODevice::ReadOnly))
	{
		m_id = QString(txtFile.readAll()).toInt();
	}
	else
	{
		m_id = -1;
	}
	return m_image.load(file.fileName());
}

bool Pixmap::save(const QFile &file) const
{
	QFileInfo fileInfo(file);
	QFile txtFile(fileInfo.absoluteDir().absolutePath() + "/" + m_id + ".info");
	txtFile.open(QIODevice::WriteOnly | QIODevice::Text);
	txtFile.write(QString::number(m_index).toStdString().c_str());
	txtFile.close();
	return m_image.save(file.fileName());
}

bool Pixmap::save(const QDir &directory) const
{
	QFile txtFile(directory.absolutePath() + "/" + m_id + ".info");
	txtFile.open(QIODevice::WriteOnly | QIODevice::Text);
	txtFile.write(QString::number(m_index).toStdString().c_str());
	txtFile.close();
	return m_image.save(directory.absolutePath() + "/" + m_id + ".png");
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
