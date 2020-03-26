#pragma once

// Project includes

// Qt includes

// STL includes
#include <vector>
#include <iostream>
#include <assert.h>

// Interface types of the CircularBuffer

// Representation of a position in 3d space containing three components: x, y and z
template<typename ElementType_TP>
class Position3D_TC {

public:
    Position3D_TC(ElementType_TP x, ElementType_TP y, ElementType_TP z)
        : _x(x),
          _y(y),
          _z(z)
    {
    }

    //! x value
    ElementType_TP _x;

    //! y value
    ElementType_TP _y;

    //! z value
    ElementType_TP _z;

    friend std::ostream& operator<<(std::ostream& s, const Position3D_TC& lhs){
        s << "Element: " << "x: " << lhs._x
          << ", y: " << lhs._y
          << ", z: " << lhs._z
          << std::endl;

        return s;
    }

};

//! Circular buffer class used as input bufer for OGLChart_C
//! After data is read with ReadLatestData(),
//! new data inside InsertAtHead(..) is written at the beginning of the buffer
template<typename ElementType_TP>
class CircularBuffer_TC
{
    // Construction / Destruction / Copying..
public:
    CircularBuffer_TC(int size)
        : _size(size)
    {
        _data_series_buffer = new ElementType_TP[_size];
    }

    ~CircularBuffer_TC()
    {
        delete _data_series_buffer;
    }

    // Public access functions
public:

    void InsertAtHead(ElementType_TP x, ElementType_TP y, ElementType_TP z)
    {
        assert(_head_idx <= _size && _head_idx >= 0 );
        _data_series_buffer[_head_idx] = x;
        _data_series_buffer[_head_idx + 1] = y;
        _data_series_buffer[_head_idx + 2] = z;
        _head_idx += 3;
    }

    void InsertAtHead(Position3D_TC<ElementType_TP> element)
    {
        assert(_head_idx <= _size && _head_idx >= 0 );
        _data_series_buffer[_head_idx] = element._x;
        _data_series_buffer[_head_idx + 1] = element._y;
        _data_series_buffer[_head_idx + 2] = element._z;
        _head_idx += 3;
    }

    //! Read latest data from the buffer.
    //! Returns a vector with zero elements if there is no new data to read (Check this with NewDataToRead())
    std::vector<Position3D_TC<ElementType_TP>> ReadLatest() {

        std::vector<Position3D_TC<ElementType_TP>> latest_data;

        if( NewDataToRead() ){
            // Because we reset the head index each time, we know
            int last_read_idx = 0;
            while (_head_idx > last_read_idx) {

                latest_data.emplace_back(_data_series_buffer[last_read_idx],
                                         _data_series_buffer[last_read_idx + 1],
                                         _data_series_buffer[last_read_idx + 2]);
                last_read_idx += 3;
            }
            // Reset head to zero because we always read everything and start writing at zero again..
            _head_idx = 0;
        }
        return latest_data;
    }

    bool NewDataToRead(){
        return _head_idx > _last_read_idx;
    }

    // Private attributes
private:
    //! Size of the buffer (maximum number of elements)
    int _size;

    //! The input buffer - use std vector to use memory locality (cache friendly)
    ElementType_TP* _data_series_buffer;

    //! Current write position inside the buffer
    // Is incremented each time new data was added via AddData(...)
    // Is resetted ( set to zero ) when the data is written into the charts vertex buffer object _chart_vbo
    int _head_idx = 0;

    //! The idx from which the buffer was read the last time
    int _last_read_idx = 0;
};
