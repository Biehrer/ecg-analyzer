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

    // Do this ogl stuff before the line: QApplication a(argc, arv)
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QApplication a(argc, argv);

    // Create a time signal and fill it with data
    TimeSignal_C<double> signal;
    signal.ReadG11Data("C://Development//projects//EcgAnalyzer//ecg-analyzer//resources//G11Data.dat");
    

    #ifdef VISUALIZATION_ENABLED
    // Create the visualization widget
    QOpenGLPlotRendererWidget* plot_widget = new QOpenGLPlotRendererWidget();
    plot_widget->show();

    // Fast initialization of plots
    // in a vertical layout
    int number_of_plots = 2;
    bool success =  plot_widget->FastInitializePlots(number_of_plots, 10000.0, 3, -3);
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
    // plot_widget->AddPlot(plot0_info);

    //int channel_id = ReadChannelFromUser<double>(signal);

    // Start a thread which adds the data to the plot(s)
    std::thread dataThread( [&] () {
        // assign plot labels just for fun
        auto plot_0 = plot_widget->GetPlotPtr(0);
        plot_0->SetLabel("plot 0");
        auto plot_1 = plot_widget->GetPlotPtr(1);
        plot_1->SetLabel("plot 1");

        // Get data of all channels
        const auto& data = signal.constData();

        // data for plot 0
        int plot0_id = plot_0->GetID();
        const auto& plot0_data = data[plot0_id]._data;
        const auto& plot0_timestamps = data[plot0_id]._timestamps;
        // data to plot 1
        int plot1_id = plot_1->GetID();
        const auto& plot1_data = data[plot1_id]._data;
        const auto& plot1_timestamps = data[plot1_id]._timestamps;

        // iterator to the data for plot 0
        auto series_1_begin_it = plot0_data.begin();
        auto timestamps_1_begin_it = plot0_timestamps.begin();
        // iterator to the data for plot 1
        auto series_2_begin_it = plot1_data.begin();
        auto timestamps_2_begin_it = plot1_timestamps.begin();

        double frequency_hz = 1000.0;
        double frequency_ms = (1.0 / frequency_hz) * 1000.0;
        bool signal_processed = false;
        int64_t time_series_end = plot0_data.size();

        while ( !signal_processed  ) { 
            if ( *series_1_begin_it < time_series_end ) {
                plot_0->AddDatapoint(*series_1_begin_it, *timestamps_1_begin_it);
                plot_1->AddDatapoint(*series_2_begin_it, *timestamps_2_begin_it);

                ++series_1_begin_it;
                ++timestamps_1_begin_it;
                ++series_2_begin_it;
                ++timestamps_2_begin_it;
            } else {
                signal_processed = true;
                std::cout << "processing finished; thread returns" << std::endl;
            }

           std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frequency_ms)));
       }
   });
   dataThread.detach();
   #endif

	return a.exec();
}

template<typename DataType_TP>
int ReadChannelFromUser(const TimeSignal_C<DataType_TP>& signal) 
{
    std::cout << std::endl << "Welcome!" << std::endl
        << "select channel to display by entering the channel id : " << std::endl;

    // Ask the user to which channel from the signal should be visualized
    int channel_id = 0;
    for ( const auto& channel_label : signal.GetChannelLabels() ) {
        std::cout <<"channel id " << channel_id << " = "<< channel_label << std::endl;
        ++channel_id;
    }
    std::cin >> channel_id;
    std::cout << "start playing of data series from channel # :" << channel_id << std::endl;

}


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

//TimeSeriesPlacer_C<double, double> wavedata_generator(signal);
//wavedata_generator.Play(1000.0, 2, PushInterfaceFunction, w);
//std::this_thread::sleep_for(std::chrono::milliseconds(6000));
//wavedata_generator.Stop();  