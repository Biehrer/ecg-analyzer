#include "jones_plot_app.h"

JonesPlotApplication_C::~JonesPlotApplication_C() {
}

void JonesPlotApplication_C::resizeEvent(QResizeEvent* event)
{
    int width = event->size().width();
    int height = event->size().height();
    int i_height = this->height();
    int i_width = this->width();
    //ui._openGL_widget->resize(event->size().width(), event->size().height());
    ui._central_widget->resize(width, height);
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
    
    connect(this, SIGNAL(resize()), ui._openGL_widget, SLOT(resize()));

    ui._central_widget->setLayout(ui._grid_layout_main);

    ui._openGL_widget->show();
    this->update();
    //connect(_paint_update_timer, SIGNAL(timeout()), this, SLOT(update()));

}

void JonesPlotApplication_C::Setup() 
{
    int height = ui._openGL_widget->height();
    int width = ui._openGL_widget->width();

    int thi_height = this->height();
    int thi_width = this->width();
    int u_height = ui._central_widget->height();

    int number_of_plots = 2;
    bool success = ui._openGL_widget->FastInitializePlots(number_of_plots, 10000.0, 3, -3);
    if ( !success ) {
        throw std::runtime_error("plot initialization failed! Abort");
    }
     // Alternative to fast initialize: 
    // describe the plot using the PlotDescription_TP struct and 
    // add the plot to the plot_widget by calling
    // AddPlot(...)  with the PlotDescription_TP object ::

    // PlotDescription_TP plot0_info;
    // plot0_info._geometry = OGLChartGeometry(pos_x, pos_y, width, height)
    // ...
    // plot_widget.AddPlot(plot0_info);

    // Start a thread which adds the data to the plot(s)
    std::thread dataThread([&]() {

        // Create a time signal and fill it with data
        TimeSignal_C<float> signal;
        signal.ReadG11Data("C://Development//projects//EcgAnalyzer//ecg-analyzer//resources//G11Data.dat");

        // assign plot labels just for fun
        auto plot_0 = ui._openGL_widget->GetPlotPtr(0);
        plot_0->SetLabel("plot 0");
        auto plot_1 = ui._openGL_widget->GetPlotPtr(1);
        plot_1->SetLabel("plot 1");

        // Get data of all channels
        const auto& data = signal.constData();
        if ( data.empty() ) {
            throw std::runtime_error("Signal is empty!!");
        }
        // data for plot 0
        int plot0_id = plot_0->GetID() + 2;
        const auto& plot0_data = data[plot0_id]._data;
        const auto& plot0_timestamps = data[plot0_id]._timestamps;
        // iterator to the data for plot 0
        auto series_1_begin_it = plot0_data.begin();
        auto timestamps_1_begin_it = plot0_timestamps.begin();

        // data to plot 1
        int plot1_id = plot_1->GetID() + 3;
        const auto& plot1_data = data[plot1_id]._data;
        const auto& plot1_timestamps = data[plot1_id]._timestamps;
        // iterator to the data for plot 1
        auto series_2_begin_it = plot1_data.begin();
        auto timestamps_2_begin_it = plot1_timestamps.begin();
        
        // Hide all this pointer stuff in convenience methods so we can use:
        // GetChannelData(int channel_idx)

        double frequency_hz = data[plot0_id]._sample_rate_hz;
        double frequency_ms = (1.0 / frequency_hz) * 1000.0;
        bool signal_processed = false;
        int64_t time_series_end = plot0_data.size();

        while ( !signal_processed ) {
            if ( *series_1_begin_it < time_series_end ) {
                plot_0->AddDatapoint(*series_1_begin_it, *timestamps_1_begin_it);
                plot_1->AddDatapoint(*series_2_begin_it, *timestamps_2_begin_it);

                ++series_1_begin_it;
                ++timestamps_1_begin_it;
                ++series_2_begin_it;
                ++timestamps_2_begin_it;
            }
            else {
                signal_processed = true;
                std::cout << "processing finished; thread returns" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frequency_ms)));
        }
    });
    dataThread.detach();
}

//template<typename YValDataType_TP, typename XValDataType_TP>
//void JonesPlotApplication_C::AddDataTest(double y_val, double x_val) {
//    _plot_widget->AddDataToAllPlots(y_val, x_val);
//}
//
//void JonesPlotApplication_C::SendDatatToPlots() {
//    _plot_widget->AddDataToAllPlots(ui._lineEdit_xVal->text().toFloat(), ui._lineEdit_yVal->text().toFloat());
//}
