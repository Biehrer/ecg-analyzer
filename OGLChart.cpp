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

// This is the specialization - pointchart realtime
OGLChart_C::OGLChart_C(int max_num_of_points_in_buffer,
                       int screen_pos_x,
                       int screen_pos_y,
                       int chart_width_S,
                       int chart_height_S, 
                       const QOpenGLWidget& parent)
    : 
    _vbo_buffer_size( max_num_of_points_in_buffer * 3 * sizeof(float) ),    // vbo buffer size(3 times the point buffer size) each point consists of 3 floats - i could make this template but then i can only do header files.
    _vbo_current_series_idx(0),
    _chart_vbo(QOpenGLBuffer::VertexBuffer),
    _x_axis_vbo(QOpenGLBuffer::VertexBuffer),
    _y_axis_vbo(QOpenGLBuffer::VertexBuffer),
    _bb_vbo(QOpenGLBuffer::VertexBuffer),
    _surface_grid_vbo(QOpenGLBuffer::VertexBuffer),
    _lead_line_vbo(QOpenGLBuffer::VertexBuffer),
    _input_buffer(max_num_of_points_in_buffer),
    _input_buffer_new(max_num_of_points_in_buffer),
    _no_line_vertices(max_num_of_points_in_buffer),
    _parent_widget(parent)
{
    DEBUG("Initialize OGLChart");

    // Sweep-chart parameters
    _number_of_wraps = 1;
    _need_to_wrap_series = false;
    _dataseries_wrapped_once = false;

    // number of visualized points in the graph
    _point_count = 0;

    // where to place Chart in respect to the OGL Viewport
    _screen_pos_x_S = screen_pos_x;
    _screen_pos_y_S = screen_pos_y;

    _width_S = chart_width_S;
    _height_S = chart_height_S;

    // chart values x and y axes
    _min_y_axis_value = -10;
    _max_y_axis_value = 10;

    _min_x_axis_val_ms = 0;
    // 10 secs of data - buffersize(OGL buffer) needs to be this big!
    // NEEDS TO MATCH WITH THE BUFFER SIZE THE USER USES AS INPUT(NUMBER OF VALUES IN BUFFER)
    // -> therefore create the buffer variable dependent on how many ms should be displayed
    _max_x_axis_val_ms = max_num_of_points_in_buffer; 

    // Allocate a vertex buffer object to store data for visualization
    AllocateSeriesVbo();

    // Allocate a vertex buffer object to store data for the lead line
    CreateLeadLineVbo();

    // Create vbo for the x and y axis
    SetupAxes();

    // Create vbo for the bounding box of the chart
    CreateBoundingBox();

    CreateSurfaceGrid(200, 5);

    _time_range_ms = _max_x_axis_val_ms - _min_x_axis_val_ms;

    _no_line_vertices.fill(NAN, max_num_of_points_in_buffer);
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

// Compare function to compare a timestamp_TP with a ChartPoint_TP (comparison is done with the underlying timestamp)
inline bool CmpTimestamps(const ChartPoint_TP<Position3D_TC<float>>& rhs, const Timestamp_TP& timestamp)
{
    return rhs._timestamp.GetMilliseconds() < timestamp.GetMilliseconds();
}

void OGLChart_C::AddDataToSeries(float y, float x_ms)
{
    // Don't add the value if its not inside the range, 
    // because its not visible eitherway -> better solution would be: Add it to the series but don't draw it!
     if ( y > _max_y_axis_value || y < _min_y_axis_value ){
         return;
     }

    // wrapp value around x-axis if the 'x_ms' value is bigger than maximum value of the x-axis
    if (_need_to_wrap_series) {
        _number_of_wraps++;
        _need_to_wrap_series = false;
	}

    // check if we need to wrap the data (when data series reached the right border of the screen)
    if (x_ms >= static_cast<float>(_max_x_axis_val_ms) * static_cast<float>(_number_of_wraps) ) {
		//calculate new x value at most left chart position
        _need_to_wrap_series = true;
	}

    // - (minus) because then the positive y axis is directing at the top of the screen
    float y_val_scaled_S = static_cast<float>(_screen_pos_y_S + _height_S) - 
        ( (y - _min_y_axis_value) / (_max_y_axis_value - _min_y_axis_value) ) * _height_S;
    // assert( y_val_scaled_S < _max_y_axis_value && y_val_scaled_S > _min_y_axis_value);
    DEBUG("Added scaled y value: "<< y_val_scaled_S << ", from raw value: " << y);

    // + so the data value runs from left to right side
    // calculate new x-index when the dataseries has reached the left border of the plot
    float x_val_wrap_corrected_ms = x_ms - static_cast<float>(_max_x_axis_val_ms) * static_cast<float>(_number_of_wraps - 1);

    // calculate x-value after wrapping
    float x_val_scaled_S = static_cast<float>(_screen_pos_x_S) + 
        ( (x_val_wrap_corrected_ms - _min_x_axis_val_ms) / (_max_x_axis_val_ms - _min_x_axis_val_ms) ) * _width_S;

   // Construction of point fails -> internal the timestamp is always zero !
   ChartPoint_TP<Position3D_TC<float>> new_point(Position3D_TC<float>(x_val_scaled_S, y_val_scaled_S, 1.0f), x_ms);
   //_input_buffer_new.InsertAtTail({ x_val_scaled_S, y_val_scaled_S, 1.0f });
   _input_buffer_new.InsertAtTail(new_point);
}

// Problem: I need to lock when doing the binary search. Therefore i need the mutex inside the input_buffer 
//-> create the mutex inside OGLChart_C
// -> pass a reference to the mutex, which should be used inside the RingBuffer, on construction of the RingBUffer
// -> Lock before doing the binary search inside OGLChart
// Alternative : Refactor this function in the RingBuffer itself ! Then we can call this function on the ringbuffer -> should work!
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
    //if ( x_ms >= static_cast<float>(_max_x_axis_val_ms) * static_cast<float>(_number_of_wraps) ) {
   if ( x_ms_modulo >= static_cast<float>(_max_x_axis_val_ms) * static_cast<float>(_number_of_wraps) ) {  // Change modulo to x_ms
        // Calculate new x value at most left chart position
        _need_to_wrap_series = true;
        _dataseries_wrapped_once = true;
    }

    // - (minus) because then the positive y axis is directing at the top of the screen
    float y_val_scaled_S = static_cast<float>(_screen_pos_y_S + _height_S) -
        ((value - _min_y_axis_value) / (_max_y_axis_value - _min_y_axis_value)) * _height_S;

    // assert( y_val_scaled_S < _max_y_axis_value && y_val_scaled_S > _min_y_axis_value);
    DEBUG("Scaled y value: " << y_val_scaled_S << ", to value: " << value);
    
    // - so the data value runs from left to right side
    float x_val_wrap_corrected_ms = x_ms_modulo - static_cast<float>(_max_x_axis_val_ms) * static_cast<float>(_number_of_wraps - 1);     // Change modulo to x_ms

    // calculate x-value after wrapping
    float x_val_scaled_S = static_cast<float>(_screen_pos_x_S) +
                           ( (x_val_wrap_corrected_ms - _min_x_axis_val_ms) / (_max_x_axis_val_ms - _min_x_axis_val_ms) ) * 
                           _width_S;

    _input_buffer_new.InsertAtTail(ChartPoint_TP<Position3D_TC<float>>(Position3D_TC<float>(x_val_scaled_S, y_val_scaled_S, 1.0f), x_ms));
    DEBUG("Scaled x value: " << x_val_scaled_S << ", to value: " << x_ms);
}


float OGLChart_C::GetScreenCoordsFromYChartValue(float y_value) 
{
   float y_value_S =  static_cast<float>(_screen_pos_y_S + _height_S) -
        ((y_value - _min_y_axis_value) / (_max_y_axis_value - _min_y_axis_value)) * _height_S;

   return y_value_S;
}

float OGLChart_C::GetScreenCoordsFromXChartValue(float x_value_ms)
{
    // calculate new x-index when the dataseries has reached the left border of the plot
    float x_val_wrap_corrected_ms = x_value_ms - static_cast<float>(_max_x_axis_val_ms) * static_cast<float>(_number_of_wraps - 1);

    // calculate x-value after wrapping
    float x_value_S = static_cast<float>(_screen_pos_x_S) +
        ((x_val_wrap_corrected_ms - _min_x_axis_val_ms) / (_max_x_axis_val_ms - _min_x_axis_val_ms)) * _width_S;

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
        // Remove old data
        RemoveOutdatedDataInsideVBO();
    }
}

// MAke sure the buffer is bound to the current context before calling this function
void OGLChart_C::WriteToVbo(QVector<float>& data)
{
    int number_of_new_data_bytes = static_cast<int>(data.size()) * static_cast<int>(sizeof(float));
 
    if (_vbo_current_series_idx + number_of_new_data_bytes <=_vbo_buffer_size) {

        // The data can completely fit into the vbo => Write data to the vbo (at the right side of the one dimensional memory)
        _chart_vbo.write(static_cast<int>(_vbo_current_series_idx), data.constData(), number_of_new_data_bytes);
        
        // new write offset in bytes
        _vbo_current_series_idx += number_of_new_data_bytes;
        
        // Count points; stop counting points after one wrap
        // (because after a wrap the point count stays the same-> _max_x_axis_value)
        if (!_dataseries_wrapped_once) {
            _point_count += data.size() / 3;
        }

    } else {
        // buffer is full or not all new data can fit into it; 
        // reset buffer index and start overwriting data at the beginning
        // Calculate how much bytes can fit into the buffer until the end is reached
        int number_of_free_bytes_until_end = _vbo_buffer_size - _vbo_current_series_idx;
        int bytes_to_write_at_beginning = number_of_new_data_bytes - number_of_free_bytes_until_end;
        int bytes_to_write_until_end = number_of_new_data_bytes - bytes_to_write_at_beginning;

        if( number_of_free_bytes_until_end > 0 /* > sizeof(float) * 3 (If its possible to write one vertex at the end)*/ ) {
            // Write data until the end of the buffer is reached
           _chart_vbo.write(static_cast<int>(_vbo_current_series_idx), data.data(), bytes_to_write_until_end);
           if ( !_dataseries_wrapped_once ) {
               _point_count += number_of_free_bytes_until_end / sizeof(float) / 3;
           }
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
    //DrawSurfaceGrid();
}


// Todo: x-axes should always be at the position where the chart hast the value zero at the y axis.
void OGLChart_C::SetupAxes() 
{
    const auto axes_vertices = CreateAxesVertices(5.0);
    // Todo dont create another copy...this is a scope problem! because the struct does not exist outside the function -> solution-> store axes in members
    auto x_axis_vertices = axes_vertices._x_axis_vertices;
    auto y_axis_vertices = axes_vertices._y_axis_vertices;

    //// Combined vertices
    //QVector<float> axis_vertices(x_axis_vertices);
    //for ( const auto& vertice : y_axis_vertices ) {
    //    axis_vertices.push_back(vertice);
    //}
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
    // x_major_tick_dist_ms and y_major_tick_dist_unit = major ticks in the same unit like the member values : min_y_axis_value and max_y_axis_value
    //p1(from)--------------------------------------------->>>>
    // |                                            |
    // |                                            |
    // ----------------------------------------------
    int number_of_vertical_grid_lines = (_max_y_axis_value- _min_y_axis_value) / y_major_tick_dist_unit;

    float p1_x_from = _screen_pos_x_S;
    float p1_y_from = _screen_pos_y_S;
    float chart_z_value = 1.0f;

    // Creaete vertices
    QVector<float> vertical_grid_lines;
    float y_axis_major_tick_value = _max_y_axis_value;
    for ( int line_idx = 0; line_idx < number_of_vertical_grid_lines; ++line_idx ) {
        float major_tick_y_pos_S = GetScreenCoordsFromYChartValue(y_axis_major_tick_value);
        y_axis_major_tick_value -= y_major_tick_dist_unit;
         // Point from
        vertical_grid_lines.push_back(p1_x_from);
        vertical_grid_lines.push_back(major_tick_y_pos_S);
        vertical_grid_lines.push_back(chart_z_value);
        // Point to
        vertical_grid_lines.push_back(p1_x_from + _width_S);
        vertical_grid_lines.push_back(major_tick_y_pos_S);
        vertical_grid_lines.push_back(chart_z_value);
    }

    // Create horizontal line vertices
    // unit - in milliseconds
    int number_of_horizontal_grid_lines = (_max_x_axis_val_ms - _min_x_axis_val_ms) / x_major_tick_dist_ms;
    
    float x_axis_major_tick_value = _max_x_axis_val_ms;
    for ( int line_idx = 0; line_idx < number_of_horizontal_grid_lines; ++line_idx ) {
        float major_tick_x_pos_S = GetScreenCoordsFromXChartValue(x_axis_major_tick_value);
        x_axis_major_tick_value -= x_major_tick_dist_ms;
        // Point from
        vertical_grid_lines.push_back(major_tick_x_pos_S);
        vertical_grid_lines.push_back(p1_y_from);
        vertical_grid_lines.push_back(chart_z_value);
        // Point to
        vertical_grid_lines.push_back(major_tick_x_pos_S);
        vertical_grid_lines.push_back(p1_y_from + _height_S);
        vertical_grid_lines.push_back(chart_z_value);
    }

    _num_of_surface_grid_vertices = vertical_grid_lines.size() / 3;

    // Create VBO
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    // Setup OGL Chart buffer - empty 
    _surface_grid_vbo.create();
    _surface_grid_vbo.bind();
    f->glEnableVertexAttribArray(0);
    // 3 positions for x and y and z data coordinates
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    _surface_grid_vbo.allocate(vertical_grid_lines.constData(), vertical_grid_lines.size() * static_cast<int>(sizeof(float)));
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
    float z_pos = 1.0f;
    _lead_line_vertices.resize(buffer_size);
    // point from:
    // y value
    _lead_line_vertices[1] = _screen_pos_y_S;
    // z value
    _lead_line_vertices[2] = z_pos;
    // point to:
    // y value
    _lead_line_vertices[4] = _screen_pos_y_S + _height_S;
    // z value
    _lead_line_vertices[5] = z_pos;
}

void OGLChart_C::UpdateLeadLinePosition(float x_value_new) 
{
    _lead_line_vertices[0] = x_value_new;
    _lead_line_vertices[3] = x_value_new;
    // Write the whole vbo...alternative: write just two values, but because of caching this should not really result in differences..
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
    // f->glDrawArrays(GL_QUADS, 0, 4);
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

    this->OnChartUpdate();
    // Draw inside the current context
    f->glEnableVertexAttribArray(0);
    //f->glEnableVertexAttribArray(1);
    // each point (GL_POINT) consists of 3 components (x, y, z)
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    // to get the abs number of points-> divide through count of each Point
    f->glDrawArrays(GL_LINE_STRIP, 0, _point_count);

    //if ( _remove_series_idx < _vbo_current_series_idx / 3 / sizeof(float) ) {
    //    f->glDrawArrays(GL_LINE_STRIP, _remove_series_idx, _point_count);
    //}
    //else {

    //    f->glDrawArrays(GL_LINE_STRIP, _remove_series_idx, _point_count);
    //    f->glDrawArrays(GL_LINE_STRIP, 0, _point_count);
    //}

    f->glDisableVertexAttribArray(0);
    _chart_vbo.release();
}

void OGLChart_C::CreateBoundingBox()
{
    // calculate corner points of the bounding box
    float bottom_left_x = _screen_pos_x_S;
    float bottom_left_y = _screen_pos_y_S + _height_S;

    float bottom_right_x = _screen_pos_x_S + _width_S;
    float bottom_right_y = _screen_pos_y_S + _height_S;

    float top_left_x = _screen_pos_x_S;
    float top_left_y = _screen_pos_y_S;

    float top_right_x = _screen_pos_x_S + _width_S;
    float top_right_y = _screen_pos_y_S;

    // Create vertices to draw four lines inside the opengl z-plane
    QVector<float> bb_vertices;

    // Draw bottom side
    // Bottom right corner
    bb_vertices.push_back(bottom_right_x);
    bb_vertices.push_back(bottom_right_y);
    bb_vertices.push_back(_screen_pos_z_S);
    // Bottom left corner
    bb_vertices.push_back(bottom_left_x);
    bb_vertices.push_back(bottom_left_y);
    bb_vertices.push_back(_screen_pos_z_S);

    // Draw right side 
    // Bottom right corner
    bb_vertices.push_back(bottom_right_x);
    bb_vertices.push_back(bottom_right_y);
    bb_vertices.push_back(_screen_pos_z_S);
    // Top right corner
    bb_vertices.push_back(top_right_x);
    bb_vertices.push_back(top_right_y);
    bb_vertices.push_back(_screen_pos_z_S);

    // Draw top side
    // Top left corner
    bb_vertices.push_back(top_left_x);
    bb_vertices.push_back(top_left_y);
    bb_vertices.push_back(_screen_pos_z_S);
    // Top right corner
    bb_vertices.push_back(top_right_x);
    bb_vertices.push_back(top_right_y);
    bb_vertices.push_back(_screen_pos_z_S);

    // Draw left side 
    // Top left corner
    bb_vertices.push_back(top_left_x);
    bb_vertices.push_back(top_left_y);
    bb_vertices.push_back(_screen_pos_z_S);
    // Bottom left corner
    bb_vertices.push_back(bottom_left_x);
    bb_vertices.push_back(bottom_left_y);
    bb_vertices.push_back(_screen_pos_z_S);

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


const XYAxisVertices_TP OGLChart_C::CreateAxesVertices(float size_S)
{
    // Todo no fixed values..needs to be adjustable from the outside through access functions
    // The user of the chart does not need to know anything from opengl so i could make an abstraction
    // But the openGl code depends on the chart type (hist, lines, points,..)
    float x_axis_width_S = _width_S;
    float x_axis_height_S = size_S;
    float y_axis_height = _height_S;
    float y_axis_width = size_S;
    float axis_pos_z = 1.0f;

    // Triangles
    QVector<float> x_axis_vertices;
    // 2d drawing of x axis
    //P1--------------------------------------------------- P4
    // |													| <- height
    //P2--------------------------------------------------- P3
    //						width

    float p1_x = _screen_pos_x_S;
    float p1_y = _screen_pos_y_S + _height_S / 2;

    float p2_x = _screen_pos_x_S;
    float p2_y = p1_y + x_axis_height_S;

    float p3_x = p2_x + x_axis_width_S;;
    float p3_y = p2_y;

    float p4_x = p1_x + x_axis_width_S;
    float p4_y = p1_y;
    // first triang P1-P2-P4
    // P1
    x_axis_vertices.push_back(p1_x);
    x_axis_vertices.push_back(p1_y);
    x_axis_vertices.push_back(axis_pos_z);
    // P2
    x_axis_vertices.push_back(p2_x);
    x_axis_vertices.push_back(p2_y);
    x_axis_vertices.push_back(axis_pos_z);
    // P4
    x_axis_vertices.push_back(p4_x);
    x_axis_vertices.push_back(p4_y);
    x_axis_vertices.push_back(axis_pos_z);

    // second triang P3 - P4 - P2
    // P3
    x_axis_vertices.push_back(p3_x);
    x_axis_vertices.push_back(p3_y);
    x_axis_vertices.push_back(axis_pos_z);
    // P4
    x_axis_vertices.push_back(p4_x);
    x_axis_vertices.push_back(p4_y);
    x_axis_vertices.push_back(axis_pos_z);
    // P2
    x_axis_vertices.push_back(p2_x);
    x_axis_vertices.push_back(p2_y);
    x_axis_vertices.push_back(axis_pos_z);

    // Triangles
    QVector<float> y_axis_vertices;
    // 2d drawing of y axis
    // width
    // P1|--|P4
    //	 |  |
    //	 |  |
    //	 |  |
    //	 |  | } length
    //	 |  |
    //	 |  |
    //	 |  |
    // P2|--|P3
    // Calculate points for y axis
    p1_x = _screen_pos_x_S;
    p1_y = _screen_pos_y_S;
    p2_x = p1_x;
    p2_y = _screen_pos_y_S + y_axis_height;
    p3_x = p1_x + y_axis_width;
    p3_y = p2_y;
    p4_x = p1_x + y_axis_width;
    p4_y = p1_y;
    // first triang: P1 - P2 - P4
    // P1
    y_axis_vertices.push_back(p1_x);
    y_axis_vertices.push_back(p1_y);
    y_axis_vertices.push_back(axis_pos_z);
    // P2
    y_axis_vertices.push_back(p2_x);
    y_axis_vertices.push_back(p2_y);
    y_axis_vertices.push_back(axis_pos_z);
    // P4
    y_axis_vertices.push_back(p4_x);
    y_axis_vertices.push_back(p4_y);
    y_axis_vertices.push_back(axis_pos_z);

    // second triang: P3 - P4 - P2
    // P3
    y_axis_vertices.push_back(p3_x);
    y_axis_vertices.push_back(p3_y);
    y_axis_vertices.push_back(axis_pos_z);
    // P4
    y_axis_vertices.push_back(p4_x);
    y_axis_vertices.push_back(p4_y);
    y_axis_vertices.push_back(axis_pos_z);
    // P2
    y_axis_vertices.push_back(p2_x);
    y_axis_vertices.push_back(p2_y);
    y_axis_vertices.push_back(axis_pos_z);

   return XYAxisVertices_TP(x_axis_vertices, y_axis_vertices);
}

// Old, unused code
static void shiftData(double *data, int len, double newValue)
{
    memmove(data, data + 1, sizeof(*data) * (len - 1));
    data[len - 1] = newValue;
}

//void OGLChart::drawNewestData(){}
//void OGLChart::deleteDataFromBeginning(int count) {}
//void OGLChart::updateChart() {}

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
