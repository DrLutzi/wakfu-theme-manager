#ifndef TYPES_H
#define TYPES_H

#include <QDir>
#include <QUrl>

#ifdef Q_OS_LINUX
	static const QString _defaultThemeDirectory (QDir::homePath() + "/.config/zaap/wakfu/theme");
#elif defined(Q_OS_WIN32)
	static const QString _defaultThemeDirectory (QDir::homePath() + "/AppData/Roaming/zaap/wakfu/theme");
#endif
	static const QString _defaultJsonThemeUrl ("https://wakfu.cdn.ankama.com/gamedata/theme/theme.json");

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
