#include "jones_plot_app.h"

JonesPlotApplication_C::~JonesPlotApplication_C() {
    delete _list_view_signals;
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
    ui._openGL_widget->SetTreeViewModel(&_plot_model);
    ui._openGL_widget->show();
    // set plot model to table view of the plot settings page
    ui._plot_settings_table_view->setModel(&_plot_model);
    ui._plot_settings_table_view->show();

    // signal model 
    ui._signals_page_main_widget->SetTreeViewModel(&_signal_model);

    // connect from here to the widget which creates the signals => Put these in function for beautyfication?
    connect(ui._signals_page_main_widget, SIGNAL(NewSignal(TimeSignal_C<int>)), this, SLOT(OnNewSignal(TimeSignal_C<int>)) );
    connect(ui._signals_page_main_widget, SIGNAL(NewSignal(TimeSignal_C<float>)), this, SLOT(OnNewSignal(TimeSignal_C<float>)));
    connect(ui._signals_page_main_widget, SIGNAL(NewSignal(TimeSignal_C<double>)), this, SLOT(OnNewSignal(TimeSignal_C<double>)));

    // Set the signal model to the list view dialog to select the signal to play
    _list_view_signals = new ListViewDialog_C();
    _list_view_signals->SetSignalModel(&_signal_model);
    connect(_list_view_signals, SIGNAL(SignalSelected(unsigned int)), this, SLOT(OnNewSignalSelected()) );

    // Signal button start/pause/stop/select button
    connect(ui._btn_plotpage_select_signal, SIGNAL(clicked()), this, SLOT(OnBtnSelectSignal()));
    connect(ui._btn_plotpage_start, SIGNAL(clicked()), this, SLOT(OnBtnPlaySignal()));
    connect(ui._btn_plotpage_pause, SIGNAL(clicked()), this, SLOT(OnBtnPauseSignal()));
    connect(ui._btn_plotpage_stop, SIGNAL(clicked()), this, SLOT(OnBtnStopSignal()));

    // Gain dial
    connect(ui._dial_gain, SIGNAL(sliderMoved(int)), this, SLOT(OnGainChanged(int)));

    connect(ui._signals_page_main_widget, SIGNAL(RemoveSignalRequested(unsigned int)),
        this, SLOT(OnRemoveSignal(unsigned int)));

}

void JonesPlotApplication_C::Setup() 
{
    int number_of_plots = 2;
    
    // Just for the two plots some custom settings for the y ranges
    std::vector<std::pair<ModelDataType_TP, ModelDataType_TP>> y_ranges;
    y_ranges.resize(number_of_plots);
    // first plot
    y_ranges[0].first = -10;   // min y value = -10 mv
    y_ranges[0].second = 10;     // max y value = 10 mV
    // second plot
    y_ranges[1].first = 0; // min val
    y_ranges[1].second = 0.0009; // max val


    bool success = _plot_model.FastInitializePlots(number_of_plots, 
                                                    ui._openGL_widget->width(), 
                                                    ui._openGL_widget->height(),
                                                    10000.0, 
                                                   // /*10*//*0.0005*/0.0009,
                                                   // *-10*/
                                                    y_ranges );

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

void JonesPlotApplication_C::OnBtnSelectSignal()
{
    _list_view_signals->show();    
}

void JonesPlotApplication_C::OnBtnPlaySignal()
{

    if ( _signal_model.Data().empty() || _is_signal_playing.load() ) {
        QMessageBox box;
        box.setText("You have to load a signal or stop the old one before you can Play one");
        return;
    }
    _is_stop_requested.store(false);

    ui._btn_plotpage_pause->setEnabled(true);
    ui._btn_plotpage_stop->setEnabled(true);
    ui._btn_plotpage_start->setEnabled(false);

    // Start a thread which adds the data to the plot(s)
    std::thread dataThread([&]() {

        _is_signal_playing.store(true);

        // assign plot labels just for fun
        auto plot_0 = _plot_model.GetPlotPtr(0);
        //plot_0->SetLabel("plot 0");
        auto plot_1 = _plot_model.GetPlotPtr(1);
         // Change appearance of plots for testing:

        // Adapt so we can see the filtered signal in plot 1
        // MIT-BIH sig: (after MA)=0.0001720.000172
        // !!
        // DONT EXECUTE THESE FUNCTIONS!: THEY ARE NOT DONE => Instead do this in JonesPLotApp::Setup()
        //!
        //plot_1->SetMinValueYAxes(0);
        //plot_1->SetMaxValueYAxes(0.0005);
        //plot_1->SetLabel("plot 1");
        //plot_1->SetTimerangeMs(10000);
        //plot_0->SetTimerangeMs(10000);

        // Load the signal which was selected by the user
        TimeSignal_C<SignalModelDataType_TP>* signal = _signal_model.Data()[_current_signal_id];

         auto& data = signal->constData();
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

        // iterator to the channel-data for plot 0
        auto series_1_begin_it = plot0_data.begin();
        auto timestamps_1_begin_it = plot0_timestamps.begin();
        // iterator to the channel-data for plot 1
        auto series_2_begin_it = plot1_data.begin();
        auto timestamps_2_begin_it = plot1_timestamps.begin();

        // Hide all this pointer stuff in convenience methods so we can use:
        double sample_rate_hz = data[plot0_id]._sample_rate_hz;
        double sample_dist_ms = (1.0 / sample_rate_hz) * 1000.0;
        auto time_series_end = plot0_data.end();

        // Opportunity 1:
        // plot_0->EnableQRSDetection(QRSDetector::PanTokpins);
        // then inside OGLSweepChart::AddDataPoint(..); i can add the datapoint to the qrs detector
        // To prevent to use an if statement, to check if we should use a qrs detector or not, inside AddDataPoint(..),
        // I could write another function AddDataPointWithDetector(..) which assumes that there is the detection going on...
        // ->To be even more convenient, it would be great if there is one AddDatapoint(..) method, 
        // which then internally calls the method which uses qrs detector or the one which does not.

        // Opportunity 2: (Idea: Make Qrs detector independent from the plot -> should be like this!)
        // Then connect the Qrs detector(there is a base class QRS detector(FiducialManager -> see below)) to the plot via EnableQRSDetection()
        //( The base class has methods like GetLatestFiducialMarks();, which can be called from inside the plot class?
        // But all algorithm specific stuff (e.g the adaptive thresholds of pan topkins) will be inside the specialised QRS-detector class)
        //
        // =>This idea is better, because the QRS detector has nothing to do with visualizing the QRS-complexes inside the plot
        // The plot needs a VerticalLineManager or FiducialMarkManager(), which visualizes all these fiducial marks
        // but it should not use the QRS detector for visualizing the Fiducial marks.. 
        // The Fiducial Manager of course needs OpenGL to render the lines inside the Charts drawing surface...

        // e.g
        // // create fiducial manager
        // QRSDetektorPanTopkinsFiducialManager p_t_detector;
        // plot_0->ConnectFiducialManager(detector);
        // 

        // Testing
        PanTopkinsQRSDetection<double> detector(sample_rate_hz, 2);
        // In Qt:
        //connect(detector, PanTopkinsQRSDetection::NewQRSComplexDetected, plot_0, OGLSweepChart_C::AddNewFiducialMark);
        
        // Callback :
        std::function<void(double)> f = std::bind(&OGLSweepChart_C<ModelDataType_TP>::AddNewFiducialMark, plot_0, std::placeholders::_1);
        detector.Connect(f);
        

        auto filt_delay_samples =  detector.GetFilterDelay();
        auto filt_delay_sec = filt_delay_samples / sample_rate_hz;

        bool signal_processed = false;

        while ( !signal_processed && 
                !_is_stop_requested.load() ) 
        {
            if ( series_1_begin_it != time_series_end ) {

                plot_0->AddDatapoint(*series_1_begin_it, *timestamps_1_begin_it);
                // Timestamp are in seconds
                // TODO: CATCH FOUND QRS PEAKS WITH SIGNAL SLOT MECHANISM. INSIDE THE CALLBACK FUNCTION, ADD THEM TO THE FIDUCIAL MARK MANAGER
                double filtered_sig = detector.AppendPoint(*series_1_begin_it, *timestamps_1_begin_it);

                // The timestamps do not match because the filtered signal is delayed ofc..
                plot_1->AddDatapoint(/**series_2_begin_it*/ filtered_sig, *(timestamps_1_begin_it)-filt_delay_sec);

                // EXAMPLE CODE ( Do it not like this) : 
                //fiducial_locations = _qrs_detector.AddDatapoint(*series_1_begin_it, *timestamps_1_begin_it);
                //// 
                //if ( !fiducial_locations.empty() ) {
                //    for (auto& fiducial_mark : fiducial_locations ){
                //        plot0->AddFiducialMark(timestamp, FiducialMark::VerticalLine);
                //    }
                //}
                ++series_1_begin_it;
                ++timestamps_1_begin_it;
                ++series_2_begin_it;
                ++timestamps_2_begin_it;
            }
            else {
                // Todo: clear plots?
                signal_processed = true;
                _is_signal_playing.store(false);
                //ui._btn_plotpage_start->setEnabled(true); => Does not work from ui thread.
                // right now, when the signal stops, you cant start another signal,
                // because the start button is deactivated after we start the thread
                std::cout << "processing finished; thread returns" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sample_dist_ms)));
        }

        _is_signal_playing.store(false);
    });

    dataThread.detach();
}

void JonesPlotApplication_C::OnBtnPauseSignal()
{
    if( _is_signal_playing.load() ){
        ui._btn_plotpage_pause->setEnabled(false);
        ui._btn_plotpage_stop->setEnabled(true);
        ui._btn_plotpage_start->setEnabled(true);

        // ui._btn_plotpage_start->setCheckable(true);
        _is_stop_requested.store(true);
    }

}

void JonesPlotApplication_C::OnBtnStopSignal() 
{
    if ( _is_signal_playing.load() ) {
        ui._btn_plotpage_pause->setEnabled(false);
        ui._btn_plotpage_stop->setEnabled(false);
        ui._btn_plotpage_start->setEnabled(true);
        //lock mutex which stops thread via setting a bool which is checked periodically inside the thread
        _is_stop_requested.store(true);
    }
    // a dirty fix, so it is possible to press the start button again, after the signal was processed completely
    // (and its not frozen forever)
    ui._btn_plotpage_start->setEnabled(true);
    _is_signal_playing = false;
}
void JonesPlotApplication_C::OnGainChanged(int new_gain) 
{
    float scaled_gain = new_gain / ( ui._dial_gain->maximum() / 10 ) ;
    ui._btn_plot_page_gain_view->setText(QString::number(scaled_gain));
    _plot_model.SetGain(scaled_gain);
}

void JonesPlotApplication_C::OnNewSignal(TimeSignal_C<int> signal)
{
    std::cout << "placeholder. Not supported currently" << std::endl;
    //_signal_model.AddSignal(signal);
    // solution 1: three vectors inside the model 
    // (one for doubles, one for floats, one for ints) and iterate through all to draw the plots and support different datatypes
    // 
}

void JonesPlotApplication_C::OnNewSignal(TimeSignal_C<float> signal)
{
    _signal_model.AddSignal(signal);
}

void JonesPlotApplication_C::OnRemoveSignal(unsigned int id) 
{
    _signal_model.RemoveSignal(id);
}
void JonesPlotApplication_C::OnNewSignal(TimeSignal_C<double> signal)
{
    std::cout << "placeholder. Not supported currently" << std::endl;
    //_signal_model.AddSignal(signal);
}

void 
JonesPlotApplication_C::OnNewSignalSelected(unsigned int signal_id) 
{
    _current_signal_id = signal_id;
}
