#pragma once

// STL includes
#include <vector>

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
