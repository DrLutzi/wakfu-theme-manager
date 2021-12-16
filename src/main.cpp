#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QTranslator translator;
	if (QLocale::system().language() == QLocale::French)
	{
		QString translatorFile(QString("wtm_fr_FR.qm"));
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
