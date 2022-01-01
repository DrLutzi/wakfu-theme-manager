#include "unzipper.h"
#include <QtGlobal>

bool Unzipper::ms_isInitialized(false);
QString Unzipper::ms_programName("");
QStringList Unzipper::ms_arguments;

Unzipper::Unzipper()
{
	if(!ms_isInitialized)
	{
		initialize();
	}
}

bool Unzipper::initialize()
{
#if defined(Q_OS_WIN)
	ms_programName = "Powershell.exe";
	ms_arguments << "Expand-Archive" << "-Force";
#else
	ms_programName = "unzip";
	ms_arguments << "-o";
#endif
	ms_isInitialized = true;
	return true;
}

bool Unzipper::unzip(const QFile &zipFile, const QDir &outputDir, QObject *parent)
{
	m_oldEntryList = outputDir.entryList(QStringList(), QDir::NoDotAndDotDot|QDir::Files|QDir::Dirs);
	QProcess unzipProcess(parent);
	QStringList arguments = ms_arguments;
	QString quote('\"');
#if defined(Q_OS_WIN)
	arguments << quote + zipFile.fileName() + quote << quote + outputDir.absolutePath() + quote;
#else
	arguments << quote + zipFile.fileName() + quote << "-d" << quote + outputDir.absolutePath() + quote;
#endif
	unzipProcess.setStandardErrorFile(outputDir.absolutePath() + "/unzip.log");
	unzipProcess.start(ms_programName, arguments);
	bool b = unzipProcess.waitForFinished();
	m_newEntryList = outputDir.entryList(QStringList(), QDir::NoDotAndDotDot|QDir::Files|QDir::Dirs);
	return b;
}

QStringList Unzipper::createdEntries()
{
	QSet<QString> oldSet = QSet<QString>(m_oldEntryList.constBegin(), m_oldEntryList.constEnd());
	QSet<QString> newSet = QSet<QString>(m_newEntryList.constBegin(), m_newEntryList.constEnd());
	QSet<QString> diffSet = newSet.subtract(oldSet);
	QStringList list = QStringList(diffSet.constBegin(), diffSet.constEnd());
	return list;
}
