#pragma once

// Project includes
#include "ogl_base_chart.h"
#include "circular_buffer.h"
#include "ogl_sweep_chart_buffer.h"
#include "ogl_lead_line_c.h"

// STL includes
#include <iostream>
#include <string>
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

enum WritingSpeed_TP {
    TwentyFive,
    Fivty,
    OneHundred,
    TwoHundred
};

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

    // Copy initialization
    OGLSweepChart_C(const OGLSweepChart_C&) = delete;
    
    // Copy assignment
    OGLSweepChart_C& operator=(const OGLSweepChart_C& other) = delete;

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
    void AddDatapoint(const DataType_TP value, const Timestamp_TP& timestamp);

    //! Draws the chart inside the opengl context from which this function is called
    //template<DrawingStyle_TP type = DrawingStyle_TP::LINE_SERIES >
    void Draw(QOpenGLShaderProgram& shader, QOpenGLShaderProgram& text_shader);

    void AddNewFiducialMark(const Timestamp_TP& timestamp);

    //! Clears the plot screen and buffers
    void Clear();

    //! Sets the chart type: 
    //! - Line chart(LINE_SERIES): 
    //!     connects datapoints with lines
    //! 
    //! - Point chart(POINT_SERIES):
    //!     draws each datapoint as a point itself
    //!
    //! \param chart_type LINE_SERIES for a line-strip-chart or 
    //!                   POINT_SERIES for a point chart
    //!                   LINES for single lines
    void SetChartType(DrawingStyle_TP chart_type);

    DrawingStyle_TP GetChartType();

    //! Set the major tick value for the x-axes.
    void SetMajorTickValueXAxes(float tick_value_ms) override;
    
    //! Set the minor tick value for the x-axes.
    void SetMinorTickValueXAxes(float tick_value_ms) override;
    
    //! Set the major tick value for the y axis
    //! The major tick value is used to draw the horizontal grid lines
    void SetMajorTickValueYAxes(float tick_value_unit) override;
    
    //! Set the minor tick value for the y axis
    //! The major tick value is used to draw the horizontal grid lines
    void SetMinorTickValueYAxes(float tick_value_unit) override;
    
    void SetMaxValueYAxes(const DataType_TP max_y_val) override;

    void SetMinValueYAxes(const DataType_TP min_y_val) override;

    void SetTimerangeMs(double time_range_ms) override;

    void SetTimerangeWritingSpeed(WritingSpeed_TP w_speed);

    //! Set the color of the lead line
    void SetLeadLineColor(const QVector3D& color);

    void SetGain(const float new_gain);

    RingBufferSize_TP GetInputBufferSize();
    // Private helper functions
private:
    //! Draws the data series to the opengl context inside the plot-area
    void DrawSeries(QOpenGLShaderProgram& shader);

    ////! Draws the lead line ==> TODO: Refactor lead line in class
    //void DrawLeadLine(QOpenGLShaderProgram& shader);

    ////! Creates the vbo used to draw the lead line indicating the most current datapoint
    //void CreateLeadLineVbo();

    ////! Update the current position of the lead line. 
    ////! Used to assign the last visualized point as lead-line position
    //void UpdateLeadLinePosition(DataType_TP x_value_new);

    void CreateAxesGrid();
    
        // Private attributes
private:
    //! Input buffer used to store the time series data
    RingBufferOptimized_TC<ChartPoint_TP<Position3D_TC<DataType_TP>>> _input_buffer;

    //! Buffer used to store fiducial markers
    RingBufferOptimized_TC<ChartPoint_TP<Position3D_TC<DataType_TP>>> _fiducial_buffer;

    //! Buffer to visualize the data series
    OGLSweepChartBuffer_C<DataType_TP> _ogl_data_series;

    //! Buffer to visualize fiducial markers
    OGLSweepChartBuffer_C<DataType_TP> _ogl_fiducial_data_series;

    OGLLeadLine_C _lead_line;

    std::atomic<float> _gain = 1.0f;

    bool _initialized = false;
};

#ifdef DEBUG_INFO
#define DEBUG(msg) std::cout << msg << std::endl;
#else
#define DEBUG(msg) do{} while(0)
#endif

template<typename DataType_TP>
OGLSweepChart_C<DataType_TP>::~OGLSweepChart_C()
{
}

template<typename DataType_TP>
OGLSweepChart_C<DataType_TP>::OGLSweepChart_C(int time_range_ms,
                                             RingBufferSize_TP buffer_size,
                                             DataType_TP max_y_value,
                                             DataType_TP min_y_value,
                                             const OGLChartGeometry_C& geometry,
                                             const QObject& parent)
    : OGLBaseChart_C(geometry, &parent),
    _input_buffer(buffer_size),
    _fiducial_buffer(RingBufferSize_TP::Size512),
    _ogl_data_series(_input_buffer.MaxSize(), time_range_ms, _input_buffer),
    _ogl_fiducial_data_series(_fiducial_buffer.MaxSize(), time_range_ms, _fiducial_buffer),
    _lead_line(_plot_area)
{

    _max_y_axis_value = max_y_value;
    _min_y_axis_value = min_y_value;
    _time_range_ms = time_range_ms;
 
}

template<typename DataType_TP>
void
OGLSweepChart_C<DataType_TP>::Initialize()
{
    if ( !_initialized ) {
        DEBUG("Initialize OGLChart");

        // Allocate a vertex buffer object to store data for visualization
        _ogl_data_series.AllocateSeriesVbo();
        _ogl_data_series.SetPrimitiveType(DrawingStyle_TP::LINE_SERIES);

        // Allocate vbo to store fiducial marks
        _ogl_fiducial_data_series.AllocateSeriesVbo();
        _ogl_fiducial_data_series.SetPrimitiveType(DrawingStyle_TP::LINES);

        CreateAxesGrid();
        
        SetLabel(_label);

        _initialized = true;
    }
}

template<typename DataType_TP>
void
OGLSweepChart_C<DataType_TP>::AddDatapoint(const DataType_TP value, 
                                          const Timestamp_TP & timestamp)
{
    // Don't add the value if its not inside the range, 
    // because its not visible eitherway -> better solution would be: Add it to the series but don't draw it!
    if ( value * _gain.load() > _max_y_axis_value || value * _gain.load() < _min_y_axis_value ) {
        return;
    }

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
            _plot_area.GetZPosition()),
        x_ms));

}

template<typename DataType_TP>
void
OGLSweepChart_C<DataType_TP>::SetChartType(DrawingStyle_TP chart_type)
{
    _ogl_data_series.SetPrimitiveType(chart_type);
}

template<typename DataType_TP>
inline 
DrawingStyle_TP 
OGLSweepChart_C<DataType_TP>::GetChartType()
{
    return _ogl_data_series.GetDrawingStyle();
}

template<typename DataType_TP>
void
OGLSweepChart_C<DataType_TP>::SetMajorTickValueXAxes(float tick_value_ms)
{
    _major_tick_x_axes = tick_value_ms;

    if ( _initialized ) {
        // Only set the timerange, if the major-tick-x-value is smaller than the new time_range_ms value, 
        // to prevent flase rendering of the surface grid.
        // OR
        // Set the timerange to major_tick_x_value, if its smaller.
        //if ( tick_value_ms <= _time_range_ms ) {
        //    tick_value_ms = _time_range_ms;
        //}
        Clear();
        CreateAxesGrid();
    }
}


template<typename DataType_TP>
void
OGLSweepChart_C<DataType_TP>::SetMajorTickValueYAxes(float tick_value_unit)
{
    _major_tick_y_axes = tick_value_unit;

    if ( _initialized ) {
        if ( tick_value_unit > _max_y_axis_value - _min_y_axis_value ) {
            tick_value_unit = _max_y_axis_value;
        }
        Clear();
        CreateAxesGrid();
    }
}

template<typename DataType_TP>
inline void OGLSweepChart_C<DataType_TP>::SetMinorTickValueYAxes(float tick_value_unit)
{
    _minor_tick_y_axes = tick_value_unit;
    if ( _initialized ) {
        Clear();
        CreateAxesGrid();
    }
}


template<typename DataType_TP>
inline
void
OGLSweepChart_C<DataType_TP>::SetMinorTickValueXAxes(float tick_value_ms)
{
    _minor_tick_x_axes = tick_value_ms;
    if ( _initialized ) {
        Clear();
        CreateAxesGrid();
    }
}

template<typename DataType_TP>
inline
void
OGLSweepChart_C<DataType_TP>::SetMaxValueYAxes(const DataType_TP max_y_val)
{
    _max_y_axis_value = max_y_val;
    if ( _initialized ) {
        Clear();
        CreateAxesGrid();
    }
}

template<typename DataType_TP>
inline 
void 
OGLSweepChart_C<DataType_TP>::SetMinValueYAxes(const DataType_TP min_y_val)
{
    _min_y_axis_value = min_y_val;

    if ( _initialized ) {
        Clear();
        CreateAxesGrid();
    }
}


template<typename DataType_TP>
inline
void
OGLSweepChart_C<DataType_TP>::SetTimerangeMs(double time_range_ms) // Two implementations: one with timerange, one with writing speed
{
    _time_range_ms = time_range_ms;
    _ogl_data_series.SetTimeRange(time_range_ms);
    _ogl_fiducial_data_series.SetTimeRange(time_range_ms);

    if ( _initialized ) {
        // Only set the timerange, if the major-tick-x-value is smaller than the new time_range_ms value, 
        // to prevent flase rendering of the surface grid.
        // OR
        // Set the timerange to major_tick_x_value, if its smaller.
        //if ( _major_tick_x_axes >= time_range_ms ) {
        //    time_range_ms = _major_tick_x_axes;
        //}
        Clear();
        
        CreateAxesGrid();
    }
    
}

template<typename DataType_TP>
inline 
void 
OGLSweepChart_C<DataType_TP>::SetTimerangeWritingSpeed(WritingSpeed_TP w_speed)
{
    int schreibgeschwindigkeit_mm_s = 25;
    switch ( w_speed ) {
        case WritingSpeed_TP::TwentyFive:
            schreibgeschwindigkeit_mm_s = 25;
            break;
        
        case WritingSpeed_TP::Fivty:
            schreibgeschwindigkeit_mm_s = 50;
            break;
        
        case WritingSpeed_TP::OneHundred:
            schreibgeschwindigkeit_mm_s = 100;
            break;

        case WritingSpeed_TP::TwoHundred:
            schreibgeschwindigkeit_mm_s = 200;
            break;
    }

    if ( _initialized ) {
        Clear();

        // convert from 'seconds_per_mm' to 'millisecs_per_mm'  
        float millisecs_per_mm = 1000.0 / static_cast<double>(schreibgeschwindigkeit_mm_s);

        QSizeF screenSize = QGuiApplication::primaryScreen()->physicalSize();
        float screen_width_mm = screenSize.width();
        //float screen_height_mm = screenSize.height();
        auto dpiX = qApp->desktop()->logicalDpiX();
        auto dpiX_mm = dpiX / 10.0; // from cm to mm
        auto dpiY = qApp->desktop()->logicalDpiY();
        auto dpiY_mm = dpiY / 10.0; // from cm to mm

        const float size_big_square_mm = 5;

        double size_big_square_pixel = size_big_square_mm * dpiY_mm; // should i use dpiX ? // the major tck value in pixel
        double size_big_square_ms = size_big_square_mm / schreibgeschwindigkeit_mm_s; //
        // Berechne timerange die mit der Schreibgeschwindigkeit möglich ist:
        // timerange should be in seconds here (use time_ms_in_one_mm for milliseconds
        _time_range_ms = millisecs_per_mm * screen_width_mm;
        // THis function is fine, but binary searching is a bit broken 
        // -> write test class for binary search ( make it a friend of the ogl_sweep_chart_buffer)
        _ogl_data_series.SetTimeRange(_time_range_ms);
        _ogl_fiducial_data_series.SetTimeRange(_time_range_ms);
        _minor_tick_x_axes = millisecs_per_mm * size_big_square_mm; // THis is wrong -> because at 100 there should be bigger ones, but less than at 25 ( now they are all teh same size always)
        //_minor_tick_x_axes = screen_width_mm / size_big_square_mm; // THis is wrong -> because at 100 there should be bigger ones, but less than at 25 ( now they are all teh same size always)

        _major_tick_x_axes = _time_range_ms / ((_time_range_ms / _minor_tick_x_axes) / 4); // create four text labels

        float mm_per_mv = 1.0; // This is input the user has to give us (Like schreibgeschwindigkeit_mm_s)
        // other vals: ;0.05 ;0.5 ;1 ; 2; 3; ..
        float screen_height_mm = screenSize.height();
        float min_max_y_mv = (screen_height_mm / mm_per_mv) / 2; // positive and negative 
        float max_y_mv = +min_max_y_mv;
        float min_y_mv = -min_max_y_mv;
        const float size_small_square_mv = 0.1;
        const float size_big_square_mv = size_small_square_mv * 5;

        double size_big_y_square_mm = size_big_square_mv / mm_per_mv ;
        double size_big_square_unit = size_big_y_square_mm / mm_per_mv;
        // Set y minor and major tick val
        //_minor_tick_y_axes = size_big_square_mm * mm_per_mv;
        // Set major tick value for text labels
        //_major_tick_y_axes = (max_y - min_y) / ( ( (max_y - min_y) / _minor_tick_y_axes ) / 4);
        CreateAxesGrid();
    }

}

template<typename DataType_TP>
void
OGLSweepChart_C<DataType_TP>::SetLeadLineColor(const QVector3D& color)
{
    _lead_line.SetColor(color);
}

template<typename DataType_TP>
void
OGLSweepChart_C<DataType_TP>::SetGain(const float new_gain) {
    _gain.store(new_gain);
}

template<typename DataType_TP>
inline 
RingBufferSize_TP 
OGLSweepChart_C<DataType_TP>::GetInputBufferSize()
{
    return _input_buffer.GetRingBufferSizeTP();
}

template<typename DataType_TP>
void
OGLSweepChart_C<DataType_TP>::Draw(QOpenGLShaderProgram& shader,
    QOpenGLShaderProgram& text_shader)
{
    DrawSeries(shader);
    // DrawBoundingBox(shader);
    _lead_line.DrawLeadLine(shader, _ogl_data_series.GetLastPlottedXValue());
    DrawSurfaceGrid(shader);
    DrawTextLabels(shader, text_shader);
}

template<typename DataType_TP>
inline
void
OGLSweepChart_C<DataType_TP>::AddNewFiducialMark(const Timestamp_TP& timestamp_sec)
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
        _plot_area.GetZPosition()),
        x_ms));

    // TO
    _fiducial_buffer.InsertAtTail(ChartPoint_TP<Position3D_TC<DataType_TP>>(Position3D_TC<DataType_TP>(x_pos_S,
        _plot_area.GetLeftTop()._y,
        _plot_area.GetZPosition()),
        x_ms));
}

template<typename DataType_TP>
inline
void
OGLSweepChart_C<DataType_TP>::Clear()
{
    if ( _initialized ) {
        _ogl_data_series.Clear();
        _ogl_fiducial_data_series.Clear();
        //UpdateLeadLinePosition(0.0);// replace with _lead_line.UpdatePOsition(0.0);
    }
}


template<typename DataType_TP>
inline
void
OGLSweepChart_C<DataType_TP>::CreateAxesGrid()
{
    // Create vertices for the current values -> min/max(x/y) AND major/minor(x/y) tick values 
    auto grid_vertices_maj_tick = CreateSurfaceGridVertices();
    auto& horizontal_verts_maj_tick = grid_vertices_maj_tick.first;
    auto& vertical_verts_maj_tick = grid_vertices_maj_tick.second;

    // Scale of the text
    double scale = 0.25f;
    // Use the surface grid vertice to create the text labels for the axes 
    InitializeAxesDescription(horizontal_verts_maj_tick,
                              vertical_verts_maj_tick,
                              scale);
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

    // Todo: Create fiducial manager which holds the color(and the _ogl_sweep_chart_buffer)?
    shader.setUniformValue("u_object_color", _fiducial_marks_color);
    _ogl_fiducial_data_series.Draw();
}
