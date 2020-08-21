#pragma once

#include "kfr/base.hpp"
#include "kfr/dsp.hpp"
#include "kfr/io.hpp"


#include <iostream>

template<typename DataType_TP>
class PanTopkinsQRSDetection {

    // Constructor / Destrcutor/...
public:
    PanTopkinsQRSDetection();
    
    // Public functions
public:
    
    //const std::vector<DataType_TP> DetectRPeaks(const std::vector<DataType_TP>& ecg_signal, bool use_learning_phase);
//private:
    // Initializes the thresholds
    void BeginLearningPhase(const std::vector<DataType_TP>& training_data);

    // Applys filtering on a window
    //const std::vector<DataType_TP> ApplyBandpassFilter(const std::vector<DataType_TP>& signal, 
    //                                                   double low_cut_freq_hz, 
    //                                                   double high_cut_freq_hz);


    // 
    //MovingAverageWindowIntegration(const std::vector<DataType_TP>& signal, double window_length_ms);

//private:
public:
    void Initialize();

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

private:
    // Pan-Topkins adaptive Threshold parameters
    double _signal_level = 0.0;
    double _noise_level = 0.0;
    double _signal_threshold = 0.0;
    double _noise_threshold = 0.0;

    // Pan Topkins Parameters for the algorithm
    //! 200 ms after a QRS complex was detected, its physiological not possible that another qrs complex appears.
    //! This parameter reduces false positives in the detection.
    double _refractory_period_ms = 200.0;
    
    //! The time after a QRS complex, when we expect the T-Wave 
    //! T-Waves can only be found after the passing of the refractory_period but before the t_wave_period passed
    double _t_wave_period_ms = 360.0;
    
    //! Duration  which will be used for the initialization of the thresholds in seconds
    unsigned int _training_phase_duration_s = 2;

    // The window length for the moving-average-integration in milliseconds.
    // A QRS normally has a max width of 150 milliseconds.
    unsigned int _window_length_ms = 150; 

    // Filter parameters
    //! Filter taps / order
    unsigned int _filter_order = 64; 
    
    //! Cutoff frequency of the lowpass filter 
    //! (all frequencies below this one will be left in the signal after filtering)
    unsigned int _low_cutoff_freq_hz = 11;

    //! Cutoff frquency of the highpass filter
    //! (all frequencies above this one will be left in the signal after filtering)
    unsigned int _high_cutoff_freq_hz = 5;

    // Parameter of current signal
    double _sample_freq_hz = 0.0;

   // kfr::filter_fir _lowpass_filter;
    //kfr::filter_fir _highpass_filter;

    //kfr::fir_bandpass _bandpass_filter;
    kfr::filter_fir<kfr::fbase, float>* _bandpass_filter;
};


template<typename DataType_TP>
PanTopkinsQRSDetection<DataType_TP>::PanTopkinsQRSDetection() 
    : _bandpass_filter(/*kfr::univector<kfr::fbase, 64>*/)
{

}

template<typename DataType_TP>
void 
PanTopkinsQRSDetection<DataType_TP>::BeginLearningPhase(const std::vector<DataType_TP>& training_data) 
{
    std::cout << "placeholder";
}

template<typename DataType_TP>
inline void PanTopkinsQRSDetection<DataType_TP>::Initialize()
{
    kfr::univector<kfr::fbase, 64> taps64;
    kfr::expression_pointer<kfr::fbase> window = kfr::to_pointer(kfr::window_kaiser(taps64.size(), 3.0));

    // Fill taps127 with the band pass FIR filter coefficients using kaiser window and cutoff=0.2 and 0.4 Hz
    kfr::fir_bandpass(taps64, 0.2, 0.4, window, true);
    // Initialize FIR filter with float input/output and fbase taps
    kfr::filter_fir<kfr::fbase, float> fir_filter(taps64);
    // initialize the class member filter
    _bandpass_filter->set_taps(taps64); // PRODUCES ERROR

    //_bandpass_filter = fir_filter;
    // TESTING
    // Apply to univector, static array, data by pointer or anything
    kfr::univector<float> output;
    kfr::univector<float> input;
    fir_filter.apply(output, input);

    
    // Initialize taps
    //univector<float, _filter_order> taps;
    //// Initialize window function
    //kfr::expression_pointer<kfr::fbase> window = kfr::to_pointer(kfr::window(taps.size(), 3.0));

    //// Initialize taps
    //kfr::fir_bandpass _bandpass_filter = kfr::fir_bandpass(taps, _high_cutoff_freq_hz, _low_cutoff_freq_hz, window, true);

    //// Initialize filter and delay line
    //filter_fir<float> filter(taps);


    //_lowpass_filter.set_taps = _filter_order;
    //kfr::univector<int, 1> taps = kfr::make_univector<int>(1);
    //_lowpass_filter = kfr::make_fir_filter<float, float>(taps)

    // TO Filter a window of signals from input to output buffer use:
    //_lowpass_filter.process_buffer(,)
    // OR
    //_lowpass_filter.apply()

    // We can directly process the incoming data and process it in-place

  
}