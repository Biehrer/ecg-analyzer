#include <OGLChart.h>

//#define DEBUG_INFO

#ifdef DEBUG_INFO
    #define DEBUG(msg) std::cout << msg << std::endl;
#else
    #define DEBUG(msg) do{} while(0)
#endif

OGLChart_C::~OGLChart_C() 
{
    _chart_vbo.destroy();
    _y_axis_vbo.destroy();
    _x_axis_vbo.destroy();
    _bb_vbo.destroy();
    _surface_grid_vbo.destroy();
}

OGLChart_C::OGLChart_C(int time_range_ms,
                       int buffer_size,
                       float max_y_value,
                       float min_y_value,
                       const OGLChartGeometry_C& geometry,
                       const QOpenGLWidget& parent)
    :
    _buffer_size(buffer_size),
    _vbo_buffer_size(buffer_size * 3 * sizeof(float)),    // vbo buffer size(3 times the point buffer size) each point consists of 3 floats - i could make this template but then i can only do header files.
    _vbo_current_series_idx(0),
    _chart_vbo(QOpenGLBuffer::VertexBuffer),
    _x_axis_vbo(QOpenGLBuffer::VertexBuffer),
    _y_axis_vbo(QOpenGLBuffer::VertexBuffer),
    _bb_vbo(QOpenGLBuffer::VertexBuffer),
    _surface_grid_vbo(QOpenGLBuffer::VertexBuffer),
    _lead_line_vbo(QOpenGLBuffer::VertexBuffer),
    _input_buffer(buffer_size),
    _input_buffer_new(buffer_size),
    _no_line_vertices(buffer_size),
    _geometry(geometry.GetLeftBottom()._x, geometry.GetLeftBottom()._y, geometry.GetChartWidth(), geometry.GetChartHeight()),
    _time_range_ms(time_range_ms),
    _parent_widget(parent)
{
    Initialize(max_y_value, min_y_value);
}

void OGLChart_C::Initialize(float max_y_val, float min_y_val) 
{
    DEBUG("Initialize OGLChart");

    // Sweep-chart parameters
    _number_of_wraps = 1;
    _need_to_wrap_series = false;
    _dataseries_wrapped_once = false;

    // number of visualized points in the graph
    _point_count = 0;
    
    _min_y_axis_value = min_y_val;
    _max_y_axis_value = max_y_val;
    
    // Allocate a vertex buffer object to store data for visualization
    AllocateSeriesVbo();
    // Allocate a vertex buffer object to store data for the lead line
    CreateLeadLineVbo();
    // Create vbo for the x and y axis
    SetupAxes();
    // Create vbo for the bounding box of the chart
    CreateBoundingBox();
    // Create surface grid vbo
    CreateSurfaceGrid(200, 5);

    _no_line_vertices.fill(NAN, _buffer_size);
}

void OGLChart_C::AllocateSeriesVbo()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    // Setup OGL-Chart buffer - empty
    _chart_vbo.create();
    _chart_vbo.bind();
    f->glEnableVertexAttribArray(0);
    // 3 coordinates make one point  (x, y, z)
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    _chart_vbo.allocate(nullptr, _vbo_buffer_size);
    _chart_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    f->glDisableVertexAttribArray(0);
    _chart_vbo.release();
}

// Todo : Refactor this function in the RingBuffer itself ! Then we can call this function on the ringbuffer -> should work!
int
OGLChart_C::FindIdxToTimestampInsideData(const Timestamp_TP& timestamp,
    const std::vector<ChartPoint_TP<Position3D_TC<float>>>& data)
{
    int current_idx =  _vbo_current_series_idx / 3 / sizeof(float);
    //if( current_idx > _remove_series_idx ){
        // case 1
        auto it = std::lower_bound(data.begin(), data.end(), timestamp, CmpTimestamps);
        if ( it != data.end() ) {
            std::size_t index = std::distance(data.begin(), it);
            return index;
        }else {
            //return -1;
        }
    //} else {
        // if _remove_idx > head_idx the data to remove is most probably at the end of the buffer => check first the end
        auto latest_raw_data_begin = data.begin() + _remove_series_idx;
        auto it_interesting_data_range_until_end = std::lower_bound(latest_raw_data_begin, data.end(), timestamp, CmpTimestamps); // search inside the old data
        if ( it_interesting_data_range_until_end != data.end() ) {
            std::size_t index = std::distance(data.begin(), it_interesting_data_range_until_end);
            return index;
        }

        auto current_data_ptr = data.begin() + current_idx;
        auto it_interesting_data_range_until_current = std::lower_bound(data.begin(), current_data_ptr, timestamp, CmpTimestamps); // search inside the old data
        if ( it_interesting_data_range_until_current != current_data_ptr ) {
            std::size_t index = std::distance(data.begin(), it_interesting_data_range_until_current);
            return index;
        } else {
            return -1;
        }
    //}
}

void OGLChart_C::AddDataTimestamp(float value, Timestamp_TP & timestamp)
{
    // Don't add the value if its not inside the range, 
   // because its not visible eitherway -> better solution would be: Add it to the series but don't draw it!
    if ( value > _max_y_axis_value || value < _min_y_axis_value ) {
        return;
    }

    // wrapp value around x-axis if the 'x_ms' value is bigger than maximum value of the x-axis
    if ( _need_to_wrap_series ) {
        _number_of_wraps++;
        _need_to_wrap_series = false;
    }

    auto x_ms = timestamp.GetMilliseconds();
    auto x_ms_modulo = x_ms % static_cast<int>(_time_range_ms);

    // check if we need to wrap the data (when data series reached the right border of the screen)
    if ( x_ms_modulo >= static_cast<float>(_time_range_ms) * static_cast<float>(_number_of_wraps) ) {  // Change modulo to x_ms
         // Calculate new x value at most left chart position
        _need_to_wrap_series = true;
        _dataseries_wrapped_once = true;
    }

    // - (minus) because then the positive y axis is directing at the top of the screen
    float y_val_scaled_S = static_cast<float>(_geometry.GetLeftTop()._y) -
        ((value - _min_y_axis_value) / (_max_y_axis_value - _min_y_axis_value)) * _geometry.GetChartHeight();

    DEBUG("Scaled y value: " << y_val_scaled_S << ", to value: " << value);

    // - so the data value runs from left to right side
    float x_val_wrap_corrected_ms = x_ms_modulo - static_cast<float>(_time_range_ms) * static_cast<float>(_number_of_wraps - 1);     // Change modulo to x_ms

    // calculate x-value after wrapping
    float x_val_scaled_S = static_cast<float>(_geometry.GetLeftBottom()._x) +
                             ((x_val_wrap_corrected_ms) / (_time_range_ms)) *
                             _geometry.GetChartWidth();

    _input_buffer_new.InsertAtTail(ChartPoint_TP<Position3D_TC<float>>(Position3D_TC<float>(x_val_scaled_S, y_val_scaled_S, 1.0f), x_ms));
    DEBUG("Scaled x value: " << x_val_scaled_S << ", to value: " << x_ms);
}


float OGLChart_C::GetScreenCoordsFromYChartValue(float y_value) 
{
   float y_value_S =  static_cast<float>(_geometry.GetLeftTop()._y) -
        ((y_value - _min_y_axis_value) / (_max_y_axis_value - _min_y_axis_value)) * _geometry.GetChartHeight();

   return y_value_S;
}

float OGLChart_C::GetScreenCoordsFromXChartValue(float x_value_ms)
{
    // calculate new x-index when the dataseries has reached the left border of the plot
    float x_val_wrap_corrected_ms = x_value_ms - static_cast<float>(_time_range_ms) * static_cast<float>(_number_of_wraps - 1);

    // calculate x-value after wrapping
    float x_value_S = static_cast<float>(_geometry.GetLeftBottom()._x) +
                     ((x_val_wrap_corrected_ms) / (_time_range_ms)) * _geometry.GetChartWidth();

    return x_value_S;
}

inline 
void 
OGLChart_C::OnChartUpdate() 
{
    if ( _input_buffer_new.IsBufferEmpty() ) {
        return;
    }

    // Get latest data from the input buffer
    auto latest_data = _input_buffer_new.PopLatest();

    if ( !latest_data.empty() ) {
        QVector<float> additional_point_vertices;
        for ( const auto& element : latest_data ) {
            // Check if its neccessary to end the line strip,
            // due to a wrap of the series from the right to the left screen border
            if ( element._value._x < _last_plotted_x_value_S ) {
                additional_point_vertices.append(NAN);
                additional_point_vertices.append(NAN);
                additional_point_vertices.append(NAN);
            }
            //DEBUG(element);
            additional_point_vertices.append(element._value._x);
            additional_point_vertices.append(element._value._y);
            additional_point_vertices.append(element._value._z);
            _last_plotted_x_value_S = element._value._x;
        }

        _last_plotted_y_value_S = (latest_data.end() - 1)->_value._y;
        _last_plotted_x_value_S = (latest_data.end() - 1)->_value._x;

        // Write data to the vbo
        WriteToVbo(additional_point_vertices);
        // Remove old data out of timerange
        RemoveOutdatedDataInsideVBO();
    }
}

// MAke sure the buffer is bound to the current context before calling this function
void OGLChart_C::WriteToVbo(const QVector<float>& data)
{
    int number_of_new_data_bytes = static_cast<int>(data.size()) * static_cast<int>(sizeof(float));
 
    if (_vbo_current_series_idx + number_of_new_data_bytes <=_vbo_buffer_size) {
        // The data can completely fit into the vbo 
        _chart_vbo.write(static_cast<int>(_vbo_current_series_idx), data.constData(), number_of_new_data_bytes);
        // new write offset in bytes
        _vbo_current_series_idx += number_of_new_data_bytes;        
        IncrementPointCount(data.size() / 3);
    } else {
        // buffer is full or not all new data can fit into it; 
        // reset buffer index and start overwriting data at the beginning
        // Calculate how much bytes can fit into the buffer until the end is reached
        int number_of_free_bytes_until_end = _vbo_buffer_size - _vbo_current_series_idx;
        int bytes_to_write_at_beginning = number_of_new_data_bytes - number_of_free_bytes_until_end;
        int bytes_to_write_until_end = number_of_new_data_bytes - bytes_to_write_at_beginning;

        if( number_of_free_bytes_until_end > 0) {
            // Write data until the end of the buffer is reached
           _chart_vbo.write(static_cast<int>(_vbo_current_series_idx), data.data(), bytes_to_write_until_end);
           IncrementPointCount(number_of_free_bytes_until_end / sizeof(float) / 3);
        }

        _dataseries_wrapped_once = true;
        // Reset the index to continue writing the rest of the data at the beginning
        _vbo_current_series_idx = 0;
        if ( bytes_to_write_at_beginning > 0 ) {
            int data_memory_offset = bytes_to_write_until_end / sizeof(float); 
            _chart_vbo.write(static_cast<int>(_vbo_current_series_idx), (data.constData() + data_memory_offset), bytes_to_write_at_beginning);
            _vbo_current_series_idx += bytes_to_write_at_beginning;
        }
    }
}

inline void OGLChart_C::IncrementPointCount(size_t increment) 
{
    // Count points; stop counting points after one wrap
    // (because after a wrap the point count stays the same-> NOT ANYMORE)
    if ( !_dataseries_wrapped_once ) {
        _point_count += increment;
    }
}

inline 
void 
OGLChart_C::RemoveOutdatedDataInsideVBO() 
{
    size_t last_added_tstamp_ms = _input_buffer_new.GetLatestItem()._timestamp.GetSeconds();
    double start_time_ms = static_cast<double>(last_added_tstamp_ms) - _time_range_ms;
    int start_time_idx = FindIdxToTimestampInsideData(Timestamp_TP(start_time_ms), _input_buffer_new.constData()) - 1;
    
    if ( start_time_idx > -1 ) {
        int bytes_to_remove = (start_time_idx + 1 - _remove_series_idx) * 3 * sizeof(float);

        if ( bytes_to_remove > 0 ) {
            _chart_vbo.write(_remove_series_idx * 3 * sizeof(float), _no_line_vertices.constData(), bytes_to_remove);
            _remove_series_idx = start_time_idx;
        } else {
            // recalculate
            int remove_series_idx_byte = _remove_series_idx * 3 * sizeof(float);
            int number_of_free_bytes_until_end = _vbo_buffer_size - remove_series_idx_byte;
            _chart_vbo.write(_remove_series_idx * 3 * sizeof(float), _no_line_vertices.constData(), number_of_free_bytes_until_end);
            int bytes_to_remove_at_beginning = (start_time_idx + 1) * 3 * sizeof(float);
            _chart_vbo.write(0, _no_line_vertices.constData(), bytes_to_remove_at_beginning);
            _remove_series_idx = start_time_idx;
        }
    }
}


void OGLChart_C::Draw()
{
    DrawSeries();
    DrawXYAxes();
    DrawBoundingBox();
    DrawLeadLine();
    DrawSurfaceGrid();
}


// Todo: x-axes should always be at the position where the chart hast the value zero at the y axis.
void OGLChart_C::SetupAxes() 
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

void OGLChart_C::DrawSurfaceGrid() 
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    _surface_grid_vbo.bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    f->glDrawArrays(GL_LINES, 0, _num_of_surface_grid_vertices);
    f->glDisableVertexAttribArray(0);
    _surface_grid_vbo.release();
}

void OGLChart_C::CreateSurfaceGrid(int x_major_tick_dist_ms, int y_major_tick_dist_unit)
{
    auto surface_grid_vertices = 
        ChartShapes_C<float>::CreateSurfaceGridVertices(_geometry, 
                                                        _time_range_ms, 
                                                        _max_y_axis_value,
                                                        _min_y_axis_value, 
                                                        1000.0,
                                                        5.0 );
    _num_of_surface_grid_vertices = surface_grid_vertices.size() / 3;

    // Create VBO
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    // Setup OGL Chart buffer - empty 
    _surface_grid_vbo.create();
    _surface_grid_vbo.bind();
    f->glEnableVertexAttribArray(0);
    // 3 positions for x and y and z data coordinates
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    _surface_grid_vbo.allocate(surface_grid_vertices.constData(), surface_grid_vertices.size() * static_cast<int>(sizeof(float)));
    _surface_grid_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    f->glDisableVertexAttribArray(0);
    _surface_grid_vbo.release();
}

void OGLChart_C::CreateLeadLineVbo() 
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

void OGLChart_C::UpdateLeadLinePosition(float x_value_new) 
{
    _lead_line_vertices[0] = x_value_new;
    _lead_line_vertices[3] = x_value_new;
    // Overwrite the whole vbo with the new data
    // alternative: write just two values, but because of caching this should not really result in differences..
    _lead_line_vbo.write(0, _lead_line_vertices.constData(), _number_of_bytes_lead_line);
}

void OGLChart_C::DrawLeadLine() 
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    _lead_line_vbo.bind();
    UpdateLeadLinePosition(_last_plotted_x_value_S);
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    f->glDrawArrays(GL_LINES, 0, 2);
    f->glDisableVertexAttribArray(0);
    _lead_line_vbo.release();
}


void OGLChart_C::DrawXYAxes()
{
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
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
}

void OGLChart_C::DrawBoundingBox()
{
    auto* f = QOpenGLContext::currentContext()->functions();
    _bb_vbo.bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    f->glDrawArrays(GL_LINES, 0, 6);
    f->glDisableVertexAttribArray(0);
    _bb_vbo.release();
}

void OGLChart_C::DrawSeries() 
{
    auto* f = QOpenGLContext::currentContext()->functions();
    // Bind buffer and send data to the gpu
    _chart_vbo.bind();
    OnChartUpdate();
    // Draw inside the current context
    f->glEnableVertexAttribArray(0);
    //f->glEnableVertexAttribArray(1);
    // each point (GL_POINT) consists of 3 components (x, y, z)
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    // to get the abs number of points-> divide through count of each Point
    f->glDrawArrays(GL_LINE_STRIP, 0, _point_count);
    f->glDisableVertexAttribArray(0);
    _chart_vbo.release();
}

void OGLChart_C::CreateBoundingBox()
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

void OGLChart_C::addRange(int count, QVector<double> data)
{
    //void* pointerToData = chartVBO.mapRange(_bufferIndex, count, QOpenGLBuffer::RangeAccessFlag::RangeWrite);
    //chartVBO.write(_bufferIndex, data.constData(), count);
    //_bufferIndex += count;
    _chart_vbo.bind();
    /*Example:
    m_vertex.bind();
    auto ptr = m_vertex.map(QOpenGLBuffer::WriteOnly);
    memcpy(ptr, Pts.data(), Pts.size() * sizeof(Pts[0]));	//map data to buffer
    m_vertex.unmap();
    //do stuff and draw
    m_vertex.release();
    */
    //echartVBO.mapRange()
    //auto pointerToData = chartVBO.mapRange(0, _bufferIndex, QOpenGLBuffer::RangeAccessFlag::RangeRead);
    auto point = _chart_vbo.map(QOpenGLBuffer::Access::ReadOnly);
    std::vector<float> dat; dat.resize(100);

    memcpy(&dat[0], &point, 100);
    _chart_vbo.release();
    //dat.toStdVector();
    //std::copy(pointerToData, *pointerToData+11,std::back_inserter(dat.toStdVector()));
}

//void OGLChart::sendDataToOGL(QOpenGLBuffer& chartVBO)//makes the function better for reuse..(we can use at as general buffer write function)
