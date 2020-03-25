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
                      QVector<float> y_axis_vertices)
        :
          _x_axis_vertices(x_axis_vertices),
          _y_axis_vertices(y_axis_vertices)
    {
    }

    QVector<float> _x_axis_vertices;
    QVector<float> _y_axis_vertices;
};

//! Description
//! Specialization of a point-plot (GL_POINTS)
//!
//! Does not save the data which is added to the series. If the buffer is filled, data is removed to make place for the new data
//! Uses a Vertex buffer objects in which new data is written in a round-robbin scheme:
//! Starts overwriting old data, beginning at the beginning of the buffer, when the buffer is filled
//! // Coordinate systems used (Big letters adress a coordinate system)
//! S - Screen coords
//! W - OpenGL world coords
class OGLChart
{

public:

	OGLChart(int bufferSize, int screenPosX_S, int screenPosY_S, int chartWidth_S,int  chartHeight_S, int chartId = 0);
    ~OGLChart();

    //! Appends new data value to the chart consisting of a x (ms) and y (no unit) component.
    //! If the chart vbo is full, old data is overwritten, starting at the beginning of the chart vbo.
    //! maps data to a plot point to a specific position in the plot itself (not the window the plot is placed in)
    //! modify data for visualization
    //!
	void addData(float y, float x_ms);
	void updateChart();
	void drawNewestData();

    void deleteDataFromBeginning(int);
	void Draw();
	void WriteSeriesToVBO();
	void addRange(int, QVector<double>);


    // Private helper functions
private:
    // Draws the chart-axes inside the opengl context
    void DrawXYAxes();

    const XYAxisVertices_TP CreateAxesVertices();

    // Creates and fills vertex buffer objects used for the axes of the chart
    void SetupAxes();


private:
	QList<double>* _dataBuffer;
	QVector<double>* _dataBufferdouble_X;
	QVector<double>* _dataBufferdouble_Y;
	std::mutex* data_lock;

    // Vertex buffer object for the chart-data
    QOpenGLBuffer _chart_vbo;

    // Vertex buffer objects for the x and y axis
    QOpenGLBuffer _x_axis_vbo;
    QOpenGLBuffer _y_axis_vbo;


    // x-position of the left top corner of the chart inside the ogl context in screen coordinates
    int screen_pos_x_S;

    // y-position of the left top corner of the chart inside the ogl context in screen coordinates
    int _screen_pos_y_S;

    int _width_S;
    int _height_S;

    int _vbo_series_idx;
	int _bufferSize;
    int _point_count;
	
	int _maxY;
	int _minY;

    int _min_x_val_ms;
    int _max_x_val_ms;

	//Input Buffer
	float* _data_series_buffer;	//save values before adding them so we can add them at once and not each point for itself( this means less calls to glbuffersubdata)
    // Is incremented each time new data was added via AddData(...)
    // Is resetted ( set to zero ) when the data is written into the charts vertex buffer object _chart_vbo
    int _series_buffer_idx;

    // Sweep chart parameters
    // counts how often the point series reached the left side of the screen and was wrapped to the left side again
    int _number_of_wraps;
    // Indicates if the chart needs to wrap
    bool _need_to_wrap_series;
    // Indicates if the dataseries was already wrapped one time from the right  to the left screen
    bool _dataseries_wrapped_once;
//
	int _chartId;
};


//this would be a waste of ressources..each plot means 10000 plotPoints and t
class plotPoint
{
public:

private:
	QOpenGLBuffer pointVBO;
};
