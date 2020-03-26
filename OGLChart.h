#pragma once

#include <iostream>
#include <string>
#include <mutex>

#include <qlist.h>
#include <qvector.h>
#include "qbuffer.h"
#include <qtimer.h>

#include <qopenglfunctions_3_3_core.h>
//#include <qopenglfunctions.h>
#include <qopenglbuffer.h>
#include <qopenglwidget.h>
//#include <qopenglfunctions_3_2_core.h>
#include <qopenglextrafunctions.h>
//#include <qopenglfunctions_3_1.h>
#include <qopenglshaderprogram.h>

#define TEMP_BUFFER_SIZE 10000


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

//! Description
//! Specialization of a OpenGl point-plot  (GL_POINTS) optimized for visualization of real time data
//!
//! Does not save the data which is added to the series. If the buffer is filled, data is removed to make place for the new data
//! Uses a Vertex buffer objects in which new data is written in a round-robbin scheme:
//! Starts overwriting old data, beginning at the beginning of the buffer, when the buffer is filled
//! // Coordinate systems used (Big letters adress a coordinate system)
//! S - Screen coords
//! W - OpenGL world coords
class OGLChart
{

    // Constructor / Destructor / Copying..
public:
    OGLChart(int buffer_size,
             int screen_pos_x_S,
             int screen_pos_y_S,
             int width_S,
             int height_S);

    ~OGLChart();

// Public access functions
public:
    //! Appends a new data value to the chart which consistss of a x-value(ms) and y-value(no unit) component.
    //! If the buffer of the chart is full, old data is overwritten, starting at the beginning of the buffer.
    //! This function maps data to a plot point which means data is mapped to a specific position in the plot itself (not the window the plot is placed in)
    //!
    void AddDataToSeries(float y, float x_ms);

    //! Draws the chart inside the opengl context from which this function is called
    void Draw();

    //! Binds and writes the data from the circular buffer to the vbo
    void WriteSeriesToVBO();

    void addRange(int, QVector<double>);


// Private helper functions
private:
    //! Draws the x- and y-axis inside the opengl context
    //! from which the function is called
    void DrawXYAxes();

    //! Craetes vertices used to draw the x and y axis
    //!
    //! \param size_S the size of the x and y axis.
    //!               In case of x axis this means the 'height'.
    //!               In case of the y axis this means the 'width'.
    //! \returns a struct containing the vertices for the x- and y-axis
    const XYAxisVertices_TP CreateAxesVertices(float size_S);

    //! Creates and fills vertex buffer objects used for the axes of the chart
    void SetupAxes();

// Private attributes
private:
    //! Vertex buffer object which contains the data series added by AddData..(..)
    QOpenGLBuffer _chart_vbo;

    //! Vertex buffer objects for the x and y axis
    QOpenGLBuffer _x_axis_vbo;
    QOpenGLBuffer _y_axis_vbo;

    //! x-position of the left top corner of the chart
    //! inside the ogl context in screen coordinates
    int _screen_pos_x_S;

    //! y-position of the left top corner of the chart
    //! inside the ogl context in screen coordinates
    int _screen_pos_y_S;

    //! width of the chart
    int _width_S;
    //! height of the chart
    int _height_S;

    int _vbo_series_idx;
    int _buffer_size;
    int _point_count = 0;
	
    int _max_y_axis_value;
    int _min_y_axis_value;

    int _min_x_axis_val_ms;
    int _max_x_axis_val_ms;

    // Input Buffer
    float* _data_series_buffer;
    //save values before adding them so we can add them at once and not each point for itself( this means less calls to glbuffersubdata)
    // Is incremented each time new data was added via AddData(...)
    // Is resetted ( set to zero ) when the data is written into the charts vertex buffer object _chart_vbo
    int _data_series_buffer_idx;

    // Sweep chart parameters
    // counts how often the point series reached the left side of the screen and was wrapped to the left side again
    int _number_of_wraps;
    // Indicates if the chart needs to wrap
    bool _need_to_wrap_series;
    // Indicates if the dataseries was already wrapped one time from the right  to the left screen
    bool _dataseries_wrapped_once;
};

// old func headers
//void updateChart();
//void drawNewestData();
//void deleteDataFromBeginning(int);
