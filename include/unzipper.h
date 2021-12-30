#ifndef UNZIPPER_H
#define UNZIPPER_H

#include <QDir>
#include <memory>
#include <QProcess>
#include <QSet>

class Unzipper
{
public:

	Unzipper();

	static bool initialize();

	bool unzip(const QFile &zipFile, const QDir &outputDir, QObject *parent = nullptr);
	QStringList createdEntries();

private:

	static bool ms_isInitialized;
	static QString ms_programName;
	static QStringList ms_arguments;

	QStringList m_oldEntryList;
	QStringList m_newEntryList;
};

#endif // UNZIPPER_H
