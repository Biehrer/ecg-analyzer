// Project includes
#include "JonesPlot.h"

// Qt includes
#include <QtWidgets/QApplication>
#include "qopenglwidget.h"

int main(int argc, char *argv[])
{
    //Do this before QApplication a(argc, arv)
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
	QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

	QApplication a(argc, argv);
    
    JonesPlotApplication_C w;
	w.show();
   
     
	return a.exec();
}

// Coding guidelines
// variables:
// _var = member Variable


// order of includes:
//
// project includes
//   qt includes
//      stl includes
// If there is an error in a project file its recoginzed before other errors in third party dependencies
