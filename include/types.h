#ifndef TYPES_H
#define TYPES_H

#include <QDir>
#include <QUrl>

struct AppParameters
{
public:

	bool operator==(const AppParameters &other)
	{
		return outputPath == other.outputPath && themesPath == other.themesPath && jsonUrl == other.jsonUrl;
	}
	bool operator!=(const AppParameters &other)
	{
		return !operator==(other);
	}

	QDir outputPath;
	QDir themesPath;
	QUrl jsonUrl;
};

#endif // TYPES_H
