#include "file_io.h"

FileIO_C::FileIO_C()
{
    _is_open = false;
}

void FileIO_C::OpenFile(const std::string & filename)
{
    if ( !_is_open ) {



        _is_open = true;
    }
}

void FileIO_C::Read()
{

}

void FileIO_C::CloseFile()
{
    if ( _is_open ) {



        _is_open = false;
    }
}

