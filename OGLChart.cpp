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
    delete _text_painter;
}

// This is the specialization - pointchart realtime
OGLChart_C::OGLChart_C(int max_num_of_points_in_buffer,
                       int screen_pos_x,
                       int screen_pos_y,
                       int chart_width_S,
                       int chart_height_S, 
                       QOpenGLWidget& parent)
    : _vbo_buffer_size( max_num_of_points_in_buffer * 3 * sizeof(float) ),    // vbo buffer size(3 times the point buffer size) each point consists of 3 floats - i could make this template but then i can only do header files.
      _vbo_series_idx(0),
      _chart_vbo(QOpenGLBuffer::VertexBuffer),
      _x_axis_vbo(QOpenGLBuffer::VertexBuffer),
      _y_axis_vbo(QOpenGLBuffer::VertexBuffer),
     _bb_vbo(QOpenGLBuffer::VertexBuffer),
     _surface_grid_vbo(QOpenGLBuffer::VertexBuffer),
     _lead_line_vbo(QOpenGLBuffer::VertexBuffer),
     _input_buffer(max_num_of_points_in_buffer),
     _input_buffer_new(max_num_of_points_in_buffer),
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
    _max_x_axis_val_ms = max_num_of_points_in_buffer; // 10000

    // Allocate a vertex buffer object to store data for visualization
    AllocateSeriesVbo();

    // Allocate a vertex buffer object to store data for the lead line
    CreateLeadLineVbo();

    // Create vbo for the x and y axis
    SetupAxes();

    // Create vbo for the bounding box of the chart
    CreateBoundingBox();

    CreateSurfaceGrid(200, 5);

    _text_painter = new QPainter();
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
        //_input_buffer.InsertAtHead(NAN, NAN, NAN);
        //_dataseries_wrapped_once = true;
	}

    // - (minus) because then the positive y axis is directing at the top of the screen
    float y_val_scaled_S = static_cast<float>(_screen_pos_y_S + _height_S) - 
        ( (y - _min_y_axis_value) / (_max_y_axis_value - _min_y_axis_value) ) * _height_S;
    // assert( y_val_scaled_S < _max_y_axis_value && y_val_scaled_S > _min_y_axis_value);
    DEBUG("Scaled y value: "<< y_val_scaled_S << ", to value: " << y);

    // + so the data value runs from left to right side
    // calculate new x-index when the dataseries has reached the left border of the plot
    float x_val_wrap_corrected_ms = x_ms - static_cast<float>(_max_x_axis_val_ms) * static_cast<float>(_number_of_wraps - 1);

    // calculate x-value after wrapping
    float x_val_scaled_S = static_cast<float>(_screen_pos_x_S) + 
        ( (x_val_wrap_corrected_ms - _min_x_axis_val_ms) / (_max_x_axis_val_ms - _min_x_axis_val_ms) ) * _width_S;

    //_input_buffer.InsertAtHead(x_val_scaled_S, y_val_scaled_S, 1.0f);
    _input_buffer_new.InsertAtTail(x_val_scaled_S, y_val_scaled_S, 1.0f);
}

void OGLChart_C::AddData(float value, Timestamp_TP & timestamp)
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

   int x_ms = timestamp.GetMilliseconds();
   int timeRange = (_max_x_axis_val_ms - _min_x_axis_val_ms);
       
    // check if we need to wrap the data (when data series reached the right border of the screen)
    //if ( x_ms >= static_cast<float>(_max_x_axis_val_ms) * static_cast<float>(_number_of_wraps) ) {
   if ( x_ms >= static_cast<float>(_max_x_axis_val_ms) * static_cast<float>(_number_of_wraps) ) {
        // Calculate new x value at most left chart position
        _need_to_wrap_series = true;
        _dataseries_wrapped_once = true;
    }

    // - (minus) because then the positive y axis is directing at the top of the screen
    float y_val_scaled_S = static_cast<float>(_screen_pos_y_S + _height_S) -
        ((value - _min_y_axis_value) / (_max_y_axis_value - _min_y_axis_value)) * _height_S;

    // assert( y_val_scaled_S < _max_y_axis_value && y_val_scaled_S > _min_y_axis_value);
    DEBUG("Scaled y value: " << y_val_scaled_S << ", to value: " << value);
    
    // + so the data value runs from left to right side
    // calculate new x-index when the dataseries has reached the left border of the plot
    float x_val_wrap_corrected_ms = x_ms - static_cast<float>(_max_x_axis_val_ms) * static_cast<float>(_number_of_wraps - 1);

    // calculate x-value after wrapping
    float x_val_scaled_S = static_cast<float>(_screen_pos_x_S) +
        ((x_val_wrap_corrected_ms - _min_x_axis_val_ms) / (_max_x_axis_val_ms - _min_x_axis_val_ms)) * _width_S;

    DEBUG("Scaled x value: " << x_val_scaled_S << ", to value: " << x_ms);
    _input_buffer.InsertAtHead(x_val_scaled_S, y_val_scaled_S, 1.0f);
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


void OGLChart_C::UpdateVbo()
{
    if( !_input_buffer.NewDataToRead() ){
        return;
    }

    // Get latest data from the input buffer
    auto latest_data = _input_buffer.ReadLatest();

    if ( !latest_data.empty() ) {
        // Todo: Spare this transformation to QVector by returning QVector directly from the input buffer
        QVector<float> additional_point_vertices;
        for (const auto& element : latest_data) {
            //DEBUG(element);

            additional_point_vertices.append(element._x);
            additional_point_vertices.append(element._y);
            additional_point_vertices.append(element._z);

            if ( element._y < _last_plotted_y_value_S * 0.80f || element._y > _last_plotted_y_value_S * 1.2f ) {
                std::cout << "value out of range:" << "value = " << element._y  
                                                 << ", last value = " << _last_plotted_y_value_S << std::endl;
            }
            _last_plotted_y_value_S = element._y;
        }
        _last_plotted_y_value_S = (latest_data.end()-1)->_y;
        _last_plotted_x_value_S = (latest_data.end()-1)->_x;



        // Write data to the vbo
        WriteToVbo(additional_point_vertices);
    }
}

void OGLChart_C::UpdateVBONew() {

    if ( _input_buffer_new.IsBufferEmpty() ) {
        return;
    }

    // Get latest data from the input buffer
    auto latest_data = _input_buffer_new.PopLatest();

    if ( !latest_data.empty() ) {
        // Todo: Spare this transformation to QVector by returning QVector directly from the input buffer
        QVector<float> additional_point_vertices;

        for ( const auto& element : latest_data ) {
            //DEBUG(element);
            additional_point_vertices.append(element._x);
            additional_point_vertices.append(element._y);
            additional_point_vertices.append(element._z);
            //if ( element._y < _last_plotted_y_value_S * 0.80f || element._y > _last_plotted_y_value_S * 1.2f ) {
            //    std::cout << "value out of range:" << "value = " << element._y
            //        << ", last value = " << _last_plotted_y_value_S << std::endl;
            //}
            //_last_plotted_y_value_S = element._y;
        }

        _last_plotted_y_value_S = (latest_data.end() - 1)->_y;
        _last_plotted_x_value_S = (latest_data.end() - 1)->_x;



        // Write data to the vbo
        WriteToVbo(additional_point_vertices);
    }
}

// MAke sure the buffer is bound to the current context before calling this function
void OGLChart_C::WriteToVbo(/*const*/ QVector<float>& data)
{
    int number_of_new_data_bytes = static_cast<int>(data.size()) * static_cast<int>(sizeof(float));
    // The data can completely fit into the vbo => Write data to the vbo
    if (_vbo_series_idx <=_vbo_buffer_size - number_of_new_data_bytes) {

        _chart_vbo.write(static_cast<int>(_vbo_series_idx), data.data(), number_of_new_data_bytes);
        // new write offset in bytes
        _vbo_series_idx += number_of_new_data_bytes;
        // Count points; stop counting points after one wrap
        // (because after a wrap the point count stays the same-> _max_x_axis_value)
        if (!_dataseries_wrapped_once) {
            _point_count += data.size() / 3;
        }

    } else {
        // buffer is full or not all new data can fit into it; 
        // reset buffer index and start overwriting data at the beginning
        // Calculate how much bytes can fit into the buffer until the end is reached
        int number_of_free_bytes_until_end = _vbo_buffer_size - _vbo_series_idx;
        int bytes_to_write_at_beginning = number_of_new_data_bytes - number_of_free_bytes_until_end;
        int bytes_to_write_until_end = number_of_new_data_bytes - bytes_to_write_at_beginning;

        std::cout << "chart data buffer full, number of datapoints(floats): " << _point_count
            << "\n" << "writing: " << number_of_new_data_bytes << " bytes" << "\n"
            << "bytes to write at beginning =" << bytes_to_write_at_beginning << "\n" 
            << "bytes to write until end =" << bytes_to_write_until_end << "\n"
            << "vbo index (before writing at beginning) =" << _vbo_series_idx << "\n";

        if( number_of_free_bytes_until_end > 0 ) {
            // Write data until the end of the buffer is reached
           _chart_vbo.write(static_cast<int>(_vbo_series_idx), data.data(), bytes_to_write_until_end);
           if ( !_dataseries_wrapped_once ) {
               _point_count += number_of_free_bytes_until_end / sizeof(float) / 3;
               std::cout << "------ Final pointcount = " << _point_count << "------" << std::endl;
           }
        }
        _dataseries_wrapped_once = true;
        //assert(_point_count == _max_x_axis_val_ms);
        //assert(_vbo_series_idx + bytes_to_write_until_end == _vbo_buffer_size);
        // Reset the index to continue writing the rest of the data at the beginning
        _vbo_series_idx = 0;

        // NAN ends the line strip. otherwise opengl continues to draw the line strip 
        // from the right side of the screen to the left side of the screen.
        int offset_in_num_points = ( bytes_to_write_until_end / sizeof(float) ) + 3 ;
        data.insert(offset_in_num_points, NAN);
        data.insert(offset_in_num_points, NAN);
        data.insert(offset_in_num_points, NAN);

        // If there is still data which was not written into the vbo because it did not fit at the end
        // => Continue writing the rest of the data at the beginning of the vbo
        if ( bytes_to_write_at_beginning > 0 ) {
            std::cout << "!!Writing line strip stopp command !!" << std::endl;
            // Increment because of the end of line strip command ( 3 x NAN )
            // bytes_to_write_at_beginning += 3 * sizeof(float);

            int data_memory_offset = (bytes_to_write_until_end / sizeof(float)); 
            _chart_vbo.write(static_cast<int>(_vbo_series_idx), (data.data() + data_memory_offset), bytes_to_write_at_beginning);
            _vbo_series_idx += bytes_to_write_at_beginning;
            std::cout << "starting at position (inside data vec): " << data_memory_offset << "\n";
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

//// Draw the axis details (units)
//bool success = _text_painter->begin(&_parent_widget);
//if ( success ) {
//    _text_painter->beginNativePainting();
//    _text_painter->setRenderHint(QPainter::TextAntialiasing);
//    QPen pen; pen.setColor(QColor(255, 255, 255));
//    _text_painter->setPen(pen);
//    _text_painter->setFont(QFont("times", 18));
//    //Draw FPS Counter
//    _text_painter->drawText(QPoint(1, 25), QString(QString::number(100000)) );
//    //Draw Hit Counter
//    _text_painter->endNativePainting();
//    //_text_painter->end();
//}
//_text_painter->end();
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

    //_num_of_surface_grid_vertices += horizontal_grid_lines.size() / 3;

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
    //this->UpdateVbo();
    this->UpdateVBONew();
    // Draw inside the current context
    f->glEnableVertexAttribArray(0);
    //f->glEnableVertexAttribArray(1);
    // each point (GL_POINT) consists of 3 components (x, y, z)
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    // to get the abs number of points-> divide through count of each Point
    f->glDrawArrays(GL_LINE_STRIP, 0, _point_count);
   
    //f->glDrawArrays(GL_LINE_STRIP, 0, _chart_vbo.);
    //f->glDisableVertexAttribArray(1);
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
