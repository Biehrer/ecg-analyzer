#pragma once

// Project includes
#include <includes/chart_types.h>

// STL includes
#include <vector>
#include <iostream>
#include <assert.h>
#include <vector>
#include <algorithm>
#include <mutex>
#include <atomic>

//! Circular buffer class used as input bufer for OGLSweepChart_C
//! After data is read with ReadLatestData(),
//! new data inside InsertAtHead(..) is written at the beginning of the buffer
template<typename T>
class RingBuffer_TC
{
    // Construction / Destruction / Copying..
public:
    RingBuffer_TC(int size)
        :
        _max_size(size),
        _data_series_buffer(size),
        _number_of_elements(0)
    {
        _data_series_buffer.reserve(size);
        _data_series_buffer.resize(size);
    }

    ~RingBuffer_TC() 
    {
    }

    // Public access functions
public:
    //! Insert a new element inside the buffer
    //! If the buffer is full, continue writing at the beginning
    void InsertAtTail(const T& element)
    {
        std::unique_lock<std::mutex> lck(_lock);
        _data_series_buffer[_tail_idx] = element;

        _tail_idx = (_tail_idx + 1) % _max_size;
        lck.unlock();
        ++_number_of_elements;
    }

    // TODO: Return by reference??!
    //! Returns and removes the last added data from the buffer
    //!
    //! \returns removes and returns a copy of the last item
    const T Pop()
    {
        T item;
        if ( !IsBufferEmpty() ) {
            std::unique_lock<std::mutex> lck(_lock);
            item = _data_series_buffer[_head_idx];
            _head_idx = (_head_idx + 1) % _max_size;
            --_number_of_elements;
        } else {
            std::cout << "buffer empty, nothing to pop" << std::endl;
        }
        return item;
    }

    // TODO: Return by reference??!
    //! Returns and removes the latest data from the buffer
    //!
    //! \returns a vector with all data which was added until the buffer was read the last time
    const std::vector<T> PopLatest() {
        std::vector<T> latest_data;
        while ( !IsBufferEmpty() ) {
            std::unique_lock<std::mutex> lck(_lock);
            latest_data.push_back(_data_series_buffer[_head_idx]);
            _head_idx = (_head_idx + 1) % _max_size;
            --_number_of_elements;
        }
        return latest_data;
    }


    //! Returns the last item which was added to the buffer
    //! Does not remove the item.
    //!
    //!\returns copy of the last item added to the buffer
      T GetLatestItem() {
       std::unique_lock<std::mutex> lck(_lock);
       if( _tail_idx == 0 ){
         return _data_series_buffer[_max_size -1];
       } else {
          return _data_series_buffer[(_tail_idx - 1) % _max_size];
       }
    }

    bool IsBufferFull() {
        std::unique_lock<std::mutex> lck(_lock);
        return _head_idx == (_tail_idx + 1) % _max_size;
    }

    //! Returns true when there are no elements inside the buffer
    bool IsBufferEmpty() {
        std::unique_lock<std::mutex> lck(_lock);
        return _head_idx == _tail_idx;
    }

    //! Returns the current number of elements inside the buffer
    int Size() {
        return _number_of_elements; 
    }

    //! Returns the maximum possible elements 
    int MaxSize() {
        std::unique_lock<std::mutex> lck(_lock);
        return _max_size;
    }

    const std::vector<T>& constData() const {
        return _data_series_buffer;
    }
    // Private attributes
private:
    //! The input buffer
    std::vector<T> _data_series_buffer;

    //! Current write position inside the buffer
    //! Is incremented each time new data was added via AddData(...)
    //! Is resetted ( set to zero ) when the data is written into the charts vertex buffer object _chart_vbo
    int _head_idx = 0;

    //! The idx from which the buffer was read the last time
    int _tail_idx = 0;

    //! Size of the buffer (maximum number of elements)
    int _max_size;

    //! current amount of elements inside the buffer
    std::atomic<unsigned int> _number_of_elements;

    //! Protects shared access to the container
    std::mutex _lock;
};



//! Flush buffer class used as input bufer for OGLSweepChart_C
//! After data is read with ReadLatestData(),
//! new data inside InsertAtHead(..) is written at the beginning of the buffer
//!
//! A rush buffer:
//! Once read, data is removed from the buffer!
template<typename ElementType_TP>
class InputBuffer_TC
{
    // Construction / Destruction / Copying..
public:
    InputBuffer_TC(int size)
        : _max_size(size)
    {
        _data_series_buffer = new ElementType_TP[_max_size];
    }

    ~InputBuffer_TC()
    {
        delete _data_series_buffer;
    }

    // Public access functions
public:

    void InsertAtHead(ElementType_TP x, ElementType_TP y, ElementType_TP z)
    {
        assert(_head_idx <= _max_size && _head_idx >= 0 );
        std::unique_lock<std::mutex> lck (_lock);

       if( _head_idx <= _max_size - 3 ) {
            _data_series_buffer[_head_idx] = x;
            _data_series_buffer[_head_idx + 1] = y;
            _data_series_buffer[_head_idx + 2] = z;
            _head_idx += 3;
       }else{
            //std::cout << "---BUFFER FULL---" << std::endl;
        }
    }

    void InsertAtHead(Position3D_TC<ElementType_TP> element)
    {
        assert(_head_idx <= _max_size && _head_idx >= 0 );
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
           std::unique_lock<std::mutex> lck (_lock);

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
       std::unique_lock<std::mutex> lck (_lock);
        return _head_idx > _last_read_idx;
    }

    // Private attributes
private:
    //! Size of the buffer (maximum number of elements)
    int _max_size;

    //! The input buffer - use std vector to use memory locality (cache friendly)
    ElementType_TP* _data_series_buffer;

    //! Current write position inside the buffer
    //! Is incremented each time new data was added via AddData(...)
    //! Is resetted ( set to zero ) when the data is written into the charts vertex buffer object _chart_vbo
    int _head_idx = 0;

    //! The idx from which the buffer was read the last time
    int _last_read_idx = 0;

    //! Protects shared access
    std::mutex _lock;
};

