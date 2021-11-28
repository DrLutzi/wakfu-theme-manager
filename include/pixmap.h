#ifndef PIXMAP_H
#define PIXMAP_H

#include <QPixmap>
#include <QFileInfo>
#include <QDir>
#include <map>

extern QJsonDocument _jsonFile;

class Pixmap
{
public:

	typedef enum {Center, NW, N, NE, W, E, SW, S, SE, PositionInvalid} Position;
	typedef enum {Normal, QuarterClockwise, Half, QuarterCounterClockwise, RotationInvalid} Rotation;

	static Position positionToEnum(QString key);
	static Rotation rotationToEnum(QString key);

	Pixmap(int index = -1);

	bool load(const QFile &file);
	bool save(const QFile &file) const;
	bool save(const QDir &directory) const;

	void set(const QString &id, const QSize &size, Position position, Rotation rotation, const QSize &xy, bool flipH, bool flipV);
	void setImage(const QImage &image);

	int index() const;
	const QImage &image() const;

	const QString &id() const;
	const QSize &size() const;
	Position position() const;
	Rotation rotation() const;
	const QSize &xy() const;
	bool flipHorizontally() const;
	bool flipVertically() const;

private:
	QString m_id;
	QSize m_size;
	Position m_position;
	Rotation m_rotation;
	QSize m_xy;
	bool m_flipHorizontally;
	bool m_flipVertically;

	int m_index; //< index in json. Used to lower search complexity by one times n.

	QImage	m_image;

	static std::map<QString, Position> ms_positionMap;
	static std::map<QString, Rotation> ms_rotationMap;
	static bool ms_uninitialized;
};

#endif // PIXMAP_H
