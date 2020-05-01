#pragma once

// Project includes
#include <CircularBuffer.h>
//#include <ogl_chart_geometry_c.h>

// STL includes
#include <iostream>
#include <string>
#include <mutex>
#include <algorithm>
#include <vector>
#include <chrono>
#include <time.h>
#include <ctime>

// Qt includes
#include <qlist.h>
#include <qvector.h>
#include "qbuffer.h"
#include <qtimer.h>
#include <qopenglfunctions.h>
#include <qopenglbuffer.h>
#include <qopenglwidget.h>
#include <qopenglextrafunctions.h>
#include <qopenglshaderprogram.h>
#include <qpainter.h>
#include <qopenglwidget.h>
#include <qdatetime.h>

struct XYAxisVertices_TP
{
public:
    XYAxisVertices_TP(QVector<float> x_axis_vertices,
                      QVector<float> y_axis_vertices )
        :
          _x_axis_vertices(x_axis_vertices),
          _y_axis_vertices(y_axis_vertices)
    {
    }
    QVector<float> _x_axis_vertices;
    QVector<float> _y_axis_vertices;
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
        auto converted_time = ClockType::from_time_t( time_t(current_time_sec/* * 1000.0*/) ).time_since_epoch();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>( converted_time);
        _timestamp = std::chrono::time_point<ClockType>( duration);
    }

    Timestamp_TP(std::chrono::time_point<ClockType>& timestamp)
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
    void Now(){ _timestamp = ClockType::now(); }

    //! Get the timestamp in milliseconds
    size_t GetMilliseconds() const { return std::chrono::duration_cast<std::chrono::milliseconds>(_timestamp.time_since_epoch()).count(); }
    size_t GetNanoseconds(){ return std::chrono::duration_cast<std::chrono::nanoseconds>(_timestamp.time_since_epoch()).count(); }
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


//! Description
//! Specialization of a OpenGl point-plot  (GL_POINTS) optimized for visualization of real time data
//!
//! Does not save the data which is added to the series. If the buffer is filled, data is removed to make place for the new data
//! Uses a Vertex buffer objects in which new data is written in a round-robbin scheme:
//! Starts overwriting old data, beginning at the beginning of the buffer, when the buffer is filled
//!
//! Coordinate systems used (Big letters adress a coordinate system)
//! S - Screen coords
//! W - OpenGL world coords
//! Unit - world unit (millimeters)
//!
//! Chart coordinate system:
//!  (0, y) ^
//!         | 
//!  (0, 0) |---------------------> (x, 0)
//!         |
//!  (0,-y) |
//!
//
class OGLChart_C
{

    // Constructor / Destructor / Copying..
public:
    OGLChart_C(int buffer_size,
             int screen_pos_x_S,
             int screen_pos_y_S,
             int width_S,
             int height_S,
             const QOpenGLWidget& parent);

    ~OGLChart_C();

// Public access functions
public:
    //! Appends a new data value to the chart which consistss of a x-value(ms) and y-value(no unit) component.
    //! If the buffer of the chart is full, old data is overwritten, starting at the beginning of the buffer.
    //! This function maps data to a plot point which means data is mapped to a specific position in the plot itself (not the window the plot is placed in)
    //!
    void AddDataToSeries(float y, float x_ms);


    //! Writes NAN-data inside the vertex buffer to remove data
    //! older than the timerange, for the viewer.
    //! This function checks if the data inside the input array exceeds the time-range and
    //! disables visualization of outdated lines (which are out of timerange) via opengl 
    //! by replacing the data with NAN values ( only inside the vertex buffer, not the input ring-buffer )
    //! This function uses binary search ( std::upper_limit ) for fast searching. 
    //! Its possible to compare Timestamp_TP objects with ChartPoint_TP objects 
    //! because of an compare-function which compares the Timestamp_TP object with the Timestamp_TP object of the ChartPoint_TP
    //!
    //! \param timestamp the timestamp to search
    //! \param data the data array of ChartPoint_TP to look for.
    //! \returns on success, the offset to the timestamp value inside the ogl chart input ring-buffer. 
    //!     If nothing was found the function returns -1.
    int FindIdxToTimestampInsideData(const Timestamp_TP & timestamp, const std::vector<ChartPoint_TP<Position3D_TC<float>>>& data);

    // Todo template the chart class for different datatypes
    void AddDataTimestamp(float value, Timestamp_TP& timestamp);

    //! Draws the chart inside the opengl context from which this function is called
    void Draw();

    // Unuused, old function
    void addRange(int, QVector<double>);

    // Updates the vertex buffer from the source data inside the input ring-buffer
    void OnChartUpdate();

    //! Returns the y-screen coordinates of a given plot y-value
    float GetScreenCoordsFromYChartValue(float y_value);

    //! Returns the y-screen coordinates of a given plot x-value
    float GetScreenCoordsFromXChartValue(float x_value);

// Private helper functions
private:
    //! Draws the x- and y-axis inside the opengl context
    //! from which the function is called
    void DrawXYAxes();

    //! Draws the border bounding box of the plot area inside the opengl context
    void DrawBoundingBox();

    //! Draws the data series to the opengl context inside the plot-area
    void DrawSeries();
    
    //! Draws the surface grid
    void DrawSurfaceGrid();

    //! Draws the lead line
    void DrawLeadLine();

    //! Creates the vbo used to draw the bounding box of the chart
    void CreateBoundingBox();

    //! Creates a vbo used to draw the grid of the chart
    void CreateSurfaceGrid(int x_dist_unit, int y_dist_unit);

    //! Creates the vbo used to draw the lead line indicating the most current datapoint
    void CreateLeadLineVbo();

    //! Update the current position of the lead line. 
    //! Used to assign the last visualized point as lead-line position
    void UpdateLeadLinePosition(float x_value_new);


    //! Creates vertices used to draw the x and y axis
    //!
    //! \param size_S the size of the x and y axis.
    //!               In case of x axis this means the 'height'.
    //!               In case of the y axis this means the 'width'.
    //! \returns a struct containing the vertices for the x- and y-axis
    const XYAxisVertices_TP CreateAxesVertices(float size_S);

    //! Creates and allocates an empty OpenGL vertex buffer object used to store data for visualization
    void AllocateSeriesVbo();

    //! Write data to the vbo for visualization of data points
    //!
    //! \param data the data to write to the vbo
    void WriteToVbo(/*const*/ QVector<float>& data);

    //! Replaces data out of timerange inside the vertex buffer object with NAN-values to not visualize them.
    void RemoveOutdatedDataInsideVBO();

    //! Creates and fills vertex buffer objects used for the axes of the chart
    void SetupAxes();

// Private attributes
private:
    //! Vertex buffer object which contains the data series added by AddData..(..)
    QOpenGLBuffer _chart_vbo;

    //! Vertex buffer object for the x axis vertices
    QOpenGLBuffer _x_axis_vbo;

    //! Vertex buffer object for the y axis vertices
    QOpenGLBuffer _y_axis_vbo;

    //! Vertex buffer object for the bounding box
    QOpenGLBuffer _bb_vbo;

    //! Vertex buffer object for the background vertical and horizontal grid lines
    QOpenGLBuffer _surface_grid_vbo;

    //! Vertex buffer object for the lead line 
    QOpenGLBuffer _lead_line_vbo;
    
    //! Vertices for the lead line
    QVector<float> _lead_line_vertices;

    //! a vector of NAN-value vertices to replace data out of timerange.
    QVector<float> _no_line_vertices;

    //! Number of bytes for the lead line used by the vbo. 
    //! This should be equal to six, when the lead line is a line and no point or other shape.
    int _number_of_bytes_lead_line;

    //! x-position of the left top corner of the chart (the chart origin)
    //! inside the ogl context in screen coordinates
    int _screen_pos_x_S;

    //! y-position of the left top corner of the chart (the chart origin)
    //! inside the ogl context in screen coordinates
    int _screen_pos_y_S;

    //! z-position of the ...
    float _screen_pos_z_S = 1.0f;
    
    //! width of the chart in screen coordinates
    int _width_S;

    //! height of the chart in screen coordinates
    int _height_S;

    //! write position for the vbo
    int64_t _vbo_current_series_idx;

    //! remove position for the vbo
    int64_t _remove_series_idx = 0;

    //! size of the vbo
    int64_t _vbo_buffer_size;

    int _point_count = 0;
	
    //! The maximum value of the y axis 
    int _max_y_axis_value;

    //! The minimum value of the y axis 
    int _min_y_axis_value;

    //! The maximum value of the x axis 
    int _max_x_axis_val_ms;

    //! The minimum value of the x axis 
    int _min_x_axis_val_ms;

    //! Timerange of the x axis in milliseconds 
    //! (_max_x_axis_val_ms - _min_x_axis_val_ms)
    double _time_range_ms;

    //! Number of vertices used to draw the vertical surface grid lines
    int _num_of_surface_grid_vertices;

    //! Input buffer used to store user data
    InputBuffer_TC<float> _input_buffer;

    //RingBuffer_TC<Position3D_TC<float>> _input_buffer_new;
    RingBuffer_TC<ChartPoint_TP<Position3D_TC<float>>> _input_buffer_new;

    //! The y component of the last value plotted
    float _last_plotted_y_value_S = 0;

    //! The x component of the last value plotted
    float _last_plotted_x_value_S = 0;

    double _last_timestamp = 0.0;
    //! The parent widget with the opengl context
    const QOpenGLWidget& _parent_widget;

    // TODO: finish and refactor bounding geometry
    //OGLChartGeometry_C _geometry;

// Sweep chart parameters
    //! counts how often the point series reached the left side of the screen and was wrapped to the left side again
    int _number_of_wraps;

    //! Indicates if the chart needs to wrap
    bool _need_to_wrap_series;
    
    //! Indicates if the dataseries was already wrapped one time from the right to the left screen
    bool _dataseries_wrapped_once;
};
