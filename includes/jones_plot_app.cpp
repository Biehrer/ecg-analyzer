#include "jones_plot_app.h"


JonesPlotApplication_C::~JonesPlotApplication_C() {
    delete _list_view_signals;
}

void JonesPlotApplication_C::resizeEvent(QResizeEvent* event)
{
    int width = event->size().width();
    int height = event->size().height();
    ui._central_widget->resize(width, height);
}

JonesPlotApplication_C::JonesPlotApplication_C(QWidget *parent)
    : QMainWindow(parent)
{
    
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

    // Connect the plot_model to the OpenGl-View.
    // The OpenGLPlotRendererWidget _openGl_widget renders the data inside the plot model
    ui._openGL_widget->SetPlotModel(&_plot_model);
    
    // Connect the plot model to the table view of the plot-settings-page of the stacked widget
    ui._plot_settings_table_view->setModel(&_plot_model);
    ui._plot_settings_table_view->show();

    // Connect signal model with tree ui view model
    ui._signals_page_main_widget->SetTreeViewModel(&_signal_model);

    // Connect to the UI-element which is responsible for the creation of new signals, 
    // to make it possible to add the new signals to the signal model
    connect(ui._signals_page_main_widget, SIGNAL(NewSignal(TimeSignal_C<int>)), this, SLOT(OnNewSignal(TimeSignal_C<int>)) );
    connect(ui._signals_page_main_widget, SIGNAL(NewSignal(TimeSignal_C<float>)), this, SLOT(OnNewSignal(TimeSignal_C<float>)));
    connect(ui._signals_page_main_widget, SIGNAL(NewSignal(TimeSignal_C<double>)), this, SLOT(OnNewSignal(TimeSignal_C<double>)));
    // Make it possible to remove signals, which are selected by the user, inside the signal model
    connect(ui._signals_page_main_widget, SIGNAL(RemoveSignalRequested(unsigned int)),
        this, SLOT(OnRemoveSignal(unsigned int)));

    // Set the signal model to the list view dialog to select the signal to play
    _list_view_signals = new ListViewDialog_C();
    _list_view_signals->SetSignalModel(&_signal_model);
    connect(_list_view_signals, SIGNAL(SignalSelected(unsigned int)), this, SLOT(OnNewSignalSelected(unsigned int)) );

    // Signal button start/pause/stop/select button
    connect(ui._btn_plotpage_select_signal, SIGNAL(clicked()), this, SLOT(OnBtnSelectSignal()));
    connect(ui._btn_plotpage_start, SIGNAL(clicked()), this, SLOT(OnBtnPlaySignal()));
    connect(ui._btn_plotpage_pause, SIGNAL(clicked()), this, SLOT(OnBtnPauseSignal()));
    connect(ui._btn_plotpage_stop, SIGNAL(clicked()), this, SLOT(OnBtnStopSignal()));

    // Gain dial
    connect(ui._dial_gain, SIGNAL(sliderMoved(int)), this, SLOT(OnGainChanged(int)));

    ui._combo_box_writing_speed->addItem("25mm/s", QVariant(25));
    ui._combo_box_writing_speed->addItem("50mm/s", QVariant(50));
    ui._combo_box_writing_speed->addItem("100mm/s", QVariant(100));
    ui._combo_box_writing_speed->addItem("200mm/s", QVariant(200));

    //connect(ui._combo_box_writing_speed, &QComboBox::currentIndexChanged, this, &JonesPlotApplication_C::TestComboBox);
    connect(ui._combo_box_writing_speed, SIGNAL(currentIndexChanged(int)), this, SLOT(TestComboBox(int)));

    // Connect model to plot widget, so modifications in the plot_model are also displayed in the widget
    qRegisterMetaType<OGLPlotProperty_TP>("OGLPlotProperty_TP");
    connect(&_plot_model, SIGNAL(NewChangeRequest(int, const OGLPlotProperty_TP&, const QVariant&)),
            ui._openGL_widget, SLOT(OnNewChangeRequest(int, const OGLPlotProperty_TP&, const QVariant&)));

    //ui._settings_page_btn_add_plot
    //ui._settings_page_btn_remove_plot
    connect(ui._settings_page_btn_setupForSignal, SIGNAL(clicked()), this, SLOT(OnSetupPlotsForSignal()));

    ui._openGL_widget->show();
    // Start drawing
    ui._openGL_widget->StartPaint();
}

void JonesPlotApplication_C::TestComboBox(int curr_idx) 
{
    // TODO Switch: curr idx
    WritingSpeed_TP w_speed;

    switch ( /*ui._combo_box_writing_speed->currentIndex() */curr_idx ) {
    case WritingSpeed_TP::TwentyFive:
        w_speed = WritingSpeed_TP::TwentyFive;
        break;
    case WritingSpeed_TP::Fivty:
        w_speed = WritingSpeed_TP::Fivty;
        break;
    case WritingSpeed_TP::OneHundred:
        w_speed = WritingSpeed_TP::OneHundred;
        break;
    case WritingSpeed_TP::TwoHundred:
        w_speed = WritingSpeed_TP::TwoHundred;
        break;
    }
    ui._openGL_widget->makeCurrent();
    _plot_model.SetTimeRangeWritingSpeed(w_speed);

    // This does also work but is hard to read because the user does not know what exactly is assigned
    //_plot_model.SetTimeRangeWritingSpeed( /*ui._combo_box_writing_speed->currentIndex()*/ );

    //_plot_model.SetTimeRangeWritingSpeed( ui._combo_box_writing_speed->itemData(ui._combo_box_writing_speed->currentIndex()).toDouble );
}

void JonesPlotApplication_C::OnSetupPlotsForSignal(){
    // choose a signal - the first one loaded for now
    if ( _signal_model.Data().empty() ) {
        std::cout << "load a signal first before you can adjust the plots for one" << std::endl;
            return;
    }
    auto& signal = _signal_model.Data()[0];
    auto num_of_plots = signal->GetChannelCount();
    auto timerange_ms = signal->GetTimerangeMs();

    auto& channel_data = signal->constData();
    std::vector<std::pair<ModelDataType_TP, ModelDataType_TP>> y_ranges;
    // Y max and Y min are 5 % bigger / smaller than the biggest / smallest values in the signal
    for ( auto& channel : channel_data ) {
        y_ranges.push_back(std::make_pair(channel._min_val - (channel._min_val * 0.05), 
                           channel._max_val + (channel._max_val * 0.05)));
    }
    // Create plots
    ui._openGL_widget->makeCurrent();
    bool success = _plot_model.InitializePlotsWithOverlap(num_of_plots,
                                                          ui._openGL_widget->width(),
                                                          ui._openGL_widget->height(),
                                                          timerange_ms, 
                                                          y_ranges);

    auto& channel_labels = signal->GetChannelLabels();
    // Setup plots now
    int id = 0;
    for ( auto& plot : _plot_model.Data() ) {
        plot->SetLabel(channel_labels[id]);
        plot->SetID(channel_data.at(0)._id);
        ++id;
    }
}

void JonesPlotApplication_C::Setup() 
{
    Timer_C performance_timer("JonesPlotApplication_C::Setup()");
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


    bool success = _plot_model.InitializePlots/*WithOverlap*/(number_of_plots,
                                                    ui._openGL_widget->width(), 
                                                    ui._openGL_widget->height(),
                                                    10000.0, 
                                                    y_ranges );

    if ( !success ) {
        throw std::runtime_error("plot initialization failed! Abort");
    }  
    
    performance_timer.StopNow();
     // Alternative to fast initialize: 
    // describe the plot using the PlotDescription_TP struct and 
    // add the plot to the plot_widget by calling
    // AddPlot(...)  with the PlotDescription_TP object ::

    // PlotDescription_TP plot0_info;
    // plot0_info._geometry = OGLChartGeometry(pos_x, pos_y, width, height)
    // ...
    // ui._plot_widget.AddPlot(plot0_info);
}


void JonesPlotApplication_C::OnButtonHomePage()
{
    //ui._openGL_widget->StartPaint();
    int _stacked_idx_home = 0;
    //ui._openGL_widget->setAttribute(Qt::WA_WState_ExplicitShowHide, false);
    //ui._openGL_widget->setAttribute(Qt::WA_WState_Hidden, true);
    //ui._openGL_widget->setAttribute(Qt::WA_DontShowOnScreen, false);

    ui._stacked_widget->setCurrentIndex(_stacked_idx_home);
    //ui._signals_page_main_widget->hide();
    //ui._openGL_widget->setAttribute(Qt::WA_DontShowOnScreen, false);
    //ui._openGL_widget->StartPaint();

}

void JonesPlotApplication_C::OnButtonSettingsPage() 
{
    //ui._openGL_widget->StopPaint();
    //ui._openGL_widget->StopPaint();
    int _stacked_idx_settings = 1;
    //ui._openGL_widget->setAttribute(Qt::WA_WState_ExplicitShowHide, true);
    //ui._openGL_widget->setAttribute(Qt::WA_WState_Hidden, false);
    //ui._openGL_widget->setAttribute(Qt::WA_DontShowOnScreen, true);

    ui._stacked_widget->setCurrentIndex(_stacked_idx_settings);
    //ui._openGL_widget->show(); // I bet it does not work because the stacked widget manages hiding process
    //ui._openGL_widget->setAttribute(Qt::WA_DontShowOnScreen, true);
    //ui._openGL_widget->StartPaint();

    //setAttribute(Qt::WA_DontShowOnScreen, true);

    //ui._signals_page_main_widget->hide();

    //auto temp_copy = _plot_model.Data()[0];
    //_plot_model.RemovePlot(0);
    
    //_plot_model.AddPlot(*temp_copy);

    //_plot_model.RemovePlot(0);
    //auto plot_0 = _plot_model.GetPlotPtr(0);
    //plot_0->SetMaxValueYAxes(20.0);
}

void JonesPlotApplication_C::OnButtonSignalsPage()
{
    int _stacked_idx_settings = 2;
    ui._stacked_widget->setCurrentIndex(_stacked_idx_settings);
    //ui._signals_page_main_widget->show();
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
        // Adapt so we can see the filtered signal in plot 1
        // MIT-BIH sig: (after MA)=0.0001720.000172
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
        int plot1_id = 3;//plot_1->GetID() + 3;
        const auto& plot1_data = data[plot1_id]._data;
        const auto& plot1_timestamps = data[plot1_id]._timestamps;

        // iterator to the channel-data for plot 0
        auto series_1_begin_it = plot0_data.begin();
        auto timestamps_1_begin_it = plot0_timestamps.begin();
        auto time_series_end = plot0_data.end();
        // iterator to the channel-data for plot 1
        auto series_2_begin_it = plot1_data.begin();
        auto timestamps_2_begin_it = plot1_timestamps.begin();

        // Hide all this pointer stuff in convenience methods so we can use:
        double sample_rate_hz = data[plot0_id]._sample_rate_hz;
        double sample_dist_ms = (1.0 / sample_rate_hz) * 1000.0;
        
        // Testing Detector 1 - plot 0
        PanTopkinsQRSDetection<double> detector_0(sample_rate_hz, 2);
        // Callback :
        std::function<void(const Timestamp_TP&)> member_callback = std::bind(&OGLSweepChart_C<ModelDataType_TP>::AddNewFiducialMark, 
                                                                             plot_0, 
                                                                             std::placeholders::_1);
        detector_0.Connect(member_callback);

        // Testing Detector 2 - plot 1
        PanTopkinsQRSDetection<double> detector_1(sample_rate_hz, 2);
        // Callback :
        std::function<void(const Timestamp_TP&)> member_callback_1 = std::bind(&OGLSweepChart_C<ModelDataType_TP>::AddNewFiducialMark,
            plot_1,
            std::placeholders::_1);
        detector_1.Connect(member_callback_1);

        // TODO: Also respect the moving average delay
        auto filt_delay_samples =  detector_0.GetFilterDelay(); 
        auto filt_delay_sec = filt_delay_samples / sample_rate_hz;

        // True, when signal visualization is finished
        bool signal_processed = false;

        while ( !signal_processed && 
                !_is_stop_requested.load() ) 
        {
            if ( series_1_begin_it != time_series_end ) {
                // AddDatapoint(..) is the only thread safe method of OGLSweepChart_C!
                plot_0->AddDatapoint(*series_1_begin_it, *timestamps_1_begin_it);
                //detector_0.AppendPoint(*series_1_begin_it, *timestamps_1_begin_it);

                plot_1->AddDatapoint(*series_2_begin_it, *timestamps_2_begin_it);
                //detector_1.AppendPoint(*series_2_begin_it, *timestamps_2_begin_it);

                // Prototyping
                /*double filtered_sig = detector_0.AppendPoint(*series_1_begin_it, *timestamps_1_begin_it);*/
                // The timestamps do not match because the filtered signal is delayed ofc and therefore need to be shifted
                //plot_1->AddDatapoint(filtered_sig, *(timestamps_1_begin_it)-filt_delay_sec);

                ++series_1_begin_it;
                ++timestamps_1_begin_it;
                ++series_2_begin_it;
                ++timestamps_2_begin_it;
            } else {
                signal_processed = true;
                _is_signal_playing.store(false);
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
    _plot_model.ClearPlotSurfaces();
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
}

void JonesPlotApplication_C::OnNewSignal(TimeSignal_C<double> signal)
{
    std::cout << "placeholder. Not supported currently" << std::endl;
    //_signal_model.AddSignal(signal);
}

void JonesPlotApplication_C::OnNewSignal(TimeSignal_C<float> signal)
{
    _signal_model.AddSignal(signal);
}

void JonesPlotApplication_C::OnRemoveSignal(unsigned int id) 
{
    _signal_model.RemoveSignal(id);
}

void 
JonesPlotApplication_C::OnNewSignalSelected(unsigned int signal_id) 
{
    _current_signal_id = signal_id;
}
