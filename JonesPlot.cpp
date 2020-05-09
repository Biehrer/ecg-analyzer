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
    //ui.openGLWidget->show();
    //ui.openGLWidget = new QOpenGLPlotRendererWidget();
   
    plot_widget = new QOpenGLPlotRendererWidget(this);
    plot_widget->show();
   
    connect(ui._btn_add_point, &QPushButton::clicked, this, &JonesPlotApplication_C::SendDatatToPlots);

    //    if( !plot_widget->IsActive() ){
    //        abort!
    //    }

    // Start thread to add data to the plots
    std::thread dataThread(&QOpenGLPlotRendererWidget::OnDataUpdateThreadFunction, plot_widget);
    dataThread.detach();
}

void JonesPlotApplication_C::SendDatatToPlots() {

    plot_widget->AddDataToAllPlots( ui._lineEdit_xVal->text().toFloat() , ui._lineEdit_yVal->text().toFloat()  );
}
