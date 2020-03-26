#include "JonesPlot.h"

JonesPlot::~JonesPlot() {

	delete singlePlot;
}
JonesPlot::JonesPlot(QWidget *parent)
	: QMainWindow(parent)
{
	//updates ogl scene
//    auto format = QSurfaceFormat::defaultFormat();
//    format.setSwapInterval(0);
//    format.setVersion(3, 3);
//    //format.setProfile(QSurfaceFormat::CompatibilityProfile);
//    format.setProfile(QSurfaceFormat::CoreProfile);
//    format.setMinorVersion(1);
//    format.setMajorVersion(2);
//    format.setProfile(QSurfaceFormat::NoProfile);
//    QSurfaceFormat::setDefaultFormat(format);

    ui.setupUi(this);
	
	singlePlot = new QOpenGLPlotter(this);
	singlePlot->show();

    //    if( !singlePlot->IsActive() ){
    //        abort!
    //    }

    // Start thread to add data to the plots
//    std::thread dataThread(&QOpenGLPlotter::dataThreadFunc, singlePlot);
//    dataThread.detach();
}
