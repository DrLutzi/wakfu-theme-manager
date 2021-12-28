#ifndef UNZIPPER_H
#define UNZIPPER_H

#include <QDir>
#include <memory>
#include <QProcess>

class Unzipper
{
public:

	Unzipper();

	static bool initialize();

	bool unzip(const QFile &zipFile, const QDir &outputDir, QObject *parent = nullptr);

private:

	static bool ms_isInitialized;
	static QString ms_programName;
	static QStringList ms_arguments;
};

#endif // UNZIPPER_H
