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
