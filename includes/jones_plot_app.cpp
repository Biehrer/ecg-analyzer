#include "jones_plot_app.h"

JonesPlotApplication_C::~JonesPlotApplication_C() {
    delete _plot_widget;
}


void JonesPlotApplication_C::resizeEvent(QResizeEvent* event)
{
    //    _plot_widget->resize(event->size().width(), event->size().height());
}

JonesPlotApplication_C::JonesPlotApplication_C(QWidget *parent)
    : QMainWindow(parent)
{
    // updates ogl scene
    //    auto format = QSurfaceFormat::defaultFormat();
    //    format.setSwapInterval(0);
    //    format.setVersion(3, 3);
    //    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    //    format.setProfile(QSurfaceFormat::CoreProfile);
    //    format.setMinorVersion(1);
    //    format.setMajorVersion(2);
    //    format.setProfile(QSurfaceFormat::NoProfile);
    //    QSurfaceFormat::setDefaultFormat(format);
    ui.setupUi(this);

    unsigned int number_of_plots = 2;
    _plot_widget = new QOpenGLPlotRendererWidget(/*number_of_plots,*/ this);
    _plot_widget->show();

    connect(ui._btn_add_point, &QPushButton::clicked, this, &JonesPlotApplication_C::SendDatatToPlots);

    // Start thread to add data to the plots
    std::thread dataThread(&QOpenGLPlotRendererWidget::OnDataUpdateThreadFunction, _plot_widget);
    dataThread.detach();

}

//template<typename YValDataType_TP, typename XValDataType_TP>
void JonesPlotApplication_C::AddDataTest(double y_val, double x_val) {
    _plot_widget->AddDataToAllPlots(y_val, x_val);
}

void JonesPlotApplication_C::SendDatatToPlots() {
    _plot_widget->AddDataToAllPlots(ui._lineEdit_xVal->text().toFloat(), ui._lineEdit_yVal->text().toFloat());
}
