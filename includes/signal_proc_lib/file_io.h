#pragma once

// STL includes
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <strstream>
#include <iterator>
#include <map>
#include <tuple>

class FileIO_C {

public:
    FileIO_C();

public:
    //! Opens the file @ filename
    //! Returns true on success
    bool OpenFile(const std::string& filename);

    //! Read2DPoints until some character or string sequence
    // void ReadUntil(const std::string & stop_string);

    //! Read2DPoints one line from the file
    const std::string ReadLine();

    // Reads multiple rows with multiple columns fast because the number of rows is known
    template<typename DataFormat_TP>
    const std::vector<DataFormat_TP> ReadRows(uint64_t num_cols, uint64_t num_rows);

    //! How to implement dynamic numbers of cols
    //! How to pass info: read the whole file -> pass n_rows = INF
    template<typename DataFormat_TP>
    std::map<unsigned int, std::vector<DataFormat_TP>> ReadColumnData(uint32_t n_cols, uint32_t n_rows);

    //template<class yValType, class xValType, uint32_t length>
    //void ReadSeparatedColumnData(char delemitter);

    template<class yVal, class xVal, uint32_t data_length>
    void Read2DPoints();

    template<class yValType, class xValType, uint32_t length>
    void ReadCSV(char delemitter);

    template<class Type_IT>
    int64_t CountLines();

    bool IsOpen() { return _filestream.is_open(); }

    //! CLoses the file
    //! Returns true if the file was closed
    //! Returns false if the file is already closed or 
    //! if the file is not even open
    bool CloseFile();

    //! Returns a ptr to the filestream
    std::ifstream* GetFile();

    //! Returns true when the fstream iterator reached the end of the file
    bool IsEof();
private:
    //!
    std::ifstream _filestream;

    std::string _filename;

    bool _is_open;
};

FileIO_C::FileIO_C() {
    _is_open = false;
}

bool
FileIO_C::OpenFile(const std::string & filename)
{
    _filestream.open(filename);

    if ( _filestream.is_open() &&
        _filestream.good() &&
        !_filestream.bad() )
    {
        _is_open = true;
        return true;
    }
    else
    {
        std::runtime_error("Coult nod open the file");
    }
    return false;
}



template<typename DataFormat_TP>
const std::vector<DataFormat_TP>
FileIO_C::ReadRows(uint64_t num_cols, uint64_t num_rows) {

    if ( !_filestream.good() ) {
        throw std::runtime_error("File seems to be closed!");
    }

    uint64_t num_values = (num_rows) * (num_cols); 
    std::vector<DataFormat_TP> values;
    values.reserve(num_values);
    values.resize(num_values);

    uint64_t idx = 0;
    DataFormat_TP value;
    while ( (_filestream >> value)  /*&& _filestream.good()*/ ) { 
        values[idx] = value;
        ++idx;
    }
    
    return values;
}

// Reads matrix ordered data from file
template<typename DataFormat_TP>
std::map<unsigned int, std::vector<DataFormat_TP>> FileIO_C::ReadColumnData(uint32_t n_cols, uint32_t n_rows)
{
    auto number_of_channels = n_cols;
    std::map<unsigned int, std::vector<DataFormat_TP> > channel_data;
  
    for ( int count = 0; count < number_of_channels; ++count ) {
        // preallocate the vectors inside the map
        std::vector<DataFormat_TP> empty;
        channel_data.insert(std::make_pair(count, empty));
    }

    auto data = ReadRows<DataFormat_TP>(number_of_channels, n_rows);

    uint32_t idx = 0;
   for( uint32_t row_id = 0; row_id < n_rows; ++row_id ){
       // for each row
       for ( uint32_t col_id = 0; col_id < number_of_channels; ++col_id ) {
           // look at each column inside the row (each element belongs to one channel)
           // and store each element inside the appropriate channel position inside map
           channel_data.at(col_id).push_back( data[idx] );
           ++idx;
       }
   }
   
    return channel_data;
}

// Can only read files with one column of timestamps and one column of ecg data
// Todo: adapt this so its possible to do this when there are multiple ecg leads (avR, II, I, HIS) inside the file
// Dynamic template parameter list ? |
// length does not need to be the exact length (number of liens) because it uses push back
template<class yValType, class xValType, uint32_t length>
void FileIO_C::Read2DPoints() {
    std::vector<std::pair<yValType, xValType>> data;
    data.reserve(length);
    yValType y_value;
    xValType x_value;
    // Process data and store inside the data vector
    while ( _filestream >> y_value >> timestamp ) {
        data.push_back(std::make_pair(y_value, timestamp));
    }
    return data;
}

//template<>
//void FileIO_C::ReadUntil(const std::string& stop_string) {}

const
std::string FileIO_C::ReadLine()
{
    std::string line;
    std::getline(_filestream, line);
    return line;
}

template<class yValType, class xValType, uint32_t length>
void FileIO_C::ReadCSV(char delemitter)
{
    std::vector<std::pair<yValType, xValType>> data;
    data.reserve(length);

    yValType y_value;
    xValType x_value;
    char token = delemitter;
    // Process data and store inside the data vector
    while ( (_filestream >> y_value >> token >> timestamp) && (token == delemitter) ) {
        data.emplace_back(std::make_pair(y_value, timestamp));
    }

    return data;
    //for ( std::string line; getline(_filestream, line); ){
        //...for each line in input...  
    //}
}

bool FileIO_C::CloseFile()
{
    if ( _filestream.is_open() ) {
        _filestream.close();
        _filestream.clear();
        return true;
    } else {
        return false;
    }
}

//! Counts lines ("rows", when counting a data matrix e.g) inside the opened file
//! Searches for '\n' chars. Starts counting from the current read position. 
//! Does not affect the stream iterator position and state.
template<class Type_IT>
int64_t FileIO_C::CountLines()
{
    if ( ! _filestream.is_open() && ! _filestream.good() && _filestream.bad() ) {
        return 0;
    }

    std::istream::iostate it_state_before_counting = _filestream.rdstate();
    // is.clear();
    std::istream::streampos it_pos_before_counting = _filestream.tellg();
    _filestream.unsetf(std::ios_base::skipws);
    
    auto count = std::count(std::istreambuf_iterator<Type_IT>(_filestream),
                             std::istreambuf_iterator<Type_IT>(), '\n');

    // Reset state and pos of filestream
    _filestream.setf(std::ios_base::skipws);
    _filestream.clear(); // previous reading may set eofbit
    //_filestream.eofbit = false;
    _filestream.seekg(it_pos_before_counting);
    _filestream.setstate(it_state_before_counting);
    return count;
}

std::ifstream* FileIO_C::GetFile()
{
    return &_filestream;
}

bool
FileIO_C::IsEof() {
    return _filestream.eof();
}

