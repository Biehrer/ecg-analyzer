#pragma once

// STL includes
#include <iostream>
#include <fstream>
#include <string>

class FileIO_C {

public:
    FileIO_C();

public:
    static void OpenFile(const std::string& filename);
    static void Read();
    static void CloseFile();

private:
    //!
    static std::ifstream _filestream;

    static std::string _filename;

    static bool _is_open;
};
