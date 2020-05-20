// Visualization lib includes
#ifdef USE_VISUALIZATION_CPP // (VISUALIZATION_ENABLED_CPP == TRUE) 
#define VISUALIZATION_ENABLED TRUE
#include "includes/visualization/jones_plot_app.h"
#include "includes/visualization/ogl_plot_renderer_widget.h"
#else
#endif

// Signal proc lib includes
#include "includes/signal_proc_lib/time_signal.h"
#include "includes/signal_proc_lib/file_io.h"
//#include "includes/signal_proc_lib/time_series_player.h"
#include "time_series_player.h"

// Qt includes
#include <QtWidgets/QApplication>
#include "qopenglwidget.h"

// STL includes
#include <string>
#include <iostream>
#include <thread>
#include <functional>

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

    // The data values of the time series inside the sample file is signed int
    // -> but they are stored as doubles -> <double>
    TimeSignal_C<double> signal;
    signal.ReadG11Data("C://Development//projects//EcgAnalyzer//ecg-analyzer//resources//G11Data.dat");

#ifdef VISUALIZATION_ENABLED
    // temporary solution
    //JonesPlotApplication_C w;
    //w.show();
    QOpenGLPlotRendererWidget* plot_widget = new QOpenGLPlotRendererWidget(2);
    // setup widget
    // ..modify plots..(set axes min/max, tick values,..)
    // show widget
    plot_widget->show();

    double frequency_hz = 1000.0;
    double frequency_ms = (1.0 / frequency_hz) * 1000.0;

    std::cout << ::endl << "Welcome!" << std::endl
     << "select channel to display by entering the channel id : " << std::endl;
    const auto labels = signal.GetChannelLabels();

    int channel_id = 0;
    for ( const auto& channel_label : labels ) {
        std::cout <<"channel id " << channel_id << " = "<< channel_label << std::endl;
        ++channel_id;
    }
    std::cin >> channel_id;

    std::thread dataThread([&]()
    {
        const auto& data = signal.constData();
        const auto& channel_data = data[channel_id]._data;
        const auto& timestamps = data[channel_id]._timestamps;
        // pointer to the data
        auto time_series_begin_it = channel_data.begin();
        auto time_series_timestamps_begin_it = timestamps.begin();

        bool signal_processed = false;
        int64_t time_series_end = data.size();

        while ( !signal_processed  ) { 
            if ( *time_series_begin_it < time_series_end ) {
                //w.AddDataTest(*time_series_begin_it, *time_series_timestamps_begin_it);
                plot_widget->AddDataToAllPlots(*time_series_timestamps_begin_it, *time_series_begin_it);
                ++time_series_begin_it;
                ++time_series_timestamps_begin_it;
            }else{
                signal_processed = true;
            }

           std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frequency_ms)));
       }
   });

   dataThread.detach();

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


    // create a time signal which has double values as x and y values
    //TimeSeriesPlacer_C<double, double> wavedata_generator(signal);
     //obtain the pointer to a member function
    //wavedata_generator.Play(1000.0, 2, PushInterfaceFunction, w);
    //std::this_thread::sleep_for(std::chrono::milliseconds(6000));
    //wavedata_generator.Stop();