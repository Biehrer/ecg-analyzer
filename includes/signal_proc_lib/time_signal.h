#pragma once

// STL includes
#include <iostream>

class FileIO_C {

public:

    FileIO_C();
    FileIO_C();

public:
    void Initialize();
    void Read(std::string filename);

private:

};

template<typename FormatSpec_TP>
class TimeSignal_C {

public:
    TimeSignal_C();
    ~TimeSignal_C();

public:


private:

    std::vector<FormatSpec_TP> _data;

};