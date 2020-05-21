#pragma once

// Project includes
#include "file_io.h"

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

template<typename DataFormat_TP>
struct ECGChannelInfo_TP {

public:
    void SetData(const std::vector<DataFormat_TP>& data) {
        _data = data;
    }

    double _high_hz = 0.0;
    double _low_hz = 0.0;
    double _sample_rate_hz = 0.0;

    // Name of the lead (e.g eindhoven or leeds)
    // avL, avF, II, I
    std::string _label = "";
    uint32_t _range_mv = 0;
    uint32_t _id = 0;
    uint32_t _scale = 0;
    std::vector<DataFormat_TP> _data;
    // optional -> use inheritance
    std::vector<DataFormat_TP> _timestamps;
};


template<typename DataType_TP>
class TimeSignal_C {

public:
    //TimeSignal_C() = default;
    
public:
    //! \param filename the filename 
    //! \returns vector, each element one line of the file
    std::vector<std::string> LoadDataFromFile(const std::string& filename, int estimated_size);

    // For the custom dataset I use
    void ReadG11Data(const std::string& filename);

    std::vector<std::string> GetChannelLabels();

    const std::vector<ECGChannelInfo_TP<DataType_TP>>& constData() const {
        return _data;
    }
    //! load physionet database
    void LoadFromMITFileFormat(const std::string& filename);

private:
    std::vector<ECGChannelInfo_TP<DataType_TP>> _data;
};

template<typename DataType_TP>
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
TimeSignal_C<DataType_TP>::LoadFromMITFileFormat(const std::string & filename)
{
    MITFileIO_C reader;


    const auto data = reader.readFromFile();

}


template<typename DataType_TP>
//inline 
std::vector<std::string>
TimeSignal_C<DataType_TP>::LoadDataFromFile(const std::string & filename, int estimated_size)
{
    FileIO_C filereader;
    bool success = filereader.OpenFile(filename);

    if ( !success ) {
        return {};
    }

    std::vector<std::string> lines;
    lines.reserve(estimated_size);

    while ( !filereader.IsEof() ) {
        lines.push_back(filereader.ReadLine());
    }

    filereader.CloseFile();
    return lines;
}

template<typename DataType_TP>
void
TimeSignal_C<DataType_TP>::ReadG11Data(const std::string& filename)
{
    // auto line_strings = LoadDataFromFile(filename, 100000);

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
            channels[current_channel_idx]._range_mv = std::stoi(value_str);
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
        auto scale_factor = ecg_channel._range_mv / max_y_val_dataset;

        std::for_each(ecg_channel._data.begin(), 
                      ecg_channel._data.end(), 
            [&](auto& element)
        {
            element *= scale_factor;
        });
        
        // Calculate x-values if there are none values inside the timestamps attribute
        if( ecg_channel._timestamps.empty() ) {
            int number_of_samples = ecg_channel._data.size();
            ecg_channel._timestamps.reserve(number_of_samples);
            auto t_dist_s = 1.0 / ecg_channel._sample_rate_hz;
            for ( int sample_idx = 0; sample_idx < number_of_samples; ++sample_idx ) {
                ecg_channel._timestamps.emplace_back(t_dist_s * sample_idx);
            }
        }
    }

    // Set the data
    _data = channels;
}
