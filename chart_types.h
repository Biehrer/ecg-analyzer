#pragma once

// STL includes
#include <vector>
#include <iostream>
#include <assert.h>
#include <vector>
#include <algorithm>
#include <mutex>
#include <atomic>

//! Representation of a position in 3d space containing three components: x, y and z
template<typename ElementType_TP>
class Position3D_TC {

public:
    Position3D_TC()
        :
        _x(0),
        _y(0),
        _z(0)
    {
    }

    Position3D_TC(ElementType_TP x, ElementType_TP y, ElementType_TP z)
        :
        _x(x),
        _y(y),
        _z(z)
    {
    }

    Position3D_TC(const Position3D_TC<ElementType_TP>& position)
        :
        _x(position._x),
        _y(position._y),
        _z(position._z)
    {
    }

public:
    //! x value
    ElementType_TP _x;

    //! y value
    ElementType_TP _y;

    //! z value
    ElementType_TP _z;

    // Operator overloads
public:
    Position3D_TC<ElementType_TP>& operator=(const Position3D_TC<ElementType_TP>& lhs) {
        // self-assignment guard
        //if ( this == &lhs ){
        //    return *this;
        //}
        _x = lhs._x;
        _y = lhs._y;
        _z = lhs._z;
        return *this;
    }

    bool operator==(const Position3D_TC<ElementType_TP>& lhs) {
        return (_x == lhs._x && _y == lhs._y && _z == lhs._z);
    }

    friend std::ostream& operator<<(std::ostream& s, const Position3D_TC& lhs) {
        s << "Element: " << "x: " << lhs._x
            << ", y: " << lhs._y
            << ", z: " << lhs._z
            << std::endl;
        return s;
    }

};


//! Defines which clock to use to record timestamps
using ClockType = std::chrono::system_clock;

//! Representation of a timestamp 
struct Timestamp_TP {
    // Construction / Destruction / Copying
public:
    Timestamp_TP()
        : _timestamp(ClockType::now())
    {
    }

    Timestamp_TP(double current_time_sec)
    {
        auto converted_time = ClockType::from_time_t(time_t(current_time_sec/* * 1000.0*/)).time_since_epoch();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(converted_time);
        _timestamp = std::chrono::time_point<ClockType>(duration);
    }

    Timestamp_TP(const std::chrono::time_point<ClockType>& timestamp)
        : _timestamp(timestamp)
    {
    }

    bool operator<(const Timestamp_TP& rhs) {
        return GetMilliseconds() < rhs.GetMilliseconds();
    }

    // Public access functions
public:
    //! Records a timestamp
    //! Stores the current time as timestamp
    void Now() { _timestamp = ClockType::now(); }

    //! Get the timestamp in milliseconds
    size_t GetMilliseconds() const { return std::chrono::duration_cast<std::chrono::milliseconds>(_timestamp.time_since_epoch()).count(); }
    size_t GetNanoseconds() { return std::chrono::duration_cast<std::chrono::nanoseconds>(_timestamp.time_since_epoch()).count(); }
    size_t GetSeconds() { return std::chrono::duration_cast<std::chrono::seconds>(_timestamp.time_since_epoch()).count(); }

    //! Get the timestamp in any resolution
    //!
    //! Usage of the template parameter:
    //! TimeRes = std::chrono::x , with x = milliseconds, seconds, nanoseconds,...
    template<typename TimeRes> size_t GetTime() {
        return std::chrono::duration_cast<TimeRes>(_timestamp.time_since_epoch()).count();
    }

    //! Returns a pointer to the internal time_point object
    std::chrono::time_point<ClockType>* GetTimestampPtr() { return &_timestamp; }

    // Private attributes
private:
    std::chrono::time_point<ClockType> _timestamp;
};


//! Todo::
//! 
template<typename Datatype_TP>
struct ChartPoint_TP
{
    // Construction / Destruction / Copying
public:
    ChartPoint_TP(Datatype_TP& value, const Timestamp_TP& timestamp)
        :
        _value(value),
        _timestamp(timestamp)
    {
    }

    ChartPoint_TP(Datatype_TP& value, double timestamp)
        :
        _value(value),
        _timestamp(timestamp)
    {
    }

    ChartPoint_TP()
    {
    }

    const bool operator<(const ChartPoint_TP& rhs) const {
        return _timestamp.GetMilliseconds() < rhs._timestamp.GetMilliseconds();
    }

    const bool operator<(const Timestamp_TP& timestamp) {
        return _timestamp.GetMilliseconds() < timestamp.GetMilliseconds();
    }
    // Public attributes
public:
    //! Stores the scaled values for the visualization 
    //! and not the raw values which were used as input from the user
    Datatype_TP _value;

    //! Timestamp of this chart point
    Timestamp_TP _timestamp;
};

//! Compare function to compare a timestamp_TP with a ChartPoint_TP (comparison is done with the underlying timestamp)
inline bool CmpTimestamps(const ChartPoint_TP<Position3D_TC<float>>& rhs, const Timestamp_TP& timestamp)
{
    return rhs._timestamp.GetMilliseconds() < timestamp.GetMilliseconds();
}