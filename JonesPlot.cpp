#include "JonesPlot.h"

JonesPlotApplication_C::~JonesPlotApplication_C() {
    delete plot_widget;
}


void JonesPlotApplication_C::resizeEvent(QResizeEvent* event)
{
//    plot_widget->resize(event->size().width(), event->size().height());
}

JonesPlotApplication_C::JonesPlotApplication_C(QWidget *parent)
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
	
    plot_widget = new QOpenGLPlotWidget(this);
    //plot_widget->showFullScreen();
    plot_widget->show();

    //    if( !singlePlot->IsActive() ){
    //        abort!
    //    }

    // Start thread to add data to the plots
    std::thread dataThread(&QOpenGLPlotWidget::OnDataUpdateThreadFunction, plot_widget);
    dataThread.detach();
}