#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QTranslator translator;
	QStringList languageList = QLocale::system().uiLanguages();
	if (!languageList.isEmpty())
	{
		QString primaryUiLanguage = languageList.first();
		QString translatorFile(QString("wtm_" + primaryUiLanguage + ".qm"));
		bool b = translator.load(translatorFile, QCoreApplication::applicationDirPath());
		if(b)
		{
			a.installTranslator(&translator);
		}
	}
	MainWindow w;
	w.show();
	w.checkOutputExistence();
	return a.exec();
}
