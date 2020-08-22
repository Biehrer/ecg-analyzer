#pragma once

// KFR includes
#include "kfr/base.hpp"
#include "kfr/dsp.hpp"
#include "kfr/io.hpp"
// STL includes
#include <iostream>

template<typename DataType_TP>
class PanTopkinsQRSDetection {

    // Constructor / Destrcutor/...
public:
    PanTopkinsQRSDetection();
    ~PanTopkinsQRSDetection();

    // Public functions
public:
    
    // alt 1:
 /*   void AppendPoint(const DataType_TP data);*/

    // alt2:
    void AppendPoint(const DataType_TP data, double timestamp/*, Timestamp_TP timestamp*/);
    const std::vector<DataType_TP> GetCurrenRPeaks();
    //const std::vector<DataType_TP> GetCurrenRPeakTimestamps();

    void ResetState();

    // Alternative:
   // const std::vector<DataType_TP> DetectRPeaks(const std::vector<DataType_TP>& ecg_signal, bool use_learning_phase);
//private:
    // Initializes the thresholds
    void InitializeThresholds(const std::vector<DataType_TP>& training_data);

    // Applys filtering on a window
    //const std::vector<DataType_TP> ApplyBandpassFilter(const std::vector<DataType_TP>& signal, 
    //                                                   double low_cut_freq_hz, 
    //                                                   double high_cut_freq_hz);


    // 
    //MovingAverageWindowIntegration(const std::vector<DataType_TP>& signal, double window_length_ms);

//private:
public:
    void Initialize(float sample_freq_hz, unsigned int training_phase_duration_sec);

    // Example usage:
    // option 1: AS STATIC CLASS -> not good because of Parameters which has to be transmitted each call...
    //PanTopkinsQRSDetection::Detect(&Signal, 5,11, 200, 360,..., OutputVectorForDetectedLocations)

    // option 2:
    // PanTopkinsQRSDetection qrs_detector;
    // qrs_detector.SetParams(...);
    // possibility: The detector starts a thread (with .Detect), which begins pushing these detected locations onto 
    // the OutputVectorForDetectedLocations vector?

    // Somehow the detector needs to get the signal values -> therefore it needs to be part/block of the real-time-filtering 'CHAIN'
    // Before we prepare the data for visualization, we need to detect the qrs complexes in the data

    // qrs_detector.Detect(Signal, OutputVectorForDetectedLocations)

    // So the Fiducial Manager does not know anything from the QRS detector and it only is responsible for plotting the Fiducial Marks

    // The chain would be:
    // =>> Data samples come in(as record from wfdb lib inside a thread which pushes one sample after the other )
    // ==> use QRS Detector inside jones_plot_app.h OnStartButton()-Method to detect locations where QRS complexes are inside the real time stream 
    // ==> Push locations of the QRS complexes to the specific plot (->The plot then inside uses the Fiducial Manager to draw the QRS-FiducialMarks) 
   
    // Option 1: The QRS Detector has a input buffer and does produce a output (QRS location or not),
    // when a specific amount of samples are inside this buffer

    // Option 2: The QRS Detector only has the possobilities to process 'windows' of 
    // samples and then tell if there is a QRS complex inside this window => 150 milliseconds? 
    // ( but this means the implementation does also need a buffer of the last sample history for the moving window integration 

    // The peak detection can be done in smaller windows theoretically 
    // (even after each sample(Then we only need a buffer of the size of one sample for peak detection),
    // But can the filtering be done in smaller windows? I think the windows can be smaller than the window of the moving-window integration
    // So this means the moving-window integration is the Bottleneck of this whole algorithm. 
    //  Additionally it cant be done like in Matlab with the 'Center' option
    // because we do not know the values in the future beforehand. We need just the Left side (Matlab 'Left' option),
    // which are the samples from the past, which we do know).
    // This means we need a buffer of at least the size of the moving-window integration, which we can use for moving window integration?
    // Does this also mean i would do an window integration after each new sample which is added?

    // ODER option3 ( Call .Detect on PARTS of the signal -> Collect windows of 150ms length and process them??) 
    // This means each detection qrs complex has at least a delay of 75 or 150 milliseconds => Test, is this is sufficient.
    // PanTopkinsQRSDetection qrs_detector;
    // qrs_detector.SetParams(...);

    // qrs_detector.DetectPekaks(Signal_PART, OutputVectorForDetectedLocations)

    // Private functions
private:
    // Returns the indices of the peaks inside the signal
    std::vector<unsigned int> FindPeaks(const std::vector<DataType_TP>& data);
    
    // Creates the derivation inplace
    void DerivationFilter(/*std::vector<DataType_TP>& data*/kfr::univector<DataType_TP>& data);

    kfr::univector<DataType_TP> MovingWindowAveraging(const kfr::univector<DataType_TP>& data);

    // Private variables
private:
    // Pan-Topkins adaptive Threshold parameter
    // Calculated from the signal level
    double _signal_threshold = 0.0;

    // Pan-Topkins adaptive Threshold parameter
    // Calculated from the noise level
    double _noise_threshold = 0.0;

    // Is updated each time, a valid QRS complex was found
    double _signal_level = 0.0;

    // Is updated each time, when noise was found
    double _noise_level = 0.0;

    // Pan Topkins Parameters for the algorithm
    //! 200 ms after a QRS complex was detected, its physiological not possible that another qrs complex appears.
    //! This parameter reduces false positives in the detection.
    const double _refractory_period_ms = 200.0;
    
    //! The time after a QRS complex, when we expect the T-Wave 
    //! T-Waves can only be found after the passing of the refractory_period but before the t_wave_period passed
    const double _t_wave_period_ms = 360.0;
    
    // The window length for the moving-average-integration in milliseconds.
    // A QRS normally has a max width of 150 milliseconds.
    const unsigned int _window_length_ms = 150;

    //! Duration  which will be used for the initialization of the thresholds in seconds
    unsigned int _training_phase_duration_s = 2;

    // Filter parameters
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
    
    //! Buffer for signal history
    kfr::univector<DataType_TP> _buffer_current;
    //std::vector<DataType_TP> _buffer_current;

    unsigned int _buffer_head_idx = 0;

    //! Buffer for last window
    kfr::univector<DataType_TP> _buffer_old;
    //std::vector<DataType_TP> _buffer_old;

    //! Buffer for the training phase of the algorithm
    std::vector<DataType_TP> _training_buffer;
    unsigned int _training_data_idx = 0;

    //! Bandpass filter
    kfr::filter_fir<kfr::fbase, double>* _bandpass_filter;
};


template<typename DataType_TP>
PanTopkinsQRSDetection<DataType_TP>::PanTopkinsQRSDetection() 
{
    kfr::univector<kfr::fbase, 63> taps63;
    kfr::expression_pointer<kfr::fbase> window = kfr::to_pointer(kfr::window_kaiser(taps63.size(), 3.0)); // 3.0=Filter gain
    // Fill taps127 with the band pass FIR filter coefficients using kaiser window and cutoff=0.2 and 0.4 Hz
    kfr::fir_bandpass(taps63, _high_cutoff_freq_hz, _low_cutoff_freq_hz, window, true);
    _bandpass_filter = new kfr::filter_fir<kfr::fbase, double>(taps63);

    // times 2, because the lowpass and highpass filtering each introduce a delay, which is half the filter order.
    _filter_delay_samples = (_filter_order / 2) * 2;
}

template<typename DataType_TP>
PanTopkinsQRSDetection<DataType_TP>::~PanTopkinsQRSDetection()
{
    delete _bandpass_filter;
}

//template<typename DataType_TP>
//inline 
//const 
//std::vector<DataType_TP> 
//PanTopkinsQRSDetection<DataType_TP>::DetectRPeaks(const std::vector<DataType_TP>& ecg_signal, bool use_learning_phase)
//{
//    /* Implement algorithm */
//    return std::vector<DataType_TP>();
//}

template<typename DataType_TP>
void
PanTopkinsQRSDetection<DataType_TP>::AppendPoint(const DataType_TP sample, double timestamp)
{
    if ( /*_buffer.size*/_buffer_head_idx < _window_length_samples ) {
        // fill buffer, because there is still room left
        // Option1: Use the ring buffer -> better, but its ok to do this later
        // Option2: Just fill the buffer here:
        _buffer_current[_buffer_head_idx] = sample;
        ++_buffer_head_idx;
    } else {
        // Buffer filled - now it would be possible to do the detection with the collected window AND:
        // Tell the user with a bool return value, that he can check the state

        // Reset latest history buffer idx? 
        _buffer_head_idx = 0;
        // Keep signal history
        _buffer_old = _buffer_current;

        // FILTER THE WINDOW? -> still not quite sure if its good to process 'windows'...
        // => For each window we need the last sample of the last window as first value in the current window?
        // Bandpass, derivation, squaring, moving window averaging
        // Bandpass filtering in place

        // Now the univector does not own the data but only references it
        //kfr::univector<double, 0> ptrToStdVec(&_buffer_current, _window_length_samples);
        //_bandpass_filter->apply(ptrToStdVec);
        // Alternative:
        _bandpass_filter->apply(_buffer_current);
        
        // Create derivation in place
        DerivationFilter( _buffer_current );

        // Squaring
        std::for_each(_buffer_current.begin(), _buffer_current.end(),
            [&](auto& sample)
        {
            sample = sample * sample;
        });
        
        // MA integration
       _buffer_current = MovingWindowAveraging(_buffer_current);


        // Training phase 1 (if not already done) => Wrap this into a function
        if ( !_thresholds_initialized ) {
            if( _training_data_idx < _number_of_training_samples-_window_length_samples){
                // Collect more data and then initialize the threshold when we got enough..
                _training_buffer.insert(_training_buffer.begin()+_training_data_idx, _buffer_current.begin(), _buffer_current.end());
                _training_data_idx = _training_data_idx + _window_length_samples;
            } else {
                InitializeThresholds(_training_buffer);
                _thresholds_initialized = true;
            }
        }

        // Thresholds are initialized; Now we can start QRS Detection with the incoming windows
        // Detect peaks in the filtered window:
        //const auto peaks = FindPeaks(_buffer_current);

        // Or just return a vector of the found locations?
        // Dont clear the buffer because we need some samples?
        //_buffer.clear();
    }
}

template<typename DataType_TP>
inline const std::vector<DataType_TP> PanTopkinsQRSDetection<DataType_TP>::GetCurrenRPeaks()
{
    return std::vector<DataType_TP>();
}

// \param training_data needs to be fully filtered (the MA-integrated) signal
template<typename DataType_TP>
void 
PanTopkinsQRSDetection<DataType_TP>::InitializeThresholds(const std::vector<DataType_TP>& training_data) 
{
    // Signal threshold
    //0.25 of the max amplitude is signal thresh
    //DataType_TP max_ampl_val = ;
    _signal_threshold = *std::max_element(training_data.begin(), training_data.end());// *(1 / 3);

    // just for testing:
    //std::vector<float> test({ 1,2,3,4 });
    std::vector<float> test; 
    test.push_back(1);
    test.push_back(2);
    test.push_back(4);

    float test_val = *std::max_element(test.begin(), test.end());
    // Calculate signal mean
    DataType_TP signal_sum = 0;
    for ( const auto& sample : training_data ) {
        signal_sum += sample;
    }
    DataType_TP signal_mean = signal_sum / training_data.size();

    // 0.5 of the mean signal is considered to be noise
    _noise_threshold = signal_mean * 1/2;

}

template<typename DataType_TP>
inline 
void PanTopkinsQRSDetection<DataType_TP>::Initialize(float sample_freq_hz, unsigned int training_phase_duration_sec)
{

    _sample_freq_hz = sample_freq_hz;
    _training_phase_duration_s = training_phase_duration_sec;
     // Create Buffers for Filtering (purpose is to keep a signal history)
    _window_length_samples = (static_cast<double>(_window_length_ms) / 1000.0) * _sample_freq_hz;
    _buffer_current.reserve(_window_length_samples);
    _buffer_current.resize(_window_length_samples);

    _number_of_training_samples = training_phase_duration_sec * _sample_freq_hz;
    _training_buffer.reserve(_number_of_training_samples);

    // TESTING
    
    // Apply to univector, static array, data by pointer or anything:
    /* kfr::univector<float> output;
    kfr::univector<float> input;
    fir_filter.apply(output, input);*/
}

template<typename DataType_TP>
inline 
void 
PanTopkinsQRSDetection<DataType_TP>::DerivationFilter(/*std::vector<DataType_TP>& data*/kfr::univector<DataType_TP>& data)
{
    // Create the derivation:
    // 
    // [data[1]-data[0], data[2]-data[1], ..., data[N]-data[N-1]]

    // Alternative: Don't do it inplace, but just create a new vector which is returned by value
    for ( int idx = 0; idx < data.size()-1; ++idx ) {
        //data[0] = data[1] - data[0]; 
        //data[1] = data[2] - data[1]; ...
        data[idx] = data[idx + 1] - data[idx];
    }
    // Pad the last value with zero because it does not exist.
    data[data.size()-1] = 0; 
}

template<typename DataType_TP>
inline
kfr::univector<DataType_TP> 
PanTopkinsQRSDetection<DataType_TP>::MovingWindowAveraging(const kfr::univector<DataType_TP>& data)
{
    // 'Center' implementation
    kfr::univector<DataType_TP> new_input;
    new_input.resize(data.size() + _window_length_samples);

    // => Pad zeros at beginning and end
    for ( int sample = 0; sample < _window_length_samples / 2; ++sample ) {
        // Pad at beginning=> instead o zeros, use the history of the last buffer and work with the samples from the last window
        new_input[sample] = 0;
        // Pad at end
        new_input[data.size() + sample] = 0;
    }
    // Now fill the rest of new_input with the original signal
    for ( int sample = 0; sample < data.size(); ++sample) {
        new_input[sample + _window_length_samples/2] = data[sample];
    }

    // Do the integration
    kfr::univector<DataType_TP> output;
    output.resize( new_input.size() );
    // iterate the original signal
    for( int counter = 0; counter < data.size(); ++counter){
        DataType_TP temp = 0;
        for ( int window_sample_idx = 0; window_sample_idx < _window_length_samples; ++window_sample_idx ) {
            temp += new_input[counter + window_sample_idx];
        }
        // Has the exact same size as the input signal
        output[counter] = temp / _window_length_samples;
    }

    return output;
}
