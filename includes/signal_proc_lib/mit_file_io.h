#pragma once

#define __cplusplus

// Project includes
#include "file_io.h"

// WFDB includes
extern "C"{
#include "wfdb/wfdb.h"
}

// STL includes
#include <typeinfo>
#include <stdio.h>
#include <iostream>
#include <string>
#include <malloc.h>

//! Interface for MIT-ECG Data outside this class,
//! so nobody needs to know the C-Datatypes offered by the wfdb library
//!
//! For information about the fields insid the header see: 
//! https://physionet.org/physiotools/wag/header-5.htm
//!
 template<typename SampleDataType_TP>
struct MITDataChannel_TP {
    //! name of the record
    std::string _filename;
    //! channel label
    std::string _description;
    //! SI-units( mv or other )
    std::string _units;
    //! gain
    double _gain = 0.0;
    //! the sample frequency of this channel
    double _sample_frequency_hz = 0.0;
    // adc input in physical units when it outputs 0 V
    double _adc_baseline_0mV_output_U = 0.0;
    // adc output in volts when 0 physical units used as input
    double _adc_baseline_0U_output_mV = 0.0;
    //! number of data samples
    unsigned int _num_samples = 0;
    //! adc resolution in bits
    unsigned int _adc_resolution_bits = 0;
    // vector with sampled data
    std::vector<SampleDataType_TP> _data;
};

template<typename SampleDataType_TP>
class MITFileIO_C {
public:
    MITFileIO_C() = default;

    ~MITFileIO_C();
public:
    bool OpenFile(const std::string& filename);

    const std::vector<MITDataChannel_TP<SampleDataType_TP>> Read(char* record_path);

    bool CloseFile();

    void SetWFDBPath(char* path);

    const std::string GetWFDBPath();

private:
    FileIO_C _filereader;
};

template<typename SampleDataType_TP>
inline 
MITFileIO_C<SampleDataType_TP>::~MITFileIO_C() 
{
    // wfdbquit();
}

template<typename SampleDataType_TP>
inline
bool
MITFileIO_C<SampleDataType_TP>::OpenFile(const std::string& filename)
{
    if ( !_filereader.IsOpen() ) {
        _filereader.OpenFile(filename);
        return true;
    }
    else {
        return false;
    }
}


template<typename SampleDataType_TP>
inline
const 
std::vector<MITDataChannel_TP<SampleDataType_TP>>
MITFileIO_C<SampleDataType_TP>::Read(char* record_path)
{
    int number_of_signals = isigopen(record_path, NULL, 0);

    if ( number_of_signals < 1 ) {
        return {/*no signal inside the file*/};
    }

    // Read the header 
    WFDB_Siginfo *signal_info;
    signal_info = (WFDB_Siginfo *)malloc(number_of_signals * sizeof(WFDB_Siginfo));
    number_of_signals = isigopen(record_path, signal_info, number_of_signals);

    std::vector<MITDataChannel_TP<SampleDataType_TP>> channel_data;
    channel_data.reserve(number_of_signals);
    channel_data.resize(number_of_signals);

    auto sample_freq_hz = static_cast<double>(sampfreq(record_path));
    for (int channel_count = 0; channel_count < number_of_signals; ++channel_count ) {
        // filename
        channel_data[channel_count]._filename = signal_info[channel_count].fname;
        // number of samples
        channel_data[channel_count]._num_samples = signal_info[channel_count].nsamp;
        // mv or other 
        channel_data[channel_count]._units = signal_info[channel_count].units;
        // description
        channel_data[channel_count]._description = signal_info[channel_count].desc;
        // sample freq
        channel_data[channel_count]._sample_frequency_hz = sample_freq_hz;
        // gain
        channel_data[channel_count]._gain = signal_info[channel_count].gain;
        // adc resolution
        channel_data[channel_count]._adc_resolution_bits = signal_info[channel_count].adcres;
        // adc output in mV when given 0 physical units as input
        channel_data[channel_count]._adc_baseline_0U_output_mV = signal_info[channel_count].baseline;
        // adc output (in physical units) given 0V DC input
        channel_data[channel_count]._adc_baseline_0mV_output_U = signal_info[channel_count].adczero;
    }
    
    // Read the body
    WFDB_Sample *sample_data;
    sample_data = (WFDB_Sample *)malloc(number_of_signals * sizeof(WFDB_Sample));
 
    // for ( int channel_count = 0; channel_count < number_of_signals; ++channel_count) {
        // assume num_samples is equal for all channels
        int number_of_samples = channel_data[/*channel_count*/0]._num_samples;
        // Read a sample from each channel and store it inside the corresponding MITDataChannel_TP object
        for ( int sample_count = 0; sample_count < number_of_samples; ++sample_count ) {
             // error codes
             //-1 End of data (contents of vector not valid)
             //-3 Failure: unexpected physical end of file
             //-4 Failure : checksum error(detected only at end of file)
            if ( getvec(sample_data) < 0 ){
                break;
            }
            // insert into data vector
            for ( int channel_id = 0; channel_id < number_of_signals; ++channel_id ) {
                channel_data[channel_id]._data.push_back(sample_data[channel_id]);
            }
        }
    // }

    return channel_data;
}


template<typename SampleDataType_TP>
inline
bool
MITFileIO_C<SampleDataType_TP>::CloseFile()
{
    if ( _filereader.IsOpen() ) {
        _filereader.CloseFile();
        return true;
    } else {
        return true;
    }
}

template<typename SampleDataType_TP>
inline 
void 
MITFileIO_C<SampleDataType_TP>::SetWFDBPath(char* path) 
{
    setwfdb(path);
}

template<typename SampleDataType_TP>
inline 
const 
std::string 
MITFileIO_C<SampleDataType_TP>::GetWFDBPath()
{
    return std::to_string(getwfdb());
}

