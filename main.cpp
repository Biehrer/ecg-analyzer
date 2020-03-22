// Project includes
#include "JonesPlot.h"

// Qt includes
#include <QtWidgets/QApplication>
#include "qopenglwidget.h"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);//BEFORE QAPPLLICATION a(argc, arv)
	QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    // THis is also done in JonesPlot.cpp -> Todo: remove one occurence!
//    QSurfaceFormat format;
//    format.setVersion(3,3);
//    format.setProfile(QSurfaceFormat::CoreProfile);
//    QSurfaceFormat::setDefaultFormat(format);

	QApplication a(argc, argv);
	JonesPlot w;
	w.show();
	return a.exec();
}
