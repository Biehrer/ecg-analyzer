#pragma once

//TODO:
// This file does not belong inside the visualization library. 
// This belongs in an utility library/Project, which is included by visualization and sig_proc_llib to use the RingBuffer 
//
// Project includes
#include "chart_Types.h"

// STL includes
#include <array>
#include <vector>
#include <iostream>
#include <assert.h>
#include <vector>
#include <algorithm>
#include <mutex>
#include <atomic>

#include <cstdint>

template<typename T>
class span {
    T* ptr_;
    std::size_t len_;

public:
    span(T* ptr, std::size_t len) noexcept
        : ptr_{ ptr }, len_{ len }
    {}

    span() noexcept
        : ptr_{ nullptr}, len_{0}
    {}

    T& operator[](int i) noexcept {
        return *ptr_[i];
    }

    T const& operator[](int i) const noexcept {
        return *ptr_[i];
    }

    std::size_t size() const noexcept {
        return len_;
    }

    T* begin() noexcept {
        return ptr_;
    }

    T* end() noexcept {
        return ptr_ + len_;
    }
};

enum RingBufferSize_TP {
    Size2, Size4, Size8, Size16, Size32,
    Size64, Size128, Size256, Size512,
    Size1024, Size2048, Size4096, Size8192,
    Size16384, Size32768, Size65536, Size131072,
    Size262144, Size524288, Size1048576, Size2097152, 
    SizeINVALID
};

// a really stupid function, but makes 
// the usage of the optimized buffer error prone..
inline
int 
TranslateRingBufferSize(RingBufferSize_TP size) 
{
    switch ( size ) 
    {
    case Size2: return 2; case Size4: return 4; 
    case Size8: return 8; case Size16: return 16;
    case Size32: return 32; case Size64: return 64;
    case Size128: return 128; case Size256: return 256;
    case Size512: return 512; case Size1024: return 1024;
    case Size2048: return 2048; case Size4096: return 4096;
    case Size8192: return 8192; case Size16384: return 16384;
    case Size32768: return 32768; case Size65536: return 65536;
    case Size131072: return 131072; case Size262144: return 262144;
    case Size524288: return 524288; case Size1048576: return 1048576;
    case Size2097152: return 2097152;
    }
    return SizeINVALID;
}

//! Circular buffer class used as input bufer for OGLSweepChart_C
//! => because masking with modulo is prevented and the & operator is used, 
//! you must use a buffer size which is a power of 2
template<typename T>
class RingBufferOptimized_TC
{
    // Construction / Destruction / Copying..
public:
    RingBufferOptimized_TC(RingBufferSize_TP size)
        :
        _data_series_buffer(TranslateRingBufferSize(size)),
        _number_of_elements(0)
    {
        _size = size;
        _max_size = TranslateRingBufferSize(size);
        _data_series_buffer.reserve(_max_size);
        _data_series_buffer.resize(_max_size);
    }

    RingBufferOptimized_TC(const RingBufferOptimized_TC& other) 
    {
        _size = other._size;
        _max_size = other._max_size;
        _data_series_buffer = other._data_series_buffer;
        _head_idx = other._head_idx;
        _tail_idx = other._tail_idx;
        //_lock = other._lock;
        _number_of_elements.store(other._number_of_elements.load());
    }

    RingBufferOptimized_TC& operator=(const RingBufferOptimized_TC& other) 
    {
        _size = other._size;
        _max_size = other._max_size;
        _data_series_buffer = other._data_series_buffer;
        _head_idx = other._head_idx;
        _tail_idx = other._tail_idx;
        //_lock = other._lock;
        _number_of_elements.store(other._number_of_elements.load());
        return *this;
    }
    // Public access functions
public:
    //! Insert a new element inside the buffer
    //! If the buffer is full, continue writing at the beginning
    void InsertAtTail(const T& element)
    {
        std::unique_lock<std::mutex> lck(_lock);
        _data_series_buffer[_tail_idx] = element;
        _tail_idx = (_tail_idx + 1) & (_max_size - 1);
        lck.unlock();
        ++_number_of_elements;
    }

    //! Returns and removes the last added data from the buffer
    //!
    //! \returns removes and returns a copy of the last item
    const T Pop()
    {
        T item;
        if ( !IsBufferEmpty() ) {
            std::unique_lock<std::mutex> lck(_lock);
            item = _data_series_buffer[_head_idx];
            _head_idx = (_head_idx + 1) & (_max_size - 1);
            --_number_of_elements;
        } else {
            std::cout << "buffer empty, nothing to pop" << std::endl;
        }
        return item;
    }


    //! Returns and removes the latest data from the buffer
    //!
    //! \returns a vector with all data which was added until the buffer was read the last time
    const std::vector<T> PopLatest() {
        std::vector<T> latest_data;
        latest_data.reserve(_number_of_elements);
        // This while loop could be dangerous, when a thread inserts data too fast.
        // the number of objects which should be poped from the buffer should be known at the beginning 
        //int current_size = Size();
        //std::unique_lock<std::mutex> lck(_lock);
        //for(int count = 0; count <= current_size; ++count ){
        while(!IsBufferEmpty() ) {
            std::unique_lock<std::mutex> lck(_lock);
            // Indexing vectors is faster than push_back and emplace
            latest_data.emplace_back(_data_series_buffer[_head_idx]);
            _head_idx = (_head_idx + 1) & (_max_size - 1);
            --_number_of_elements;
        }

        return latest_data;
    }

    /*const*/ /*array_view*/span<T> PopLatestRef() {
        if ( !IsBufferEmpty() ) {
            // return just a slice from the original vector
            int num_elements = _number_of_elements;
            span<T> av(&_data_series_buffer[_head_idx], num_elements);
            _head_idx = (_head_idx + num_elements) & (_max_size - 1);
            _number_of_elements -= num_elements;
            return av;
        } else {
            return {}; // TODO what is constructed here in span? nullptr?
        }

        //if ( !IsBufferEmpty() ){
        //    //return { _data_series_buffer[_head_idx], _data_series_buffer[_tail_idx]);
        //    //_number_of_elements -= (_tail_idx - _head_idx);
        //    //_head_idx = (_head_idx + 1) & (_max_size - 1);
        //    array_view<T> av(_data_series_buffer[_head_idx], _number_of_elements);
        //} else {
        //    return {};
        //}
    }



    //! Returns the last item which was added to the buffer.
    //! Returns the standard constructed item T, if no item is inside the buffer
    //! Does not remove the item.
    //!
    //!\returns copy of the last item added to the buffer
      T GetLatestItem() 
      {
          std::unique_lock<std::mutex> lck(_lock);
          if( _tail_idx == 0 ) { 
              // case: there is only one element inside the buffer
              if( _number_of_elements != 0 ){
               return _data_series_buffer[_max_size - 1];
              } else {
                  return {};
              }
          } else {
             return _data_series_buffer[(_tail_idx - 1) & (_max_size - 1)];
          }
    }

    bool IsBufferFull() {
        std::unique_lock<std::mutex> lck(_lock);
        return _head_idx == (_tail_idx + 1) & (_max_size - 1);
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

    RingBufferSize_TP GetRingBufferSizeTP() {
        return _size;
    }

    const std::vector<T>& constData() const {
        return _data_series_buffer;
    }

    // Private attributes
private:
    //! The input buffer
    std::vector<T> _data_series_buffer;

    RingBufferSize_TP _size;

    //! Current write position inside the buffer
    //! Is incremented each time new data was added via AddData(...)
    //! Is resetted ( set to zero ) when the data is written into the charts vertex buffer object _chart_vbo
    int _head_idx = 0;

    //! The idx from which the buffer was read the last time
    int _tail_idx = 0;

    //! Size of the buffer (maximum number of elements)
    int _max_size = 0;

    //! current amount of elements inside the buffer
    std::atomic<unsigned int> _number_of_elements;

    //! Protects shared access to the container
    std::mutex _lock;
};


//! The not optimized version of the RingBufferOptimized_TC (With the modulo operation)
//! This buffer can support all buffer sizes and is not limited to buffer sizes which are a power of two.
//!
//! Circular buffer class used as input bufer for OGLSweepChart_C
//! because masking with modulo is prevented and the & operator is used, 
//! you must use a buffer size which is a power of 2
template<typename T>
class RingBuffer_TC
{
    // Construction / Destruction / Copying..
public:
    RingBuffer_TC(size_t size)
        :
        _data_series_buffer(size),
        _number_of_elements(0)
    {
        _max_size = size-1;
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

    //! Returns and removes the last added data from the buffer
    //!
    //! \returns removes and returns a copy of the last item
    const T Pop()
    {
        T item;
        if ( !IsBufferEmpty() ) {
            std::unique_lock<std::mutex> lck(_lock);
            item = _data_series_buffer[_head_idx];
            // TODO: INSTEAD OF doing _max_size-1 every time, just do -1 inside the constructor????
            _head_idx = (_head_idx + 1) % _max_size/* - 1)*/;
            --_number_of_elements;
        }
        else {
            std::cout << "buffer empty, nothing to pop" << std::endl;
        }
        return item;
    }


    //! Returns and removes the latest data from the buffer
    //!
    //! \returns a vector with all data which was added until the buffer was read the last time
    const std::vector<T> PopLatest() {
        std::vector<T> latest_data;
        // This while loop could be dangerous when an thread inserts data too fast 
        // the number of objects which should be poped from the buffer should be known at the beginning 
        //int current_size = Size();
        //std::unique_lock<std::mutex> lck(_lock);
        //for(int count = 0; count <= current_size; ++count ){
        while ( !IsBufferEmpty() ) {
            std::unique_lock<std::mutex> lck(_lock);
            // Indexing vectors is faster than push_back and emplace
            latest_data.emplace_back(_data_series_buffer[_head_idx]);
            _head_idx = (_head_idx + 1) % _max_size;
            --_number_of_elements;
        }
        return latest_data;
    }

    //! Returns the last item which was added to the buffer.
    //! Returns the standard constructed item T, if no item is inside the buffer
    //! Does not remove the item.
    //!
    //!\returns copy of the last item added to the buffer
    T GetLatestItem()
    {
        std::unique_lock<std::mutex> lck(_lock);
        if ( _tail_idx == 0 ) {
            // case: there is only one element inside the buffer
            if ( _number_of_elements != 0 ) {
                return _data_series_buffer[_max_size];
            }
            else {
                return {};
            }
        }
        else {
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
        return _max_size+1;
    }

    int GetTailIdx() {
        std::unique_lock<std::mutex> lck(_lock);
        return _tail_idx;
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
    unsigned int _max_size = 0;

    //! current amount of elements inside the buffer
    std::atomic<unsigned int> _number_of_elements;

    //! Protects shared access to the container
    std::mutex _lock;
};


//! InputBuffer_TC class
//!
//! Flush buffer class used as input bufer for OGLSweepChart_C
//! After data is read with ReadLatestData(),
//! new data inside InsertAtHead(..) is written at the beginning of the buffer
//!
//! A flush buffer:
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

    //! The input buffer
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

