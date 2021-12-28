#include "unzipper.h"

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
	//is tar installed on every build of windows?
	ms_programName = "Powershell.exe";
	ms_arguments << "Expand-Archive" << "-Force";
#else
	ms_programName = "unzip";
	ms_arguments << "";
#endif
	ms_isInitialized = true;
	return true;
}

bool Unzipper::unzip(const QFile &zipFile, const QDir &outputDir, QObject *parent)
{
	QProcess unzipProcess(parent);
	QStringList arguments = ms_arguments;
#if defined(Q_OS_WIN)
	arguments << zipFile.fileName() << outputDir.absolutePath();
#endif
	unzipProcess.start(ms_programName, arguments);
	return unzipProcess.waitForFinished();
}
