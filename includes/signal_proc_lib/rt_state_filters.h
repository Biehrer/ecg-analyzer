#pragma once

// STL includes
#include <vector>

// visualization includes - this is not good -> put the buffer in utility project/lib ?
#include "../visualization/circular_buffer.h"
//#include "../../includes/visualization/circular_buffer.h"

///////////////////////////////////////////////////////
//
// Class: DerivationStateFilter
//
template<typename DataType_TP>
class DerivationStateFilter {

public:
    void ResetState();
    void Apply(DataType_TP& value);

private:
    DataType_TP _last_input = 0;

    DataType_TP _current_input = 0;

    bool _running = false;
};

template<typename DataType_TP>
inline
void 
DerivationStateFilter<DataType_TP>::ResetState()
{
    _last_input = 0;
    _running = false;
}

// Needs at least two values to produce one valid output
template<typename DataType_TP>
inline 
void 
DerivationStateFilter<DataType_TP>::Apply(DataType_TP& value)
{
    _current_input = (value);
    value -= _last_input;
    _last_input = _current_input;
 
}

///////////////////////////////////////////////////////
//
// Class: MovingAverageStateFilter
//
template<typename DataType_TP>
class MovingAverageStateFilter {
    // Construction / Destruction / Copying..
public:
    MovingAverageStateFilter(int window_length_samples);
    MovingAverageStateFilter();

    // Public functions
public:
    /*void*/bool Apply(DataType_TP& sample);
    void ResetState();
    void SetParams(int window_length_samples);
    int GetFilterDelay();
    // Private vars
private:
    // sum of all samples inside the current 'window'
    DataType_TP _current_sum = 0;
    // number of saples processed since state reset
    size_t _num_samples = 0;
    // length of the sliding window in samples
    int _interval_length = 0;
    // delay of the filter due to the sliding window in samples
    unsigned int _delay_samples = 0;

    // 'sliding window' buffer
    std::vector<DataType_TP> _input_buffer;
    //  this is the idx inside the input buffer, where a new value is inserted
    size_t _head_idx = 0;
    // idx of the value to be removed inside the input buffer, when a new value is added
    size_t _tail_idx = 0;
    
};

template<typename DataType_TP>
inline 
MovingAverageStateFilter<DataType_TP>::MovingAverageStateFilter(int window_length_samples)
{
    _interval_length = window_length_samples;
    _delay_samples = window_length_samples / 2;
    _input_buffer.reserve(window_length_samples);
    _input_buffer.resize(window_length_samples);
}

template<typename DataType_TP>
inline
MovingAverageStateFilter<DataType_TP>::MovingAverageStateFilter()
{
}

// Returns true when a valid output was produces, false if not.
template<typename DataType_TP>
inline
bool 
MovingAverageStateFilter<DataType_TP>::Apply(DataType_TP & sample)
{
    _current_sum += sample;
    ++_num_samples; 

    // store current sample
    _input_buffer[_head_idx] = sample;
    _head_idx = (_head_idx + 1) % _interval_length;

    // return moving average
    sample = _current_sum / static_cast<DataType_TP>(_interval_length);

    if (_num_samples <_interval_length) {
        // Until not the complete window was collected, return false
        return false;
    } else {
        // remove last sample from signal history 
        _current_sum -= _input_buffer[_tail_idx];
        // TODO: take in consideration that: _tail_idx == _head_idx -1, 
        // after init phase is completed
        // -> but then we need to check when _head_idx is zero, because then _tail_idx will be minus one if this is the case
        _tail_idx = (_tail_idx + 1) % _interval_length;
        return true;
    }
}

template<typename DataType_TP>
inline 
void 
MovingAverageStateFilter<DataType_TP>::ResetState()
{
    _current_sum = 0;
    _num_samples = 0;
    _head_idx = 0;
    _tail_idx = 0;
}

template<typename DataType_TP>
inline
void 
MovingAverageStateFilter<DataType_TP>::SetParams(int window_length_samples)
{
    _interval_length = window_length_samples;
    _delay_samples = window_length_samples/2;
    _input_buffer.reserve(window_length_samples);
    _input_buffer.resize(window_length_samples);

}

template<typename DataType_TP>
inline int MovingAverageStateFilter<DataType_TP>::GetFilterDelay()
{
    return _delay_samples;
}




inline 
long long 
mod_negative(const long long x, const long long y) 
{
    if ( x >= y ) {
        return x % y;
    }
    else if ( x < 0 ) {
        return (x % y + y) % y;
    }
    else {
        return x;
    }
}

///////////////////////////////////////////////////////
//
// Class: PeakDetectorFilter
//
template<typename DataType_TP>
class PeakDetectorFilter {

    // Construction / Destructon / Copying
public:
    PeakDetectorFilter(unsigned int buffer_size);

public:
    // First three return values ARE NOT VALID RETURN VALUES
    // Returns true when the sample previously added to the current sample was a peak.
    // This means output peak location is: current_sample_loc - 1
    bool Apply(const DataType_TP& sample);

    // A window version of the Apply() function, which returns the index of the found peaks inside the window-array
    //
    // The last value from the window is restored in the delay line to check if the first value of the next window is a peak
    // Returns an empty vector, if no peak was found
    std::vector<unsigned int> Apply(const std::vector<DataType_TP>& window);

private:
    RingBuffer_TC<DataType_TP> _buffer;

    DataType_TP _value_previous = 0;

    DataType_TP _value_prev_previous = 0;

    DataType_TP _last_value_last_window = 0;
};

template<typename DataType_TP>
PeakDetectorFilter<DataType_TP>::PeakDetectorFilter(unsigned int buffer_size)
    : _buffer(buffer_size)
{
}

template<typename DataType_TP>
inline
bool 
PeakDetectorFilter<DataType_TP>::Apply(const DataType_TP & sample)
{
    _buffer.InsertAtTail(sample);     

    const auto curr_head_idx = _buffer.GetTailIdx(); 
    // Do peak detection after three samples are aquired
    if ( _buffer.Size() >= 3 ) { 

        if ( curr_head_idx < 3 ) {
            // Special "wrap" case  -> don't access invalid memory locations ( When _head_idx < 2 )
            // When _head_idx == 0 -> compare _buffer[MAX] with _buffer[MAX-1] AND _buffer[0]
            // When _head_idx == 1 -> comp. _buffer[0] with _buffer[MAX] and _buffer[1]
            _value_previous = _buffer.constData()[ mod_negative(curr_head_idx - 2, _buffer.MaxSize()) ];
            _value_prev_previous = _buffer.constData()[ mod_negative(curr_head_idx - 3, _buffer.MaxSize()) ];
        } else {
            _value_previous = _buffer.constData()[ curr_head_idx - 2 ];
            _value_prev_previous = _buffer.constData()[ curr_head_idx - 3 ];
        }

        // Check if there was a peak
        if ( _value_previous >= _value_prev_previous ) {
            if ( _value_previous >= sample ) {
                // PEAK DETECTED
                return true;
            }
        }
    }

    return false;
}

template<typename DataType_TP>
inline
std::vector<unsigned int>
PeakDetectorFilter<DataType_TP>::Apply(const std::vector<DataType_TP>& window)
{
    std::vector<unsigned int> peak_indices;

    //First check the last value from the last window with the first value from the current window? Requires extra if(
    if (window[0] >= _last_value_last_window  &&
        window[0] >= window[1] )
    {
        peak_indices.push_back(0);
    }

    // Check for peaks inside the window
    for ( size_t idx = 1; idx < window.size(); ++idx ) {
        if ( window[idx-1] <= window[idx] &&
            window[idx] >= window[idx+1])
        {
            peak_indices.push_back(idx)
        }
    }

    // Remember last value from the window to check if the first value from the next window is a peak
    _last_value_last_window = (window.end() - 1); // I think end points at not the end but one after the end

    return peak_indices;
}
