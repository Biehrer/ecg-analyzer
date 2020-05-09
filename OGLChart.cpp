#include <OGLChart.h>

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
                       int buffer_size,
                       float max_y_value,
                       float min_y_value,
                       const OGLChartGeometry_C& geometry,
                       const QOpenGLWidget& parent)
    :
    _x_axis_vbo(QOpenGLBuffer::VertexBuffer),
    _y_axis_vbo(QOpenGLBuffer::VertexBuffer),
    _bb_vbo(QOpenGLBuffer::VertexBuffer),
    _surface_grid_vbo(QOpenGLBuffer::VertexBuffer),
    _lead_line_vbo(QOpenGLBuffer::VertexBuffer),
    _geometry(geometry),
    _time_range_ms(time_range_ms),
    _input_buffer(buffer_size),
    _ogl_data_series(buffer_size, time_range_ms, _input_buffer),
    _parent_widget(parent)
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
    auto grid_vertices = CreateSurfaceGrid(_major_tick_x_axes, _major_tick_y_axes);
    auto& horizontal_verts = grid_vertices.first;
    auto& vertical_verts = grid_vertices.second;
    // Use the surface grid vertice
    InitializeAxesDescription(horizontal_verts, vertical_verts, 0.25f);
}


void OGLSweepChart_C::InitializeAxesDescription(const QVector<float>& horizontal_grid_vertices, 
                                                const QVector<float>& vertical_grid_vertices,
                                                float scale)
{
    int num_of_horizontal_desc = horizontal_grid_vertices.size() / 2 / 3 + 1;   // /2 -> only need half the vertices (Point FROM, not point TO)
    int num_of_vertical_desc = vertical_grid_vertices.size() / 2 / 3 + 1;      // /3 -> only need number of text fields for now. Three vertices are one text field
    // +1 because of the 0 lines !!!!
    int num_of_descriptions = num_of_horizontal_desc + num_of_vertical_desc;

    _plot_axes.reserve(num_of_descriptions);
    _plot_axes.resize(num_of_descriptions);

    for ( auto& description : _plot_axes ) {
        description.Initialize(Font2D_TP::ARIAL);
    }

    // time_range = 10 ms -> 10 x descriptions 0 - 10 (11 nums)
    // max_y_val = -5 / +5 -> 2x description (1 x -5 ; 1 x +5; 1 x 0) 
    int offset_from_left_screen_border = 10;

    auto _plot_axes_horizontal_end = _plot_axes.begin() + num_of_horizontal_desc;
    float current_y_description = _max_y_axis_value;
    // horizontal changing descriptions (y-axes)
    for ( auto plot_desc_y_it = _plot_axes.begin(); plot_desc_y_it != _plot_axes_horizontal_end; ++plot_desc_y_it ) {
        // Start with the first
        int pos_x = _geometry.GetLeftBottom()._x + offset_from_left_screen_border;
        int pos_y = GetScreenCoordsFromYChartValue(current_y_description);
        std::string text = std::to_string(current_y_description);
        plot_desc_y_it->SetText(text, pos_x, pos_y, scale);
        current_y_description -= _major_tick_y_axes;
    }

    // vertical changing descriptions (x-axes)
    float current_x_description = _time_range_ms; 
    for ( auto plot_desc_x_it = _plot_axes_horizontal_end; plot_desc_x_it != _plot_axes.end(); ++plot_desc_x_it ) {
        int pos_x = GetScreenCoordsFromXChartValue(current_x_description);
        int pos_y = _geometry.GetLeftTop()._y + 5; // +_geometry.GetChartHeight() / 2;
        std::string text = std::to_string(current_x_description);
        plot_desc_x_it->SetText(text, pos_x, pos_y, scale);
        current_x_description -= _major_tick_x_axes;
    }


}

void OGLSweepChart_C::AddDataTimestamp(float value, Timestamp_TP & timestamp)
{
    // Don't add the value if its not inside the range, 
   // because its not visible eitherway -> better solution would be: Add it to the series but don't draw it!
    if ( value > _max_y_axis_value || value < _min_y_axis_value ) {
        return;
    }

    auto x_ms = timestamp.GetMilliseconds();
    auto x_ms_modulo = x_ms % static_cast<int>(_time_range_ms);

    // - (minus) because then the positive y axis is directing at the top of the screen
    float y_val_scaled_S = static_cast<float>(_geometry.GetLeftTop()._y) -
        ((value - _min_y_axis_value) / (_max_y_axis_value - _min_y_axis_value)) * _geometry.GetChartHeight();

    DEBUG("Scaled y value: " << y_val_scaled_S << ", to value: " << value);

    // - so the data value runs from left to right side
    float x_val_wrap_corrected_ms = x_ms_modulo;

    // calculate x-value after wrapping
    float x_val_scaled_S = static_cast<float>(_geometry.GetLeftBottom()._x) +
                             ((x_val_wrap_corrected_ms) / (_time_range_ms)) *
                             _geometry.GetChartWidth();

    _input_buffer.InsertAtTail(ChartPoint_TP<Position3D_TC<float>>(Position3D_TC<float>(x_val_scaled_S, y_val_scaled_S, 1.0f), x_ms));
    DEBUG("Scaled x value: " << x_val_scaled_S << ", to value: " << x_ms);
}


float OGLSweepChart_C::GetScreenCoordsFromYChartValue(float y_value) 
{
   float y_value_S =  static_cast<float>(_geometry.GetLeftTop()._y) -
        ((y_value - _min_y_axis_value) / (_max_y_axis_value - _min_y_axis_value)) * _geometry.GetChartHeight();

   return y_value_S;
}

float OGLSweepChart_C::GetScreenCoordsFromXChartValue(float x_value_ms)
{
    // calculate new x-index when the dataseries has reached the left border of the plot
    float x_val_wrap_corrected_ms = x_value_ms;

    // calculate x-value after wrapping
    float x_value_S = static_cast<float>(_geometry.GetLeftBottom()._x) +
                     ((x_val_wrap_corrected_ms) / (_time_range_ms)) * _geometry.GetChartWidth();

    return x_value_S;
}

void OGLSweepChart_C::SetMajorTickValueXAxes(float tick_value_ms)
{
    _major_tick_x_axes = tick_value_ms;
}

void OGLSweepChart_C::SetMajorTickValueYAxes(float tick_value_unit)
{
    _major_tick_y_axes = tick_value_unit;
}

void OGLSweepChart_C::SetAxesColor(const QVector3D& color)
{
    _axes_color = color;
}

void OGLSweepChart_C::SetTextColor(const QVector3D& color)
{
    _text_color = color;
}
void OGLSweepChart_C::SetSeriesColor(const QVector3D& color)
{
    _series_color = color;
}

void OGLSweepChart_C::SetBoundingBoxColor(const QVector3D& color)
{
    _bounding_box_color = color;
}

void OGLSweepChart_C::SetSurfaceGridColor(const QVector3D& color)
{
    _surface_grid_color = color;
}

void OGLSweepChart_C::SetLeadLineColor(const QVector3D& color)
{
    _lead_line_color = color;
}

void OGLSweepChart_C::SetModelViewProjection(QMatrix4x4 model_view_projection)
{
    _chart_mvp = model_view_projection;
}

void OGLSweepChart_C::Draw(QOpenGLShaderProgram& shader, QOpenGLShaderProgram& text_shader) 
{
    DrawSeries(shader);
    DrawBoundingBox(shader);
    DrawLeadLine(shader);
    DrawSurfaceGrid(shader);
    DrawXYAxes(shader, text_shader);
}


// Todo: x-axes should always be at the position where the chart hast the value zero at the y axis.
void OGLSweepChart_C::SetupAxes() 
{
    const auto axes_vertices = ChartShapes_C<float>::MakesAxesVertices(_geometry, 5.0); //CreateAxesVertices(5.0);
    auto& x_axis_vertices = axes_vertices._x_axis_vertices;
    auto& y_axis_vertices = axes_vertices._y_axis_vertices;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	// Setup OGL Chart buffer - empty 
    _x_axis_vbo.create();
    _x_axis_vbo.bind();
	f->glEnableVertexAttribArray(0);
    // 3 positions for x and y and z data coordinates
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    _x_axis_vbo.allocate(x_axis_vertices.constData(), x_axis_vertices.size() * static_cast<int>(sizeof(float)) );
    _x_axis_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	f->glDisableVertexAttribArray(0);
    _x_axis_vbo.release();

	// Setup OGL Chart buffer - empty 
    _y_axis_vbo.create();
    _y_axis_vbo.bind();
	f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    _y_axis_vbo.allocate(y_axis_vertices.constData(), y_axis_vertices.size() * static_cast<int>(sizeof(float)) );
    _y_axis_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	f->glDisableVertexAttribArray(0);
    _y_axis_vbo.release();
}

inline
void OGLSweepChart_C::DrawSurfaceGrid( QOpenGLShaderProgram& shader)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    shader.bind();
    shader.setUniformValue("u_object_color", _surface_grid_color);
    _surface_grid_vbo.bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    f->glDrawArrays(GL_LINES, 0, _num_of_surface_grid_positions);
    f->glDisableVertexAttribArray(0);
    _surface_grid_vbo.release();
}

std::pair<QVector<float>, QVector<float>>
OGLSweepChart_C::CreateSurfaceGrid(int x_major_tick_dist_ms, int y_major_tick_dist_unit)
{
    auto surface_grid_vertices = 
        ChartShapes_C<float>::CreateSurfaceGridVertices(_geometry, 
                                                        _time_range_ms, 
                                                        _max_y_axis_value,
                                                        _min_y_axis_value, 
                                                        x_major_tick_dist_ms,
                                                        y_major_tick_dist_unit);

    auto& horizontal_grid_vertices = surface_grid_vertices.first;
    auto& vertical_grid_vertices = surface_grid_vertices.first;

    _num_of_surface_grid_positions = horizontal_grid_vertices.size() / 3 + 
                                       vertical_grid_vertices.size() / 3;

    QVector<float> combined_vertices; 
    combined_vertices.reserve(horizontal_grid_vertices.size() + vertical_grid_vertices.size());
    for ( auto& horizontal_vert : horizontal_grid_vertices ) {
        combined_vertices.append(horizontal_vert);
    }
    for ( auto& vertical_vert : vertical_grid_vertices ) {
        combined_vertices.append(vertical_vert);
    }

    // Create VBO
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    // Setup OGL Chart buffer - empty 
    _surface_grid_vbo.create();
    _surface_grid_vbo.bind();
    f->glEnableVertexAttribArray(0);
    // 3 positions for x and y and z data coordinates
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    _surface_grid_vbo.allocate(combined_vertices.constData(), combined_vertices.size() * static_cast<int>(sizeof(float)));
    _surface_grid_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    f->glDisableVertexAttribArray(0);
    _surface_grid_vbo.release();

    return surface_grid_vertices;
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
    _lead_line_vertices[1] = _geometry.GetLeftBottom()._y;
    // z value
    _lead_line_vertices[2] = _geometry.GetZPosition();
    // point to:
    // y value
    _lead_line_vertices[4] = _geometry.GetLeftTop()._y;
    // z value
    _lead_line_vertices[5] = _geometry.GetZPosition();
}

void OGLSweepChart_C::UpdateLeadLinePosition(float x_value_new) 
{
    _lead_line_vertices[0] = x_value_new;
    _lead_line_vertices[3] = x_value_new;
    // Overwrite the whole vbo with the new data
    // alternative: write just two values, but because of caching this should not really result in differences..
    _lead_line_vbo.write(0, _lead_line_vertices.constData(), _number_of_bytes_lead_line);
}

// expects that the shader is bound to the context
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

inline
void OGLSweepChart_C::DrawXYAxes(QOpenGLShaderProgram& shader, QOpenGLShaderProgram& text_shader)
{
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    shader.bind();
    shader.setUniformValue("u_object_color", _axes_color);
    _y_axis_vbo.bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    f->glDrawArrays(GL_TRIANGLES, 0, 6);
    f->glDisableVertexAttribArray(0);
    _y_axis_vbo.release();

    _x_axis_vbo.bind();
	f->glEnableVertexAttribArray(0);
	f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	f->glDrawArrays(GL_TRIANGLES, 0, 6);
	f->glDisableVertexAttribArray(0);
    _x_axis_vbo.release();

    // Draw the descriptions
    for ( const auto& description : _plot_axes ) {
        // => Cherno 2 in 1 shader 
        // -> bind a white 1x1 texture to use flat colors and bind a 1.0 color to just draw the texture, 
        // use a texture sampler, if not, just use the uniform color !
         description.RenderText(text_shader, _text_color, _chart_mvp);
    }
}

inline
void OGLSweepChart_C::DrawBoundingBox(QOpenGLShaderProgram& shader)
{
    auto* f = QOpenGLContext::currentContext()->functions();
    shader.bind();
    shader.setUniformValue("u_object_color", _bounding_box_color);
    _bb_vbo.bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    f->glDrawArrays(GL_LINES, 0, 6);
    f->glDisableVertexAttribArray(0);
    _bb_vbo.release();
}

inline
void OGLSweepChart_C::DrawSeries(QOpenGLShaderProgram& shader)
{
    auto* f = QOpenGLContext::currentContext()->functions();
    shader.bind();
    shader.setUniformValue("u_object_color", _series_color);
    _ogl_data_series.Draw();
}

void OGLSweepChart_C::CreateBoundingBox()
{
    auto bb_vertices = ChartShapes_C<float>::MakeBoundingBoxVertices(_geometry);
    // Setup vbo
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    _bb_vbo.create();
    _bb_vbo.bind();
    f->glEnableVertexAttribArray(0);
    // 3 positions for x and y and z data coordinates
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    _bb_vbo.allocate(bb_vertices.constData(), bb_vertices.size() * static_cast<int>(sizeof(float)));
    _bb_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    f->glDisableVertexAttribArray(0);
    _bb_vbo.release();
}

void OGLSweepChart_C::addRange(int count, QVector<double> data)
{
    //void* pointerToData = chartVBO.mapRange(_bufferIndex, count, QOpenGLBuffer::RangeAccessFlag::RangeWrite);
    //chartVBO.write(_bufferIndex, data.constData(), count);
    //_bufferIndex += count;
    //_chart_vbo.bind();
    /*Example:
    m_vertex.bind();
    auto ptr = m_vertex.map(QOpenGLBuffer::WriteOnly);
    memcpy(ptr, Pts.data(), Pts.size() * sizeof(Pts[0]));	//map data to buffer
    m_vertex.unmap();
    //do stuff and draw
    m_vertex.release();
    */
    //echartVBO.mapRange()
    ////auto pointerToData = chartVBO.mapRange(0, _bufferIndex, QOpenGLBuffer::RangeAccessFlag::RangeRead);
    //auto point = _chart_vbo.map(QOpenGLBuffer::Access::ReadOnly);
    //std::vector<float> dat; dat.resize(100);

    //memcpy(&dat[0], &point, 100);
    //_chart_vbo.release();
    //dat.toStdVector();
    //std::copy(pointerToData, *pointerToData+11,std::back_inserter(dat.toStdVector()));
}

//void OGLChart::sendDataToOGL(QOpenGLBuffer& chartVBO)//makes the function better for reuse..(we can use at as general buffer write function)
