#pragma once

// STL includes
#include <vector>
#include <iostream>
#include <assert.h>
#include <vector>
#include <algorithm>
#include <mutex>
#include <atomic>

// Qt includes
#include <qvector.h>



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
    Position3D_TC<ElementType_TP>& operator=(const Position3D_TC<ElementType_TP>& rhs) {
        // self-assignment guard
        //if ( this == &rhs ){
        //    return *this;
        //}
        _x = rhs._x;
        _y = rhs._y;
        _z = rhs._z;
        return *this;
    }

    Position3D_TC<ElementType_TP>& operator-(const Position3D_TC<ElementType_TP>& rhs) {
        _x -= rhs._x;
        _y -= rhs._y;
        _z -= rhs._z;
        return *this;
    }

    Position3D_TC<ElementType_TP>& operator+(const Position3D_TC<ElementType_TP>& rhs) {
        _x += rhs._x;
        _y += rhs._y;
        _z += rhs._z;
        return *this;
    }

    bool operator==(const Position3D_TC<ElementType_TP>& rhs) {
        return (_x == rhs._x && _y == rhs._y && _z == rhs._z);
    }

    friend std::ostream& operator<<(std::ostream& s, const Position3D_TC& obj) {
        s << "Element: " << "x: " << obj._x
                        << ", y: " << obj._y
                        << ", z: " << obj._z
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
    const size_t GetMilliseconds() const { return std::chrono::duration_cast<std::chrono::milliseconds>(_timestamp.time_since_epoch()).count(); }
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


//! Storage for x and y axes vertices
struct XYAxisVertices_TP
{
public:
    XYAxisVertices_TP(QVector<float> x_axis_vertices,
        QVector<float> y_axis_vertices)
        :
        _x_axis_vertices(x_axis_vertices),
        _y_axis_vertices(y_axis_vertices)
    {
    }
    QVector<float> _x_axis_vertices;
    QVector<float> _y_axis_vertices;
};

template<typename PositionType_TP>
struct BoundingBoxArea_TP {

    BoundingBoxArea_TP(const Position3D_TC<PositionType_TP>& left_top,
        const Position3D_TC<PositionType_TP>& left_bottom,
        const Position3D_TC<PositionType_TP>& right_top,
        const Position3D_TC<PositionType_TP>& right_bottom)
        :
        _left_top(left_top),
        _left_bottom(left_bottom),
        _right_top(right_top),
        _right_bottom(right_bottom)
    {
        _width = _left_bottom._x - _right_bottom._x;
        _height = _left_bottom._y - _left_top._y;
    }

    BoundingBoxArea_TP(PositionType_TP screen_pos_x_S,
        PositionType_TP screen_pos_y_S,
        PositionType_TP width_S,
        PositionType_TP height_S)
        :
        _width(width_S),
        _height(height_S)

    {
        // calculate corner points of the bounding box
        _left_bottom._x = screen_pos_x_S;
        _left_bottom._y = screen_pos_y_S;
        _left_bottom._z = _z_pos;

        _left_top._x = screen_pos_x_S;
        _left_top._y = screen_pos_y_S + _height;
        _left_top._z = _z_pos;

        _right_bottom._x = screen_pos_x_S + _width;
        _right_bottom._y = screen_pos_y_S;
        _right_bottom._z = _z_pos;

        _right_top._x = screen_pos_x_S + _width;
        _right_top._y = screen_pos_y_S + _height;
        _right_top._z = _z_pos;
    }

    BoundingBoxArea_TP()
    {
    }


public:
    //! Returns the left top of the area in screen coordinates
    const Position3D_TC<PositionType_TP>& GetLeftTop() const { return _left_top; }

    //! Returns the left bottom of the area in screen coordinates
    const Position3D_TC<PositionType_TP>& GetLeftBottom() const { return _left_bottom; }

    //! Returns the right top of the area in screen coordinates
    const Position3D_TC<PositionType_TP>& GetRightTop() const { return _right_top; }

    //! Returns the right bottom of the area in screen coordinates
    const Position3D_TC<PositionType_TP>& GetRightBottom() const { return _right_bottom; }

    //! Returns the with of the area
    const PositionType_TP GetWidth() const { return _width; }

    //! Returns the height of the area
    const PositionType_TP GetHeight() const { return _height; }

    //! Returns the z position
    const PositionType_TP GetZPosition() const { return _left_top._z; }

public:
    Position3D_TC<PositionType_TP> _left_top;

    Position3D_TC<PositionType_TP> _left_bottom;

    Position3D_TC<PositionType_TP> _right_top;

    Position3D_TC<PositionType_TP> _right_bottom;

    PositionType_TP _width = 0;

    PositionType_TP _height = 0;

    PositionType_TP _z_pos = 0;
};
