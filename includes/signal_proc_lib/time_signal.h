#pragma once

// Project includes
#include "file_io.h"
#include "mit_file_io.h"

// STL includes
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <istream>
#include <strstream>
#include <fstream>
#include<iterator>
#include <streambuf>
#include <cstddef>

template<typename DataFormat_TP>
struct ECGChannelInfo_TP {


    ECGChannelInfo_TP() 
    {
    }
    
    ECGChannelInfo_TP(const ECGChannelInfo_TP& other) {
        _data = other._data;
        _timestamps = other._timestamps;
        _units = other._units;
        _id = other._id;
        _label = other._label;
        _sample_rate_hz = other._sample_rate_hz;
        _high_hz = other._high_hz;
        _low_hz = other._low_hz;
        _range_mV = other._range_mV;
        _scale = other._scale;
    }

    //ECGChannelInfo_TP(ECGChannelInfo_TP&& other) {
    //    _data = other._data;
    //    _timestamps = other._timestamps;
    //    _units = other._units;
    //    _id = other._id;
    //    _label = other._label;
    //    _sample_rate_hz = other._sample_rate_hz;
    //    _high_hz = other._high_hz;
    //    _low_hz = other._low_hz;
    //    _range_mV = other._range_mV;
    //    _scale = other._scale;
    //}


public:
    void SetData(const std::vector<DataFormat_TP>& data) {
        _data = data;
    }

    double _high_hz = 0.0;

    double _low_hz = 0.0;

    //! sample rate in hz
    double _sample_rate_hz = 0.0;

    //! Name of the lead (e.g eindhoven or leeds)
    //! avL, avF, II, I
    std::string _label = "";

    //! SI-unit
    std::string _units = "mV";

    //! Range in mV
    uint32_t _range_mV = 0;

    //! Channel ID
    uint32_t _id = 0;

    //! scale / gain factor
    uint32_t _scale = 0;

    //! Channel samples
    std::vector<DataFormat_TP> _data;

    //! Channel timestamps. Each timestamp at a specific 
    //! position corresponds to the sample value at the same position inside the _data vector member
    std::vector<DataFormat_TP> _timestamps;
};


template<typename DataType_TP>
class TimeSignal_C {

public:
    //TimeSignal_C() = default;
    
    TimeSignal_C(const TimeSignal_C<DataType_TP>& signal);

    TimeSignal_C(/*const */TimeSignal_C<DataType_TP>&& signal);

    TimeSignal_C();

public:
    //! load physionet database file
    //!
    //! \param filename the path to the record WITHOUT the file suffix (.dat/.hea)
    void LoadFromMITFileFormat(const std::string filename);

    // For the custom dataset I use
    void ReadG11Data(const std::string& filename);

    const std::vector<ECGChannelInfo_TP<DataType_TP>>& constData() const {
        return _data;
    }

    std::vector<std::string> GetChannelLabels();

    std::string GetLabel() {
        return _label;
    }

    void SetLabel(const std::string& label);
    void SetID(unsigned int id);

    unsigned int GetChannelCount() { 
        return _data.size(); 
    }

    double GetTimerangeMs() { 
        if ( !_data.empty() ) { 
            return (_data[0]._timestamps.size() * ( 1 / _data[0]._sample_rate_hz) ) * 1000.0 ;
        } else {
            return 0.0;
        }
    }

    unsigned int GetID() { return _id; }

    std::string GetDatatype() {

        //if ( DataType_TP == float ) {
        //    return "float";
        //} else if ( DataType_TP == double ) {
        //    return "double";
        //} else if ( DataType_TP == int ) {
        //    return "int";
        //} 

        return "NA";
    }

    // Private helper functions
private:
    void GenerateTimestamps(std::vector<DataType_TP>& timestamp_vec,
        unsigned int num_samples,
        double sample_rate_hz);

private:
    std::vector<ECGChannelInfo_TP<DataType_TP>> _data;

    std::string _label = "";

    unsigned int _id = 0;

};

template<typename DataType_TP>
inline
std::vector<std::string>
TimeSignal_C<DataType_TP>::GetChannelLabels()
{
    std::vector<std::string> labels;
    for (const auto& channel_data : _data ) {
        labels.push_back(channel_data._label);
    }
    return labels;
}

template<typename DataType_TP>
inline 
void 
TimeSignal_C<DataType_TP>::SetLabel(const std::string & label)
{
    _label = label;
}

template<typename DataType_TP>
inline 
void 
TimeSignal_C<DataType_TP>::SetID(unsigned int id)
{
    _id = id;
}

template<typename DataType_TP>
inline 
TimeSignal_C<DataType_TP>::TimeSignal_C(const TimeSignal_C<DataType_TP>& signal)
{
    _data = signal._data;
    _id = signal._id;
    _label = signal._label;
}

template<typename DataType_TP>
inline 
TimeSignal_C<DataType_TP>::TimeSignal_C(TimeSignal_C<DataType_TP>&& signal)
{
    _data = signal._data;
    //signal._data = nullptr;
    _id = signal._id;
    _label = signal._label;
}

template<typename DataType_TP>
inline 
TimeSignal_C<DataType_TP>::TimeSignal_C()
{
}

template<typename DataType_TP>
void 
TimeSignal_C<DataType_TP>::LoadFromMITFileFormat(const std::string filename)
{
    MITFileIO_C<DataType_TP> reader;
    // Prepare wfdb path variable
    // Extraxt the path to the directory in which the record is located
    auto last_bslash_pos = filename.find_last_of('/\\');
    auto record_dir_path = filename.substr(0, last_bslash_pos);

    if ( last_bslash_pos == std::string::npos ) {
        std::cout << "found no slash! filename probably wrong!. Try to place the record in a subfolder" << std::endl;
        return;
    }
    // Set the path of the directory in which the record is located (this is required by the wfdb lib)
    char database_path_char[128];
    strcpy_s(database_path_char, record_dir_path.size() + 1, record_dir_path.c_str() );
    reader.SetWFDBPath(database_path_char);

    // Now extract just the name of the record without the directory path (I should do all this stuff inside the reader itself probably?)
    auto record_name = filename.substr(last_bslash_pos + 1);
    // remove the data suffix .dat or .hea (each 4 chars) if there is a dot inside the record name
    if( record_name.find('.') != std::string::npos ){
        record_name = record_name.substr(0, record_name.size() - 4);
    }
    char record_name_char[128];
    strcpy_s(record_name_char, record_name.size() + 1, record_name.c_str());
    auto mit_data = reader.Read(record_name_char);

    // Translate the data structure of the wfcb lib to the ECGChannelInfo_TP datastructure
    std::vector<ECGChannelInfo_TP<DataType_TP>> ecg_data;
    ecg_data.reserve(mit_data.size());
    ecg_data.resize(mit_data.size());
    
    unsigned int channel_idx = 0;
    for ( const auto& mit_channel : mit_data ) {
        ecg_data[channel_idx]._sample_rate_hz = mit_channel._sample_frequency_hz;
        ecg_data[channel_idx]._data = mit_channel._data;
        ecg_data[channel_idx]._label = mit_channel._description;
        ecg_data[channel_idx]._id = channel_idx;
        // ecg_data[channel_idx]._scale = /* Todo */;
        // ecg_data[channel_idx]._range_mV = /* Todo */;
        ecg_data[channel_idx]._units = mit_channel._units;

        // scale y values to the real voltage range (physical units)
        std::for_each(ecg_data[channel_idx]._data.begin(),
                      ecg_data[channel_idx]._data.end(),
            [&](auto& element)
        {
            element = (element - mit_channel._adc_baseline_0U_output_mV) / mit_channel._gain;
        });

        GenerateTimestamps(ecg_data[channel_idx]._timestamps,
            mit_channel._num_samples,
            mit_channel._sample_frequency_hz);

        ++channel_idx;
    }

    // Set data
    _data = ecg_data;
}

template<typename DataType_TP>
void
TimeSignal_C<DataType_TP>::ReadG11Data(const std::string& filename)
{
    FileIO_C filereader;
    bool success = filereader.OpenFile(filename);

    if ( !success ) {
        std::cout << "could not open the file" << std::endl;
        return;
    }
    // Stores all channels + header and body data
    std::vector< ECGChannelInfo_TP<DataType_TP> > channels;

    // Process the header
    int current_channel_idx = 0;
    int num_of_channels = 0;
    bool header_processed = false;
    while ( !header_processed ){
        auto line_str = filereader.ReadLine();
        std::string key_str;
        std::string value_str;
        auto pos_colon = line_str.find(":");

        if ( pos_colon == std::string::npos ) {
            key_str = line_str;
            value_str = "0";
        } else {
            key_str = line_str.substr(0, pos_colon);
            value_str = line_str.substr(pos_colon + 1, line_str.size());
        }

        if ( key_str == "Channels exported" ) {
            num_of_channels = std::stoi(value_str);
            // preallocate all channels inside the vector with standard c'tor
            for ( int count = 0; count < num_of_channels; ++count ) {
                channels.push_back({});
            }
        } else if ( key_str == "Channel #" ) {
            int channel_id = std::stoi(value_str);
            current_channel_idx = channel_id - 1;
            channels[current_channel_idx]._id = channel_id;
        } else if ( key_str == "Label" ) {
            channels[current_channel_idx]._label = value_str;
        } else if ( key_str == "Range" ) {
            channels[current_channel_idx]._range_mV = std::stoi(value_str);
        } else if ( key_str == "Sample rate" ) {
            channels[current_channel_idx]._sample_rate_hz = std::atof(value_str.c_str());
        } else if (key_str == "Scale"){
            channels[current_channel_idx]._scale = std::stoi(value_str);
            if ( current_channel_idx + 1 == num_of_channels ) {
                header_processed = true;
            }
        }
    }

    if ( channels.empty() ) {
        std::cout << "there is no channel data inside the header" << std::endl;
        return;
    }

    // Process data body
    // Count lines from end of the header to the end of the file (data-body)
    // We do this so its easy to read the data matrix with a specific format 
    // (e.g like matlab fscanf function)
    auto num_of_data_rows = filereader.CountLines<char>();

    // + 1 because the first column is just the enumeration for the values
    // (they also need memory) and it does not count as a 'channel'
    auto channel_data = filereader.ReadColumnData<DataType_TP>(num_of_channels + 1, num_of_data_rows);
    // File reading finished
    filereader.CloseFile();

    // start at channel_idx = 1 because the first channel, 
    // at position zero inside the vector,
    // is just the enumeration for the sample values 
    // (a 'time series' with consecutive values, starting at zero (0,1,2,3,..num_of_data_rows)
    int channel_idx = 1;
    for ( auto& channel : channels ) {
        channel.SetData(channel_data[channel_idx]);
        ++channel_idx;
    }

    for( auto& ecg_channel : channels ) {
        // scale y values to the real voltage range
        auto max_y_val_dataset = *max_element(ecg_channel._data.begin(),
                                              ecg_channel._data.end()); 
        auto scale_factor = ecg_channel._range_mV / max_y_val_dataset;
        ecg_channel._scale = scale_factor;
        std::for_each(ecg_channel._data.begin(), 
                      ecg_channel._data.end(), 
            [&](auto& element)
        {
            element *= scale_factor;
        });
        
        // Calculate x-values if there are none values inside the timestamps attribute
        if( ecg_channel._timestamps.empty() ) {
            GenerateTimestamps(ecg_channel._timestamps, 
                ecg_channel._data.size(),
                ecg_channel._sample_rate_hz);
        }
    }

    // Set the data
    _data = channels;
}


template<typename DataType_TP>
void
TimeSignal_C<DataType_TP>::GenerateTimestamps(std::vector<DataType_TP>& timestamp_vec, 
                                              unsigned int num_samples, 
                                              double sample_rate_hz) 
{
    timestamp_vec.reserve(num_samples);
    double t_dist_s = 1.0 / sample_rate_hz;
    for (unsigned int sample_idx = 0; sample_idx < num_samples; ++sample_idx ) {
        timestamp_vec.emplace_back(t_dist_s * sample_idx);
    }

}