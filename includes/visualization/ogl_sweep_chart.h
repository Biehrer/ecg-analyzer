#pragma once

// Project includes
#include "ogl_base_chart.h"
#include "circular_buffer.h"
#include "ogl_chart_geometry_c.h"
#include "chart_shapes_c.h"
#include "chart_types.h"
#include "ogl_sweep_chart_buffer.h"
#include "text_renderer_2d.h"

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

//! Description
//! Specialization of a OpenGl point-plot  (GL_POINTS) optimized for visualization of real time data
//!
//! Does not save the data which is added to the series. If the buffer is filled, data is removed to make place for the new data
//! Uses a Vertex buffer objects in which new data is written in a round-robbin scheme:
//! Starts overwriting old data, beginning at the beginning of the buffer, when the buffer is filled
//!
//! Coordinate systems used (Big letters adress a coordinate system)
//! _S --- e.g: some_var_S = some value in Screen coords
//!
//! See OGLChartGeometry for the chart coordinate system
class OGLSweepChart_C : public OGLBaseChart_C
{
    // Constructor / Destructor / Copying..
public:

    OGLSweepChart_C(int time_range_ms,
                    RingBufferSize_TP buffer_size,
                    float max_y_Value, 
                    float min_y_value,
                    const OGLChartGeometry_C& geometry,
                    const QOpenGLWidget& parent);

    ~OGLSweepChart_C();

// Public access functions
public:
    //! TODO: ..
    void Initialize();

    //! Appends a new data value to the chart which consistss of a x-value(ms) and y-value(no unit) component.
    //! If the buffer of the chart is full, old data is overwritten, starting at the beginning of the buffer.
    //! This function maps data to a plot point which means data is mapped to a specific position in the plot itself (not the window the plot is placed in)
    //!
    // Todo template the chart class for different datatypes
    void AddDataTimestamp(const float value, const Timestamp_TP& timestamp);

    //! Draws the chart inside the opengl context from which this function is called
    //template<DrawingStyle_TP type = DrawingStyle_TP::LINE_SERIES >
    void Draw( QOpenGLShaderProgram& shader, QOpenGLShaderProgram& text_shader);

    //! Returns the y-screen coordinates of a given plot y-value
    virtual float GetScreenCoordsFromYChartValue(float y_value);

    //! Returns the y-screen coordinates of a given plot x-value
    virtual float GetScreenCoordsFromXChartValue(float x_value);

    //! Sets the chart type: 
    //! - Line chart(LINE_SERIES): 
    //!     connects datapoints with lines
    //! 
    //! - Point chart(POINT_SERIES):
    //!     draws each datapoint as a point itself
    //!
    //! \param chart_type LINE_SERIES for a line chart or 
    //!                   POINT_SERIES for a point chart
    void SetChartType(DrawingStyle_TP chart_type);

    //! Set the major tick value for the x-axes.
    void SetMajorTickValueXAxes(float tick_value_ms);

    //! Set the major tick value for the y axis
    //! The major tick value is used to draw the horizontal grid lines
    void SetMajorTickValueYAxes(float tick_value_unit);

    ////! Set the color of the x- and y-axes
    //void SetAxesColor(const QVector3D& color);
    //
    ////! Set color for text rendering (e.g axes units)
    //void SetTextColor(const QVector3D & color);

    ////! Set the data series color
    //void SetSeriesColor(const QVector3D& color);
    //
    ////! Set the bounding box color
    //void SetBoundingBoxColor(const QVector3D& color);
    //
    ////! Set the color of the surface grid
    //void SetSurfaceGridColor(const QVector3D& color);
    //
    //! Set the color of the lead line
    void SetLeadLineColor(const QVector3D& color);

    //! Sets the model view projection matrix used for text rendering (e.g. axes units)
    //void SetModelViewProjection(QMatrix4x4 model_view_projection);


// Private helper functions
private:

    //! Create the text for displaying axes units
    /*void InitializeAxesDescription(const QVector<float>& horizontal_grid_vertices, 
                                   const QVector<float>& vertical_grid_vertices, 
                                   float scale);
*/
    //! Draws the x- and y-axis inside the opengl context
    //! from which the function is called
    //void DrawXYAxes(QOpenGLShaderProgram& shader, QOpenGLShaderProgram& text_shader);

    //! Draws the border bounding box of the plot area inside the opengl context
    //void DrawBoundingBox(QOpenGLShaderProgram& shader);

    //! Draws the data series to the opengl context inside the plot-area
    void DrawSeries(QOpenGLShaderProgram& shader);
    
    //! Draws the surface grid
    //void DrawSurfaceGrid(QOpenGLShaderProgram& shader);

    //! Draws the lead line
    void DrawLeadLine(QOpenGLShaderProgram& shader);

    //! Creates the vbo used to draw the bounding box of the chart
    //void CreateBoundingBox();

    //! Creates a vbo used to draw the grid of the chart
    //std::pair<QVector<float>, QVector<float>> CreateSurfaceGrid(int x_dist_unit, int y_dist_unit);

    //! Creates the vbo used to draw the lead line indicating the most current datapoint
    void CreateLeadLineVbo();

    //! Creates and fills vertex buffer objects used for the axes of the chart
    //void SetupAxes();

    //! Update the current position of the lead line. 
    //! Used to assign the last visualized point as lead-line position
    void UpdateLeadLinePosition(float x_value_new);

// Private attributes
private:

    //! Vertex buffer object for the lead line 
    QOpenGLBuffer _lead_line_vbo;
    
    //! Vertices for the lead line
    QVector<float> _lead_line_vertices;

    //! Number of bytes for the lead line used by the vbo. 
    //! This should be equal to six, when the lead line is a line and no point or other shape.
    int _number_of_bytes_lead_line;
	
    //! The maximum value of the y axis 
    int _max_y_axis_value;

    //! The minimum value of the y axis 
    int _min_y_axis_value;

    //! Timerange of the x axis in milliseconds 
    //! (_max_x_axis_val_ms - _min_x_axis_val_ms)
    double _time_range_ms;

    //! Input buffer used to store user data
    RingBuffer_TC<ChartPoint_TP<Position3D_TC<float>>> _input_buffer;

    //! Buffer for visualization - the user does not know this one
    OGLSweepChartBuffer_C _ogl_data_series;

    //! The y component of the last value plotted
    float _last_plotted_y_value_S = 0;

    //! The x component of the last value plotted
    float _last_plotted_x_value_S = 0;

    // Colors for the shader
    QVector3D _lead_line_color;

    //! modifies the surface grid
    float _major_tick_x_axes;

    //! modifies the surface grid
    float _major_tick_y_axes;

    //! unit descriptions for the x and y axes
    std::vector<OGLTextBox> _plot_axes;

    //! Model view projection transform matrix for text rendering
    QMatrix4x4 _chart_mvp;

 };
