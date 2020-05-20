#pragma once
// Project includes
//#include ""

// Qt includes
#include <qobject.h>

// STL includes
#include <mutex>

template<typename YValDataType_TP, typename XValDataType_TP >
struct SeriesDatapoint_TP {

    SeriesDatapoint_TP(YValDataType_TP y_val, XValDataType_TP x_val) 
        : _y_val(y_val),
          _x_val(xval)
    {
    }

    YValDataType_TP _y_val;
    XValDataType_TP _x_val;
};

template<typename YValDataType_TP, typename XValDataType_TP >
class TimeSeriesPlacer_C {

public:
    TimeSeriesPlacer_C(const TimeSignal_C<YValDataType_TP>& time_series);

    typedef void OutputFunction(YValDataType_TP, XValDataType_TP, JonesPlotApplication_C& visualization_app);
    //typedef std::visualization_push_func<void(YValDataType_TP, XValDataType_TP)> OutputFunction;
    //typedef void OutputFunction(JonesPlotApplication_C::*AddDataTest)(double, double);

                                      //! this visualization_push_func starts a thread which calls the visualization_push_func 
     //! inside the argument periodically with the input frequency. 
     //! each time the visualization_push_func is called, two datavalues are passed to the OutputFunction 
     //! (y and x value of  TimeSignal_C's data; )
     //! When data from a specific TimeSignal_C is played, 
     //! its not safe to acces the data of this TimeSignal_C object!
    void Play(double frequency_hz, int channel_id, OutputFunction function, JonesPlotApplication_C& visualization_app);
    
    //! Stops playing the time signal.
    //! If Play() is called, after the signal was stopped, 
    //! the signal again starts playing from the beginning
    void Stop();

private:

    //! Const reference to the time series which is played
    const TimeSignal_C<YValDataType_TP>& _time_series;

    bool _stop_playing_series = false;

    std::mutex _stop_play_lock;
};

//signals:
    //    // Needs to be registere qt meta bs
    //    SeriesDatapoint_TP<YValDataType_TP, XValDataType_TP> OnNewDataValue();
    //
template<typename YValDataType_TP, typename XValDataType_TP>
inline
TimeSeriesPlacer_C<YValDataType_TP, 
    XValDataType_TP>::TimeSeriesPlacer_C(const TimeSignal_C<YValDataType_TP>& time_series)
    : _time_series(time_series)
{

}


template<typename YValDataType_TP, typename XValDataType_TP>
inline 
void 
TimeSeriesPlacer_C<YValDataType_TP, XValDataType_TP>::Play(double frequency_hz, 
                                                           int channel_id,
                                                           OutputFunction visualization_push_func,
                                                           JonesPlotApplication_C& visualization_app)
{
    double frequency_ms = (1.0 / frequency_hz) * 1000.0;
    
    // How to use thread.detach() so we can stop the thread ? - for testing it would not be necessary to stop...
    // or check why it throws errors on  .detach
    // Todo:
    // - pass plot widget funtion inside the Play() visualization_push_func -> but then we have a dependency to the visualization...very bad

    std::thread dataThread( [&] ()
    {
        int64_t time_series_end = _time_series.constData().size();
        const auto& data = _time_series.constData();

        const auto& channel_data = data[channel_id]._data;
        const auto& timestamps = data[channel_id]._timestamps;

        auto time_series_begin_it = channel_data.begin();
        auto time_series_timestamps_begin_it = timestamps.begin();

        bool local_copy_stop_flag = false;

        while ( !local_copy_stop_flag ) {
            // check if the thread should return
            _stop_play_lock.lock();
            local_copy_stop_flag = _stop_playing_series;
            _stop_play_lock.unlock();

            if( !local_copy_stop_flag && 
                *time_series_begin_it < time_series_end ) 
            {
                visualization_push_func(*time_series_begin_it, *time_series_timestamps_begin_it, visualization_app);
                ++time_series_begin_it;
                ++time_series_timestamps_begin_it;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frequency_ms)));
        }

    });

    dataThread.join();
}

template<typename YValDataType_TP, typename XValDataType_TP>
inline
void
TimeSeriesPlacer_C<YValDataType_TP, XValDataType_TP>::Stop()
{
    // aquire lock, set the bool and let the thread shutdown
    _stop_play_lock.lock();
    _stop_playing_series = true;
    _stop_play_lock.unlock();
}
