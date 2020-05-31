#include "jones_plot_app.h"

JonesPlotApplication_C::~JonesPlotApplication_C() {
}

void JonesPlotApplication_C::resizeEvent(QResizeEvent* event)
{
    int width = event->size().width();
    int height = event->size().height();
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
    ui._central_widget->setLayout(ui._grid_layout_general);

    // manage stacked widget pages 
    ui._plot_page_stckd->setLayout(ui._grid_layout_main);
    ui._settings_page_stckd->setLayout(ui._grid_layout_page_settings);
    ui._signals_page_stckd->setLayout(ui._grid_layout_page_signals);

    ui._stacked_widget->addWidget(ui._plot_page_stckd);
    ui._stacked_widget->addWidget(ui._settings_page_stckd);
    ui._stacked_widget->addWidget(ui._signals_page_stckd);

    connect(ui._btn_settings, SIGNAL(clicked()), this, SLOT(OnButtonSettingsPage()));
    connect(ui._btn_home, SIGNAL(clicked()), this, SLOT(OnButtonHomePage()));
    connect(ui._btn_load_signal, SIGNAL(clicked()), this, SLOT(OnButtonSignalsPage()));

    // set plot model to open gl view
    ui._openGL_widget->SetModel(&_plot_model);
    ui._openGL_widget->show();
    // set plot model to table view
    ui._plot_settings_table_view->setModel(&_plot_model);
    ui._plot_settings_table_view->show();

    // Gain dial
    connect(ui._dial_gain, SIGNAL(sliderMoved(int)), this, SLOT(OnGainChanged(int)) );
}

void JonesPlotApplication_C::Setup() 
{
    int number_of_plots = 2;
    bool success = _plot_model.FastInitializePlots(number_of_plots, 
                                                    ui._openGL_widget->width(), 
                                                    ui._openGL_widget->height(),
                                                    1000.0, 
                                                    10,
                                                   -10);

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
        //signal.ReadG11Data("C://Development//projects//EcgAnalyzer//ecg-analyzer//resources//G11Data.dat");
        signal.LoadFromMITFileFormat("C:\\Development\\00ed2097-cd14-4f03-ab33-853da5be5550");

        // assign plot labels just for fun
        auto plot_0 = _plot_model.GetPlotPtr(0);//ui._openGL_widget->GetPlotPtr(0);
        //plot_0->SetLabel("plot 0");
        auto plot_1 = _plot_model.GetPlotPtr(1);//ui._openGL_widget->GetPlotPtr(1);
        //plot_1->SetLabel("plot 1");

        // Get data of all channels
        const auto& data = signal.constData();
        if ( data.empty() ) {
            throw std::runtime_error("Signal is empty!!");
        }
        // data for plot 0
        int plot0_id = 2;//plot_0->GetID() + 2;
        const auto& plot0_data = data[plot0_id]._data;
        const auto& plot0_timestamps = data[plot0_id]._timestamps;
        
        // data to plot 1
        int plot1_id = 4;//plot_1->GetID() + 3;
        const auto& plot1_data = data[plot1_id]._data;
        const auto& plot1_timestamps = data[plot1_id]._timestamps;

        // iterator to the data for plot 0
        auto series_1_begin_it = plot0_data.begin();
        auto timestamps_1_begin_it = plot0_timestamps.begin();
        // iterator to the data for plot 1
        auto series_2_begin_it = plot1_data.begin();
        auto timestamps_2_begin_it = plot1_timestamps.begin();
        
        // Hide all this pointer stuff in convenience methods so we can use:
        double frequency_hz = data[plot0_id]._sample_rate_hz;
        double frequency_ms = (1.0 / frequency_hz) * 1000.0;
        bool signal_processed = false;
        //int64_t time_series_end = plot0_data.size() -1;
        auto time_series_end = plot0_data.end();
        while ( !signal_processed ) {
            if ( series_1_begin_it != time_series_end ) {
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


void JonesPlotApplication_C::OnButtonHomePage()
{
    int _stacked_idx_home = 0;
    ui._stacked_widget->setCurrentIndex(_stacked_idx_home);
    ui._signals_page_main_widget->hide();
}

void JonesPlotApplication_C::OnButtonSettingsPage() 
{
    int _stacked_idx_settings = 1;
    ui._stacked_widget->setCurrentIndex(_stacked_idx_settings);
    ui._signals_page_main_widget->hide();
}

void JonesPlotApplication_C::OnButtonSignalsPage()
{
    int _stacked_idx_settings = 2;
    ui._stacked_widget->setCurrentIndex(_stacked_idx_settings);
    ui._signals_page_main_widget->show();
}

void JonesPlotApplication_C::OnGainChanged(int new_gain) 
{
    float scaled_gain = new_gain / ( ui._dial_gain->maximum() / 10 ) ;
    ui._btn_plot_page_gain_view->setText(QString::fromStdString(std::to_string(scaled_gain)));
    _plot_model.SetGain(scaled_gain);
}
