#pragma once

// Project includes
#include "ogl_base_chart.h"
#include "circular_buffer.h"
#include "ogl_sweep_chart_buffer.h"

// STL includes
#include <iostream>
#include <string>
#include <mutex>
#include <algorithm>
#include <vector>
#include <chrono>
#include <time.h>
#include <ctime>
#include <atomic>

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

//! 
//! Class usage:
//! 
//! // Create the chart
//! OGLSweepChart_C chart(...);
//! // Setup the chart
//! chart.SetMajorTickValue(..)
//! chart.SetLabel("some_chart");
//! ...
//! // after setup call Initialize
//! chart.Initialize();
//! // now you can use the .Draw() method inside the QOpenGLWidget paintgl() method
//! 
//! void paintGL() 
//! {
//!    clearColor(r, g, b, a)
//!     ...
//!     chart.Draw(flat_color_shader, text_shader)
//!     ...
//! }
//

template <typename DataType_TP>
class OGLSweepChart_C : public OGLBaseChart_C
{
    // Constructor / Destructor / Copying..
public:

    OGLSweepChart_C(int time_range_ms,
                    RingBufferSize_TP buffer_size,
                    DataType_TP max_y_Value,
                    DataType_TP min_y_value,
                    const OGLChartGeometry_C& geometry,
                    const QObject& parent);

    ~OGLSweepChart_C();

// Public access functions
public:
    //! Initializes the charts internal objects. 
    //! Must be called before the chart can be draw.
    void Initialize();

    //! Appends a new data value to the chart which consistss of a x-value(ms) and y-value(no unit) component.
    //! If the buffer of the chart is full, old data is overwritten, starting at the beginning of the buffer.
    //! This function maps data to a plot point which means data is mapped to a specific position in the plot itself (not the window the plot is placed in)
    //!
    // Todo template the chart class for different datatypes
    void AddDatapoint(const DataType_TP value, const Timestamp_TP& timestamp);

    //! Draws the chart inside the opengl context from which this function is called
    //template<DrawingStyle_TP type = DrawingStyle_TP::LINE_SERIES >
    void Draw( QOpenGLShaderProgram& shader, QOpenGLShaderProgram& text_shader);

    void AddNewFiducialMark(/*const*/ Timestamp_TP/*double*//*&*/ timestamp);

    //! Returns the y-screen coordinates of a given plot y-value
    DataType_TP GetScreenCoordsFromYChartValue(DataType_TP y_value);

    //! Returns the y-screen coordinates of a given plot x-value
    DataType_TP GetScreenCoordsFromXChartValue(DataType_TP x_value);

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
    float GetMajorTickValueXAxes();

    //! Set the major tick value for the y axis
    //! The major tick value is used to draw the horizontal grid lines
    void SetMajorTickValueYAxes(float tick_value_unit);
    float GetMajorTickValueYAxes();

    void SetMaxValueYAxes(const DataType_TP max_y_val);
    DataType_TP GetMaxValueYAxes();

    void SetMinValueYAxes(const DataType_TP min_y_val);
    DataType_TP GetMinValueYAxes();

    void SetTimerangeMs(double time_range_ms);
    double GetTimerangeMs();

    //! Set the color of the lead line
    void SetLeadLineColor(const QVector3D& color);

    void SetGain(const float new_gain);

// Private helper functions
private:
    //! Draws the data series to the opengl context inside the plot-area
    void DrawSeries(QOpenGLShaderProgram& shader);
    
    //! Draws the lead line
    void DrawLeadLine(QOpenGLShaderProgram& shader);

    //! Creates the vbo used to draw the lead line indicating the most current datapoint
    void CreateLeadLineVbo();

    //! Update the current position of the lead line. 
    //! Used to assign the last visualized point as lead-line position
    void UpdateLeadLinePosition(DataType_TP x_value_new);

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
    /*int*/DataType_TP _max_y_axis_value;

    //! The minimum value of the y axis 
    /*int*/DataType_TP _min_y_axis_value;

    //! Timerange of the x axis in milliseconds 
    //! (_max_x_axis_val_ms - _min_x_axis_val_ms)
    double _time_range_ms;

    //! Input buffer used to store the time series data
    RingBufferOptimized_TC<ChartPoint_TP<Position3D_TC<DataType_TP>>> _input_buffer;

    //! Buffer used to store fiducial markers
    RingBufferOptimized_TC<ChartPoint_TP<Position3D_TC<DataType_TP>>> _fiducial_buffer;

    //! Buffer to visualize the data series
    OGLSweepChartBuffer_C<DataType_TP> _ogl_data_series;

    //! Buffer to visualize fiducial markers
    OGLSweepChartBuffer_C<DataType_TP> _ogl_fiducial_data_series;

    //! The y component of the last value plotted
    DataType_TP _last_plotted_y_value_S = 0;

    //! The x component of the last value plotted
    DataType_TP _last_plotted_x_value_S = 0;

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

    std::atomic<float> _gain = 1.0f;
 };

#ifdef DEBUG_INFO
#define DEBUG(msg) std::cout << msg << std::endl;
#else
#define DEBUG(msg) do{} while(0)
#endif

 template<typename DataType_TP>
 OGLSweepChart_C<DataType_TP>::~OGLSweepChart_C()
 {
     _lead_line_vbo.destroy();
 }

 template<typename DataType_TP>
 OGLSweepChart_C<DataType_TP>::OGLSweepChart_C(int time_range_ms,
     RingBufferSize_TP buffer_size,
     DataType_TP max_y_value,
     DataType_TP min_y_value,
     const OGLChartGeometry_C& geometry,
     const QObject& parent)
     : OGLBaseChart_C(geometry, parent),
     _lead_line_vbo(QOpenGLBuffer::VertexBuffer),
     _time_range_ms(time_range_ms),
     _input_buffer(buffer_size),
     _fiducial_buffer(RingBufferSize_TP::Size512), // Are 512 lines enough?
     _ogl_data_series(_input_buffer.MaxSize(), time_range_ms, _input_buffer),
     _ogl_fiducial_data_series(_fiducial_buffer.MaxSize() , time_range_ms, _fiducial_buffer)
 {
     _max_y_axis_value = max_y_value;
     _min_y_axis_value = min_y_value;
 }

 template<typename DataType_TP>
 void
     OGLSweepChart_C<DataType_TP>::Initialize()
 {
     DEBUG("Initialize OGLChart");

     // Allocate a vertex buffer object to store data for visualization
     _ogl_data_series.AllocateSeriesVbo();

     // Allocate vbo to store fiducial marks
     _ogl_fiducial_data_series.AllocateSeriesVbo();
     _ogl_fiducial_data_series.SetPrimitiveType(DrawingStyle_TP::LINES);

     // Allocate a vertex buffer object to store data for the lead line
     CreateLeadLineVbo();

     // Create vbo for the x and y axis
     SetupAxes();

     // Create vbo for the bounding box of the chart
     CreateBoundingBox();

     // Create surface grid vbo
     auto grid_vertices = CreateSurfaceGrid(_major_tick_x_axes,
         _major_tick_y_axes,
         _time_range_ms,
         _max_y_axis_value,
         _min_y_axis_value);

     auto& horizontal_verts = grid_vertices.first;
     auto& vertical_verts = grid_vertices.second;
     double scale = 0.25f;
     // Use the surface grid vertice
     InitializeAxesDescription(horizontal_verts,
         vertical_verts,
         scale,
         _time_range_ms,
         _max_y_axis_value,
         _major_tick_x_axes,
         _major_tick_y_axes);
 }

 template<typename DataType_TP>
 void
     OGLSweepChart_C<DataType_TP>::AddDatapoint(const DataType_TP value, const Timestamp_TP & timestamp)
 {
     // Don't add the value if its not inside the range, 
     // because its not visible eitherway -> better solution would be: Add it to the series but don't draw it!
     if ( value * _gain.load() > _max_y_axis_value || value * _gain.load() < _min_y_axis_value ) {
         return;
     }
     // Attention: the bounding box is not templated and uses a using definition for the datatype !!

     // - (minus) because then the positive y axis is directing at the top of the screen
     float y_val_scaled_S = _plot_area.GetLeftTop()._y -
         (((value * _gain.load()) - _min_y_axis_value) / (_max_y_axis_value - _min_y_axis_value)) *
         _plot_area.GetChartHeight();

     auto x_ms = timestamp.GetMilliseconds();
     // use modulo to wrap the dataseries
     float x_val_scaled_S = _plot_area.GetLeftBottom()._x +
         ((x_ms % static_cast<int>(_time_range_ms)) / _time_range_ms) *
         _plot_area.GetChartWidth();

     DEBUG("Scaled x value: " << x_val_scaled_S << ", to value: " << x_ms_modulo);
     DEBUG("Scaled y value: " << y_val_scaled_S << ", to value: " << value);
     _input_buffer.InsertAtTail(
         ChartPoint_TP<Position3D_TC<DataType_TP>>(Position3D_TC<DataType_TP>(x_val_scaled_S,
             y_val_scaled_S,
             1.0f),
             x_ms));

 }

 //! Uses the bounding box
 template<typename DataType_TP>
 DataType_TP
     OGLSweepChart_C<DataType_TP>::GetScreenCoordsFromYChartValue(DataType_TP y_value)
 {
     return static_cast<float>(_bounding_box.GetLeftTop()._y) -
         ((y_value - _min_y_axis_value) / (_max_y_axis_value - _min_y_axis_value)) *
         _bounding_box.GetChartHeight();

 }


 template<typename DataType_TP>
 DataType_TP
     OGLSweepChart_C<DataType_TP>::GetScreenCoordsFromXChartValue(DataType_TP x_value_ms)
 {
     // calculate x-value after wrapping
     return static_cast<DataType_TP>(_bounding_box.GetLeftBottom()._x) +
         ((x_value_ms) / (_time_range_ms)) *
         _bounding_box.GetChartWidth();

 }

 template<typename DataType_TP>
 void
     OGLSweepChart_C<DataType_TP>::SetChartType(DrawingStyle_TP chart_type)
 {
     _ogl_data_series.SetPrimitiveType(chart_type);
 }

 template<typename DataType_TP>
 void
     OGLSweepChart_C<DataType_TP>::SetMajorTickValueXAxes(float tick_value_ms)
 {
     _major_tick_x_axes = tick_value_ms;
 }

 template<typename DataType_TP>
 inline float OGLSweepChart_C<DataType_TP>::GetMajorTickValueXAxes()
 {
     return _major_tick_x_axes;
 }

 template<typename DataType_TP>
 void
     OGLSweepChart_C<DataType_TP>::SetMajorTickValueYAxes(float tick_value_unit)
 {
     _major_tick_y_axes = tick_value_unit;
 }

 template<typename DataType_TP>
 inline float OGLSweepChart_C<DataType_TP>::GetMajorTickValueYAxes()
 {
     return _major_tick_y_axes;
 }

 template<typename DataType_TP>
 inline void OGLSweepChart_C<DataType_TP>::SetMaxValueYAxes(const DataType_TP max_y_val)
 {
     _max_y_axis_value = max_y_val;
 }

 template<typename DataType_TP>
 inline DataType_TP OGLSweepChart_C<DataType_TP>::GetMaxValueYAxes()
 {
     return _max_y_axis_value;
 }

 template<typename DataType_TP>
 inline void OGLSweepChart_C<DataType_TP>::SetMinValueYAxes(const DataType_TP min_y_val)
 {
     _min_y_axis_value = min_y_val;
 }

 template<typename DataType_TP>
 inline DataType_TP OGLSweepChart_C<DataType_TP>::GetMinValueYAxes()
 {
     return _min_y_axis_value;
 }

 template<typename DataType_TP>
 inline 
 void 
 OGLSweepChart_C<DataType_TP>::SetTimerangeMs(double time_range_ms)
 {
     _time_range_ms = time_range_ms;
 }

 template<typename DataType_TP>
 inline 
double OGLSweepChart_C<DataType_TP>::GetTimerangeMs()
 {
     return _time_range_ms;
 }

 template<typename DataType_TP>
 void
OGLSweepChart_C<DataType_TP>::SetLeadLineColor(const QVector3D& color)
 {
     _lead_line_color = color;
 }

 template<typename DataType_TP>
 //inline 
 void
 OGLSweepChart_C<DataType_TP>::SetGain(const float new_gain) {
     _gain.store(new_gain);
 }

 template<typename DataType_TP>
 //inline
 void
 OGLSweepChart_C<DataType_TP>::Draw(QOpenGLShaderProgram& shader,
                                    QOpenGLShaderProgram& text_shader)
 {
     DrawSeries(shader);
     //DrawBoundingBox(shader);
     DrawLeadLine(shader);
     DrawSurfaceGrid(shader);
     DrawXYAxes(shader, text_shader);
 }

template<typename DataType_TP>
inline 
void 
 OGLSweepChart_C<DataType_TP>::AddNewFiducialMark(/*const*/ Timestamp_TP/*double*//*&*/ timestamp_sec)
 {
     auto x_ms = timestamp_sec.GetMilliseconds();
     // use modulo to wrap the dataseries 
     float x_pos_S = _plot_area.GetLeftBottom()._x +
         ((x_ms % static_cast<int>(_time_range_ms)) / _time_range_ms) *
         _plot_area.GetChartWidth();

     // => Do all this stuff inside the fiducial marker manager?(the manager holds the _ogl_data_series_buffer)
     // FROM
     _fiducial_buffer.InsertAtTail(ChartPoint_TP<Position3D_TC<DataType_TP>>(Position3D_TC<DataType_TP>(x_pos_S,
                                     _plot_area.GetLeftBottom()._y,
                                     1.0f),
                                     x_ms));

     // TO
     _fiducial_buffer.InsertAtTail(ChartPoint_TP<Position3D_TC<DataType_TP>>(Position3D_TC<DataType_TP>(x_pos_S,
                                   _plot_area.GetLeftTop()._y,
                                   1.0f),
                                   x_ms));
 }

 template<typename DataType_TP>
 void
     OGLSweepChart_C<DataType_TP>::CreateLeadLineVbo()
 {
     int buffer_size = 2 * 3;
     _number_of_bytes_lead_line = buffer_size * sizeof(float);

     QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
     // Setup OGL-Chart buffer - empty
     _lead_line_vbo.create();
     _lead_line_vbo.bind();
     f->glEnableVertexAttribArray(0);
     // 3 coordinates make one point  (x, y, z)
     f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
     // the lead line consists of one line -> 2 points, each 3 vertices of the type float
     _lead_line_vbo.allocate(nullptr, buffer_size * sizeof(float));
     _lead_line_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
     f->glDisableVertexAttribArray(0);
     _lead_line_vbo.release();

     // setup the buffer: The most vertices are fixed and only the x value of the vertices needs to be adapted when new data was added.
     // The only value that changes when adding new data, is the x value of the lead line. 
     // The y values are fixed because the lead line is a vertical line, which is always drawn through the whole chart.
     // The z values are fixed anyway
     _lead_line_vertices.resize(buffer_size);
     // point from:
     // y value
     _lead_line_vertices[1] = _plot_area.GetLeftBottom()._y;
     // z value
     _lead_line_vertices[2] = _plot_area.GetZPosition();
     // point to:
     // y value
     _lead_line_vertices[4] = _plot_area.GetLeftTop()._y;
     // z value
     _lead_line_vertices[5] = _plot_area.GetZPosition();
 }

 template<typename DataType_TP>
 void
     OGLSweepChart_C<DataType_TP>::UpdateLeadLinePosition(DataType_TP x_value_new)
 {
     _lead_line_vertices[0] = x_value_new;
     _lead_line_vertices[3] = x_value_new;
     // Overwrite the whole vbo with the new data
     // alternative: write just two values, but because of caching this should not really result in differences..
     _lead_line_vbo.write(0, _lead_line_vertices.constData(), _number_of_bytes_lead_line);
 }

 template<typename DataType_TP>
 inline
     void
     OGLSweepChart_C<DataType_TP>::DrawLeadLine(QOpenGLShaderProgram& shader)
 {
     QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
     shader.bind();
     shader.setUniformValue("u_object_color", _lead_line_color);
     _lead_line_vbo.bind();

     UpdateLeadLinePosition(_ogl_data_series.GetLastPlottedXValue());

     f->glEnableVertexAttribArray(0);
     f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
     f->glDrawArrays(GL_LINES, 0, 2);
     f->glDisableVertexAttribArray(0);
     _lead_line_vbo.release();
 }

 template<typename DataType_TP>
 inline
     void
     OGLSweepChart_C<DataType_TP>::DrawSeries(QOpenGLShaderProgram& shader)
 {
     auto* f = QOpenGLContext::currentContext()->functions();
     shader.bind();
     shader.setUniformValue("u_object_color", _series_color);
     _ogl_data_series.Draw();

     // Todo: Change _series color to: _fiducial_mark_color? ->Create fiducial manager which holds the color(and the _ogl_sweep_chart_buffer)
      shader.setUniformValue("u_object_color", _fiducial_marks_color);
     _ogl_fiducial_data_series.Draw();
 }
