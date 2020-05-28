#pragma once

// Project includes
#include "file_io.h"
// WFDB includes
#include "wfdb/wfdb.h"

// There is a c++ wfdb library which offers functions to get MIT data from .hea and .dat files

//! One 
//! For information about the fields insid the header see: 
//! https://physionet.org/physiotools/wag/header-5.htm
//!
struct MITHeaderChannelInfo_TP {
    //! The label of the channel
    std::string _label;

    //! the sample frequency of this channel
    double sample_frequency;

   // The most common formats are format 8 (eight - bit first differences) 
   // and format 16 (sixteen - bit amplitudes); see signal(5) (or <wfdb / wfdb.h>)
    int format;
     
    // Todo: ...
    // std::vector data
};

struct MITChannelData_TP {
    //std::map<MITHeaderChannelInfo_TP, /*std::vector<> vector with body data*/ >
};

template<typename SampleDataType_TP>
class MITFileIO_C {
public:
    MITFileIO_C() = default;

public:
    bool OpenFile(const std::string& filename);

    const MITChannelData_TP Read();

    bool CloseFile();

private:
    std::vector< MITHeaderChannelInfo_TP > ReadMITHeader();

private:
    FileIO_C _filereader;
};

//const MITChannelData_TP MITFileIO_C::ReadMITDatFile() {
//
//    //char data[2] = _filereader.ReadBytes(2);
//}

template<typename SampleDataType_TP>
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
const MITChannelData_TP MITFileIO_C<SampleDataType_TP>::Read()
{
    int sample_size_bytes = 2;
    char* buffer = new char[sample_size_bytes];

    auto success = _filereader.ReadBytes(sample_size_bytes, buffer);
    uint16_t sample;
    sample << *buffer;

    delete buffer;
    return MITChannelData_TP();
}


template<typename SampleDataType_TP>
bool
MITFileIO_C<SampleDataType_TP>::CloseFile()
{
    if ( _filereader.IsOpen() ) {
        _filereader.CloseFile();
        return true;
    }
    else {
        return true;
    }
}


template<typename SampleDataType_TP>
std::vector< MITHeaderChannelInfo_TP>
MITFileIO_C<SampleDataType_TP>::ReadMITHeader()
{
    return{};
}


