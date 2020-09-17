#pragma once

// Projects includes
#include "rt_state_filters.h"

// KFR includes
#include "kfr/base.hpp"
#include "kfr/dsp.hpp"
#include "kfr/io.hpp"

// STL includes
#include <iostream>
#include <functional>

template<typename DataType_TP>
class PanTopkinsQRSDetection {

    // Constructor / Destrcutor/...
public:
    PanTopkinsQRSDetection(double sample_freq_hz, unsigned int training_hase_duration_sec);
    ~PanTopkinsQRSDetection();

    // Public functions
public:
    //! Real time implementation of the pan topkings qrs detection method.
    //!
    //! \param data the current sample of the datastream which is analyzed
    //! \param timestamp the timestamp of the current sample inside the datastream
    /*void*/DataType_TP AppendPoint(const DataType_TP data, double timestamp_sec);

    //! Returns the delay due to the filtering in number of samples
    int GetFilterDelay();

    //! Resets the filter state ( Removes all values in the delay_line)
    void Reset(float sample_freq_hz, unsigned int training_phase_duration_sec);

    //! Stores the callback, which is called, when a qrs complex was detected
    void Connect(std::function<void(double)> callback);

    // Private functions
public:

    // Initializes the thresholds - learning phase 1
    void InitializeThresholds(const std::vector<DataType_TP>& training_data);

    // Private variables
private:
    // Pan-Topkins adaptive Threshold parameter
    // Calculated from the signal level
    double _signal_threshold = 0.0;

    // Pan-Topkins adaptive Threshold parameter
    // Calculated from the noise level
    double _noise_threshold = 0.0;

    // Is updated each time, a valid QRS complex was found. Higher thresdhold
    double _signal_level = 0.0;

    // Is updated each time, when noise was found. Lower threshold
    double _noise_level = 0.0;

    // Pan Topkins Parameters for the algorithm
    //! 200 ms after a QRS complex was detected, its physiological not possible that another qrs complex appears.
    //! This parameter reduces false positives in the detection.
    const double _refractory_period_ms = 200.0;

    //! The time after a QRS complex occured, when we expect the T-Wave 
    //! T-Waves can only be found after refractory_period passed but before the t_wave_period passed
    const double _t_wave_period_ms = 360.0;

    // The window length for the moving-average-integration in milliseconds.
    // A QRS normally has a max width of 150 milliseconds.
    const unsigned int _window_length_ms = 150;

    //! Duration  which will be used for the initialization of the thresholds in seconds
    unsigned int _training_phase_duration_s = 2;

    //! Filter taps / order
    const unsigned int _filter_order = 63;

    //! Cutoff frequency of the lowpass filter 
    //! (all frequencies below this one will be left in the signal after filtering)
    const unsigned int _low_cutoff_freq_hz = 11;

    //! Cutoff frquency of the highpass filter
    //! (all frequencies above this one will be left in the signal after filtering)
    const unsigned int _high_cutoff_freq_hz = 5;

    //! Sample frequency of the signal on which the qrs detection is done.
    double _sample_freq_hz = 0.0;

    //! the length of the MA-window in samples
    unsigned int _window_length_samples = 0;

    //! The number of training samples
    unsigned int _number_of_training_samples = 0;

    //! Delay of the input signal due to the filtering in samples
    unsigned int _filter_delay_samples = 0;

    //! True if the training phase is completed and thresholds are initialized
    bool _thresholds_initialized = false;

    //! Buffer for the training phase of the algorithm
    std::vector<DataType_TP> _training_buffer;
    unsigned int _training_data_idx = 0;

    //! Bandpass filter
    kfr::filter_fir<kfr::fbase, double>* _bandpass_filter;

    //! This value is initialized with _t_wave_period_ms, when a qrs complex was detected and decremented gradually, to detect
    //! if a Peak appears right after another peak. If a peak occurs _t_wave_period_ms ms after another peak, 
    //! but it is not in the refractory period, then it is most likely a T-Wave
    double _t_wave_counter = 0;
    
    //! Initialized with _refractory_period_ms, when a qrs complex was detected, 
    //! to tell if another peak, which appears right after another a QRS-complex, 
    //! is in the refractory period(and therefore can't be a qrs complex)
    double _refractory_period_counter = 0;
    
    //! Stores the timestamp of the last peak detected
    double _last_peak_timestamp = 0.0;
    
    //! Time since the last peak was detected
    double _time_since_last_peak_sec = 0.0;

    //! The number of detected qrs complex, since Reset() was called, or this class was initialized
    unsigned int _qrs_counter = 0;

    //! Amplitude value of the current peak
    DataType_TP _peak_amplitude = 0;
    //! Timestamp of the current peak
    double _peak_timestamp = 0;

    kfr::univector<DataType_TP> _input_buff;

    //! Moving average filter
    MovingAverageStateFilter<DataType_TP> _ma_filter;

    //! Differentiation filter
    DerivationStateFilter<DataType_TP> _diff_filter;

    //! Filter to detect peaks in the data-stream
    PeakDetectorFilter<DataType_TP> _peak_filter;

    //! The function called, when a qrs complex is detected
    std::function<void(double)> _qrs_callback;
};


template<typename DataType_TP>
PanTopkinsQRSDetection<DataType_TP>::PanTopkinsQRSDetection(double sample_freq_hz,
    unsigned int training_phase_duration_sec)
    : _peak_filter(sample_freq_hz * 3) // Make a buffer which can store up to 3 secs of signal history(Which seems bit too small (we need to lookup last eight R-R intervalls))
{
    _sample_freq_hz = sample_freq_hz;
    _training_phase_duration_s = training_phase_duration_sec;
    // Create Buffers for Filtering (purpose is to keep a signal history)
    _window_length_samples = (static_cast<double>(_window_length_ms) / 1000.0) * _sample_freq_hz;

    _number_of_training_samples = training_phase_duration_sec * _sample_freq_hz;
    _training_buffer.resize(_number_of_training_samples);

    _input_buff.resize(1);

    // Create the bandpass filter
    kfr::univector<kfr::fbase, 63> taps63;
    kfr::expression_pointer<kfr::fbase> window = kfr::to_pointer(kfr::window_kaiser(taps63.size(), 3.0)); // 3.0=Filter gain
    // Fill taps63 with the band pass FIR filter coefficients using kaiser window and cutoff=y.x and u.t Hz
    kfr::fir_bandpass(taps63, _high_cutoff_freq_hz, _low_cutoff_freq_hz, window, true);
    _bandpass_filter = new kfr::filter_fir<kfr::fbase, double>(taps63);
    // Calculate filter delay
    // times 2, because the lowpass and highpass filtering each introduce a delay, which is half the filter order.
    _filter_delay_samples = (_filter_order / 2) * 2;

    // Configure MA-Filter 
    _ma_filter.SetParams(_window_length_samples);
    //_filter_delay_samples += _ma_filter.GetFilterDelay();
}

template<typename DataType_TP>
PanTopkinsQRSDetection<DataType_TP>::~PanTopkinsQRSDetection()
{
    delete _bandpass_filter;
}


template<typename DataType_TP>
//void
DataType_TP // TODO: Pass sample by refernce
PanTopkinsQRSDetection<DataType_TP>::AppendPoint(const DataType_TP sample, double timestamp)
{
    // For testing only: TODO: Function should return void again and not the filtered signal sample by sample! ( Or it should return a bool(true), if its a peak
    _input_buff[0] = sample;
    // Use bandpass as state filter: filter each sample by sample
    _bandpass_filter->apply(_input_buff);
    // derivation state filter
    _diff_filter.Apply(_input_buff[0]);
    // square derivated sig
    _input_buff[0] = _input_buff[0] * _input_buff[0];
    // moving average state filter 
    _ma_filter.Apply(_input_buff[0]);

    // Training phase 1 (if not already done) => Wrap this into a function,
    // outside of the Apply() function so we dont need to call if() each time?
    // => but for safe use of the class, we should always check if thresholds are initialized eitherway?
    if ( !_thresholds_initialized ) {
        if ( _training_data_idx < _number_of_training_samples ) {
            // Collect more data and then initialize the threshold when we got enough..
            _training_buffer[_training_data_idx] = _input_buff[0];
            // thresholds are not initialized yet
        } else {
            InitializeThresholds(_training_buffer);
            _thresholds_initialized = true;
        }
        ++_training_data_idx;
    }

    // Thresholds are initialized; Now we can start QRS Detection 
    // Peak detection (=> detect peaks only when not in refractory period)
    bool is_peak = _peak_filter.Apply(_input_buff[0]);

    bool is_qrs = false;
    // bool is_t_wave = false;

    // Performance tips:  check right here, if we are still in refractory period. If this is the case, 
    // we do not need to do all stuff below
    if ( is_peak ) {

        _time_since_last_peak_sec = timestamp - _last_peak_timestamp; 
        _refractory_period_counter -= _time_since_last_peak_sec;
        _t_wave_counter -= _time_since_last_peak_sec;
        _last_peak_timestamp = timestamp;

        // Now use criterias (T-period, inhibitation time,..) to remove false positive peaks and only return valid peak locations
        
        // Get amplitude of the peak (THIS IS NOT THE CURRENT SAMPLE VALUE, but the value before !-> this is how real time peak detection works)
        if ( _peak_amplitude > _signal_threshold ) {
            // Found a potential QRS peak

            // first check if it could also be a t - wave
            if ( _t_wave_counter > 0 && 
                _refractory_period_counter <= 0 )
            {
                //is_t_wave = true;
                _t_wave_counter = 0;
            } // It's not a t-wave, check for qrs:
            else if ( _refractory_period_counter <= 0 ) {
                is_qrs = true;
                // Call callback to notify the listener the detected qrs location
                _qrs_callback(_peak_timestamp-(_filter_delay_samples/_sample_freq_hz) );

                // update signal level
                _signal_level = 0.125 * _peak_amplitude + 0.875 * _signal_level;
                ++_qrs_counter;
                // Start refactory period, because a QRS complex was detected
                _refractory_period_counter = _refractory_period_ms / 1000.0;
                // Start t - wave counter, because they appear right after QRS complexes
                _t_wave_counter = _t_wave_period_ms / 1000.0;
            }

        }
        else if ( _peak_amplitude > _noise_threshold &&
            _peak_amplitude < _signal_threshold )
        {
            // Found noise
            // update noise level
            _noise_level = 0.125 * _peak_amplitude + 0.875 * _noise_level;
        }

        // Update signal and noise thresholds
        // first set of thresholds
        _signal_threshold = _signal_level + 0.25 * (_signal_level - _noise_level);
        _noise_threshold = 0.5 * _signal_threshold;

        // Todo: second set of thresholds for qrs search back
        //adapt_thresholds(peak_idx).Threshold21 = 0.0; % The equation is found on the paper
        //adapt_thresholds(peak_idx).Threshold22 = 0.5 * adapt_thresholds(peak_idx).Threshold22;
    }

    // The current amplitude is the next peak which can be detected
    // TODO The timestamp of the detected peak is  actually _timestamp_last_sample ( does not exist yet )
    // and not the timestamp of the current sample
    // => calculate _timestamp_last_sample with the sample frequency: _timestamp_last_sample = timestamp_current - _sample_dist_sec
    _peak_amplitude = _input_buff[0];
    _peak_timestamp = timestamp;
    // Just for prototyping: return the filtered signal
    return _input_buff[0];
}

template<typename DataType_TP>
inline
void PanTopkinsQRSDetection<DataType_TP>::Reset(float sample_freq_hz, unsigned int training_phase_duration_sec)
{
    _t_wave_counter = 0;
    _refractory_period_counter = 0;
    _qrs_counter = 0;

    _thresholds_initialized = false;
    _training_data_idx = 0;

    _sample_freq_hz = sample_freq_hz;
    _training_phase_duration_s = training_phase_duration_sec;
    _window_length_samples = (static_cast<double>(_window_length_ms) / 1000.0) * _sample_freq_hz;
    // Create Buffers to keep data for training phase 1 (Threshold initialization)
    _number_of_training_samples = training_phase_duration_sec * _sample_freq_hz;
    _training_buffer.resize(_number_of_training_samples);
}

template<typename DataType_TP>
inline 
void 
PanTopkinsQRSDetection<DataType_TP>::Connect(std::function<void(double)> qrs_callback)
{
    _qrs_callback = qrs_callback;
}


template<typename DataType_TP>
inline
int
PanTopkinsQRSDetection<DataType_TP>::GetFilterDelay() {
    return _filter_delay_samples;
}

// \param training_data needs to be the filtered (MA-integrated) signal
template<typename DataType_TP>
void
PanTopkinsQRSDetection<DataType_TP>::InitializeThresholds(const std::vector<DataType_TP>& training_data)
{
    // Signal threshold
    // 0.25 of the max amplitude is signal thresh
    _signal_threshold = *std::max_element(training_data.begin(), training_data.end()) * 0.25;
        
    // Noise threshold
    // Calculate signal mean
    DataType_TP signal_sum = 0;
    for ( const auto& sample : training_data ) {
        signal_sum += sample;
    }
    DataType_TP signal_mean = signal_sum / training_data.size();
    // 0.5 of the mean signal is considered to be the noise threhold
    _noise_threshold = signal_mean * 1 / 2;
}

