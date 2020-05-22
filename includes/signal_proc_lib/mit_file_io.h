#pragma once

// Project includes
#include "file_io.h"

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

class MITFileIO_C {

    bool OpenFile(const std::string& filename);

    const MITChannelData_TP Read();

    bool CloseFile();

private:
    std::vector< MITHeaderChannelInfo_TP > ReadMITHeader();

private:
    FileIO_C _filereader;
};

const MITChannelData_TP MITFileIO_C::Read() {
}

bool
MITFileIO_C::OpenFile(const std::string& filename) 
{
    if( !_filereader.IsOpen() ) {
        _filereader.OpenFile(filename);
    }
}

bool 
MITFileIO_C::CloseFile() {
    if ( _filereader.IsOpen() ) {
        _filereader.CloseFile();
    }
}

std::vector< MITHeaderChannelInfo_TP > MITFileIO_C::ReadMITHeader() {}

