// Project includes
#include "jones_plot_app.h"

// Qt includes
#include <QtWidgets/QApplication>

// STL includes
#include <string>
#include <iostream>

int main(int argc, char *argv[])
{
    std::cout << "argc: " << argc << ", arguments:" << std::endl;
    for ( int idx = 0; idx < argc; ++idx ) {
        std::cout << std::to_string(*argv[idx]) << std::endl;
    }

    // auto format = QSurfaceFormat::defaultFormat();
    // 0 - Disable VSync; 1 - Enable VSync; > 1 - check documentary!
    // format.setSwapInterval(1); 

    // format.setVersion(3, 3);
    // format.setMinorVersion(1);
    // format.setMajorVersion(2);

    // format.setProfile(QSurfaceFormat::CompatibilityProfile);
    // format.setProfile(QSurfaceFormat::CoreProfile);
    // format.setProfile(QSurfaceFormat::NoProfile);
    
    // QSurfaceFormat::setDefaultFormat(format);

    // Do this ogl stuff before the line: QApplication a(argc, arv)
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QApplication a(argc, argv);
    
    JonesPlotApplication_C j;    
    j.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    j.showFullScreen();
    j.Setup();
    j.showNormal();
    j.showMaximized();

   // #ifdef VISUALIZATION_ENABLED
   //#endif

	return a.exec();
}

//template<typename DataType_TP>
//int ReadChannelFromUser(const TimeSignal_C<DataType_TP>& signal) 
//{
//    std::cout << std::endl << "Welcome!" << std::endl
//        << "select channel to display by entering the channel id : " << std::endl;
//
//    // Ask the user to which channel from the signal should be visualized
//    int channel_id = 0;
//    for ( const auto& channel_label : signal.GetChannelLabels() ) {
//        std::cout <<"channel id " << channel_id << " = "<< channel_label << std::endl;
//        ++channel_id;
//    }
//    std::cin >> channel_id;
//    std::cout << "start playing of data series from channel # :" << channel_id << std::endl;
//
//}

////////////////////////////////////////////////////
// 
//    Coding guidelines
//
////////////////////////////////////////////////////

//////////////////////
// Text format:
// - Use only spaces and no tabs !
// - Always leave 4 spaces to the left side of the page on each line! 

//////////////////////
// variables:
//
// - member variables start with a underscore:
// _var = member Variable
//
// - local variables do not:
// some_var = local variable
//
// - separate words by underscore and not camel case:
// number_of_points and NOT numberOfPoints;

//////////////////////
// functions:
// - Use CamelCase as function name:
//   FunctionNamesInCamelCase()

//////////////////////
// Parantheses of IF and WHILE statements:
//
// if there is one condition:
//     if ( condition1 ) {
//        DoStuff();
//     }
//
// if there is more than one condition:
//     if ( condition1 &&
//          condition2 )
//       {
//            DoStuff();
//       }

//////////////////////
// order of includes:
//
// project includes
//   qt includes
//      stl includes
//
//=> If there is an error in a project file its recoginzed before other errors in third party dependencies
