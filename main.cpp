// Visualization lib includes
#ifdef USE_VISUALIZATION_CPP // (VISUALIZATION_ENABLED_CPP == TRUE) 
#define VISUALIZATION_ENABLED TRUE
#include "includes/visualization/jones_plot_app.h"
#else
#endif

// Signal proc lib includes
#include "includes/signal_proc_lib/file_io.h"

// Qt includes
#include <QtWidgets/QApplication>
#include "qopenglwidget.h"

// STL includes
#include <string>

int main(int argc, char *argv[]) 
{
    std::cout << "argc:" << argc << ", arguments:" << std::endl;
    for ( int idx = 0; idx < argc; ++idx ) {
        std::cout << std::to_string(*argv[idx]) << std::endl;
    }
    
    //Do this before QApplication a(argc, arv)
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
	QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
	QApplication a(argc, argv);
    
   #ifdef VISUALIZATION_ENABLED
    // temporary solution
    JonesPlotApplication_C w;
    w.show();
   #endif

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
