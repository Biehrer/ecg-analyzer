#include "ogl_sweep_chart.h"

//#define DEBUG_INFO

#ifdef DEBUG_INFO
    #define DEBUG(msg) std::cout << msg << std::endl;
#else
    #define DEBUG(msg) do{} while(0)
#endif


OGLSweepChart_C::~OGLSweepChart_C()
{
    _y_axis_vbo.destroy();
    _x_axis_vbo.destroy();
    _bb_vbo.destroy();
    _surface_grid_vbo.destroy();
}


OGLSweepChart_C::OGLSweepChart_C(int time_range_ms,
                       RingBufferSize_TP buffer_size,
                       float max_y_value,
                       float min_y_value,
                       const OGLChartGeometry_C& geometry,
                       const QOpenGLWidget& parent)
    :  OGLBaseChart_C(geometry, parent),
    _lead_line_vbo(QOpenGLBuffer::VertexBuffer),
    _time_range_ms(time_range_ms),
    _input_buffer(buffer_size),
    _ogl_data_series(_input_buffer.MaxSize(), time_range_ms, _input_buffer)
{
    _max_y_axis_value = max_y_value;
    _min_y_axis_value = min_y_value;
}

void OGLSweepChart_C::Initialize() 
{
    DEBUG("Initialize OGLChart");

    // Allocate a vertex buffer object to store data for visualization
    _ogl_data_series.AllocateSeriesVbo();
    
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
    auto& vertical_verts   = grid_vertices.second;
    double scale = 0.25f;
    // Use the surface grid vertice
    InitializeAxesDescription(horizontal_verts, 
                              vertical_verts, 
                              scale, 
                             _time_range_ms, 
                             _max_y_axis_value, 
                             _major_tick_x_axes, 
                             _major_tick_y_axes );
}

void OGLSweepChart_C::AddDataTimestamp(const float value, const Timestamp_TP & timestamp)
{
    // Don't add the value if its not inside the range, 
    // because its not visible eitherway -> better solution would be: Add it to the series but don't draw it!
    if ( value > _max_y_axis_value || value < _min_y_axis_value ) {
        return;
    }

    // - (minus) because then the positive y axis is directing at the top of the screen
    float y_val_scaled_S = static_cast<float>(_plot_area.GetLeftTop()._y) -
                          ((value - _min_y_axis_value) / (_max_y_axis_value - _min_y_axis_value)) * 
        _plot_area.GetChartHeight();

    auto x_ms = timestamp.GetMilliseconds();
    // use modulo to wrap the dataseries
    float x_val_scaled_S = static_cast<float>(_plot_area.GetLeftBottom()._x) +
                                             ( ( x_ms % static_cast<int>(_time_range_ms) ) / _time_range_ms ) *
        _plot_area.GetChartWidth();

    DEBUG("Scaled x value: " << x_val_scaled_S << ", to value: " << x_ms_modulo);
    DEBUG("Scaled y value: " << y_val_scaled_S << ", to value: " << value);
    _input_buffer.InsertAtTail(
        ChartPoint_TP<Position3D_TC<float>> (Position3D_TC<float>(x_val_scaled_S, 
                                                                  y_val_scaled_S, 
                                                                  1.0f), x_ms));
}

//! Uses the bounding box
float OGLSweepChart_C::GetScreenCoordsFromYChartValue(float y_value) 
{
   float y_value_S =  static_cast<float>(_bounding_box.GetLeftTop()._y) -
        ((y_value - _min_y_axis_value) / (_max_y_axis_value - _min_y_axis_value)) * _bounding_box.GetChartHeight();

   return y_value_S;
}


float OGLSweepChart_C::GetScreenCoordsFromXChartValue(float x_value_ms)
{
    // calculate new x-index when the dataseries has reached the left border of the plot
    float x_val_wrap_corrected_ms = x_value_ms;

    // calculate x-value after wrapping
    float x_value_S = static_cast<float>(_bounding_box.GetLeftBottom()._x) +
                     ((x_val_wrap_corrected_ms) / (_time_range_ms)) * _bounding_box.GetChartWidth();

    return x_value_S;
}

void OGLSweepChart_C::SetChartType(DrawingStyle_TP chart_type)
{
    _ogl_data_series.SetPrimitiveType(chart_type);
}

void OGLSweepChart_C::SetMajorTickValueXAxes(float tick_value_ms)
{
    _major_tick_x_axes = tick_value_ms;
}

void OGLSweepChart_C::SetMajorTickValueYAxes(float tick_value_unit)
{
    _major_tick_y_axes = tick_value_unit;
}

void OGLSweepChart_C::SetLeadLineColor(const QVector3D& color)
{
    _lead_line_color = color;
}

void OGLSweepChart_C::Draw(QOpenGLShaderProgram& shader, QOpenGLShaderProgram& text_shader) 
{
    DrawSeries(shader);
    DrawBoundingBox(shader);
    DrawLeadLine(shader);
    DrawSurfaceGrid(shader);
    DrawXYAxes(shader, text_shader);
}

void OGLSweepChart_C::CreateLeadLineVbo() 
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

void OGLSweepChart_C::UpdateLeadLinePosition(float x_value_new) 
{
    _lead_line_vertices[0] = x_value_new;
    _lead_line_vertices[3] = x_value_new;
    // Overwrite the whole vbo with the new data
    // alternative: write just two values, but because of caching this should not really result in differences..
    _lead_line_vbo.write(0, _lead_line_vertices.constData(), _number_of_bytes_lead_line);
}

inline
void OGLSweepChart_C::DrawLeadLine(QOpenGLShaderProgram& shader)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    shader.bind();
    shader.setUniformValue("u_object_color", _lead_line_color);
    _lead_line_vbo.bind();

    UpdateLeadLinePosition(_ogl_data_series.GetLastPlottedXValue() );

    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    f->glDrawArrays(GL_LINES, 0, 2);
    f->glDisableVertexAttribArray(0);
    _lead_line_vbo.release();
}

void OGLSweepChart_C::DrawSeries(QOpenGLShaderProgram& shader)
{
    auto* f = QOpenGLContext::currentContext()->functions();
    shader.bind();
    shader.setUniformValue("u_object_color", _series_color);
    _ogl_data_series.Draw();
}
