#pragma once

// Project includes
#include <CircularBuffer.h>

// STL includes
#include <iostream>
#include <string>
#include <mutex>

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

struct Timestamp_TP {

public:

    Timestamp_TP() 
        : _timestamp(std::chrono::high_resolution_clock::now())
    {
    }

    void Now(){ _timestamp = std::chrono::high_resolution_clock::now(); }
    size_t GetMilliseconds() { return std::chrono::duration_cast<std::chrono::milliseconds>(_timestamp.time_since_epoch()).count(); }
    size_t GetNanoseconds(){ return std::chrono::duration_cast<std::chrono::nanoseconds>(_timestamp.time_since_epoch()).count(); }
    size_t GetSeconds() { return std::chrono::duration_cast<std::chrono::seconds>(_timestamp.time_since_epoch()).count(); }

    std::chrono::time_point<std::chrono::high_resolution_clock>* GetTimestampPtr() { return &_timestamp; }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _timestamp;
};

template<typename Datatype_TP>
struct ChartPoint_TP {

    ChartPoint_TP(Datatype_TP value, const Timestamp_TP& timestamp) 
        : _value(value),
        _timestamp(timestamp)
    {
    }

// Timestamp_TP _timestamp;
    Datatype_TP _value;
    Timestamp_TP _timestamp;
};

//! Description
//! Specialization of a OpenGl point-plot  (GL_POINTS) optimized for visualization of real time data
//!
//! Does not save the data which is added to the series. If the buffer is filled, data is removed to make place for the new data
//! Uses a Vertex buffer objects in which new data is written in a round-robbin scheme:
//! Starts overwriting old data, beginning at the beginning of the buffer, when the buffer is filled
//! // Coordinate systems used (Big letters adress a coordinate system)
//! S - Screen coords
//! W - OpenGL world coords
class OGLChart_C
{

    // Constructor / Destructor / Copying..
public:
    OGLChart_C(int buffer_size,
             int screen_pos_x_S,
             int screen_pos_y_S,
             int width_S,
             int height_S,
             QOpenGLWidget& parent);

    ~OGLChart_C();

// Public access functions
public:
    //! Appends a new data value to the chart which consistss of a x-value(ms) and y-value(no unit) component.
    //! If the buffer of the chart is full, old data is overwritten, starting at the beginning of the buffer.
    //! This function maps data to a plot point which means data is mapped to a specific position in the plot itself (not the window the plot is placed in)
    //!
    void AddDataToSeries(float y, float x_ms);

    // Todo template the chart class for different datatypes
    void AddData(float value, Timestamp_TP& timestamp);

    //! Draws the chart inside the opengl context from which this function is called
    void Draw();

    void addRange(int, QVector<double>);

    //! Binds and writes the data from the circular buffer to the vbo
    void UpdateVbo();

    //! Returns the screen coordinates of a given plot y-value
    float GetScreenCoordsFromYChartValue(float y_value);

    //! Returns the screen coordinates of a given plot x-value
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

    void CreateLeadLineVbo();

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
    //! \param data the data to write to the vbo
    void WriteToVbo(/*const*/ QVector<float>& data);


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

    //! Number of bytes for the lead line used by the vbo. 
    //! This should be equal to six, when the lead line is a line and no point or other shape.
    int _number_of_bytes_lead_line;

    //! QPainter object for simple rendering of text inside the ogl context
    QPainter* _text_painter;

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
    int64_t _vbo_series_idx;

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

    //! Number of vertices used to draw the vertical surface grid lines
    int _num_of_surface_grid_vertices;

    //! Input buffer used to store user data
    CircularBuffer_TC<float> _input_buffer;

    //! The y component of the last value plotted
    float _last_plotted_y_value_S = 0;

    //! The x component of the last value plotted
    float _last_plotted_x_value_S = 0;

    double _last_timestamp = 0.0;
    //! The parent widget with the opengl context
    QOpenGLWidget& _parent_widget;

// Sweep chart parameters
    //! counts how often the point series reached the left side of the screen and was wrapped to the left side again
    int _number_of_wraps;

    //! Indicates if the chart needs to wrap
    bool _need_to_wrap_series;
    
    //! Indicates if the dataseries was already wrapped one time from the right to the left screen
    bool _dataseries_wrapped_once;
};

// old func headers
//void updateChart();
//void drawNewestData();
//void deleteDataFromBeginning(int);
