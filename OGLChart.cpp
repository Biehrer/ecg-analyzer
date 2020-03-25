#include <OGLChart.h>


OGLChart::~OGLChart()
{
    _chart_vbo.destroy();
    _y_axis_vbo.destroy();
    _x_axis_vbo.destroy();
    delete _data_series_buffer;
}

// This is the specialization - pointchart realtime
OGLChart::OGLChart(int bufferSize,
                   int screenPosX,
                   int screenPosY,
                   int chartWidth_S,
                   int chartHeight_S,
                   int chartID)
    : _bufferSize( bufferSize * 3 * sizeof(float) ),    // each point consists of 3 floats - i could make this template but then i can only do header files.
      _vbo_series_idx(0),
      _chart_vbo(QOpenGLBuffer::VertexBuffer),
      _x_axis_vbo(QOpenGLBuffer::VertexBuffer),
      _y_axis_vbo(QOpenGLBuffer::VertexBuffer),
      _series_buffer_idx(0)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    // Sweep-chart parameters
    _number_of_wraps = 1;
    _need_to_wrap_series = false;
    _dataseries_wrapped_once = false;

    _chartId = chartID;
    //buffer which collects data at CPU side for OGL
    _data_series_buffer = new float[TEMP_BUFFER_SIZE];

    //number of visualized points in the graph
    _point_count = 0;
    //where to place Chart in respect to the OGL Viewport
    screen_pos_x_S = screenPosX;
    _screen_pos_y_S = screenPosY;

    _width_S = chartWidth_S;
    _height_S = chartHeight_S;
    //chart values x and y axes
    _minY = 0;
    _maxY = 10;
    _min_x_val_ms = 0;
    //10 secs of data - buffersize(OGL buffer) needs to be this big! NEEDS TO MATCH WITH THE BUFFER SIZE THE USER USES AS INPUT(NUMBER OF VALUES IN BUFFER) -> therefore create the buffer variable dependent on how many ms should be displayed
    _max_x_val_ms = 10000;
    //_maxX_ms = _bufferSize;

    //Setup OGL Chart buffer - empty
    _chart_vbo.create();
    _chart_vbo.bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);		//3 positions for x and y and z data coordinates
    _chart_vbo.allocate(nullptr, _bufferSize);
    _chart_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    f->glDisableVertexAttribArray(0);
    _chart_vbo.release();

    SetupAxes();
}



static void shiftData(double *data, int len, double newValue)
{
	memmove(data, data + 1, sizeof(*data) * (len - 1));
	data[len - 1] = newValue;
}

bool timerange_wrapped = false;

void OGLChart::addData(float y, float x_ms)
{
    // wrapp value around x-axis if the 'x_ms' value is bigger than maximum value of the x-axis
    if (_need_to_wrap_series) {
        _number_of_wraps++;
        _need_to_wrap_series = false;
	}

    // save raw data - or not? is this the duty of the plot itself or should the user be able to decide whether he wants to safe displayed data..
    // check if we need to wrap the data (when data series reached the right border of the screen)
    if (x_ms >= static_cast<float>(_max_x_val_ms) * static_cast<float>(_number_of_wraps) ) {
		//calculate new x value at most left chart position
        _need_to_wrap_series = true;
        _dataseries_wrapped_once = true;
	}

    // chart_pos_screen_coords * scale_factor_screen_cords

    // - because then the positive y axis is directing at the top of the screen
    float y_val_scaled_S = static_cast<float>(_screen_pos_y_S) - (y * ( static_cast<float>(_height_S) / (_maxY - _minY) ) );

    // + so the data value runs from left to right side
    // calculate new x-index when the dataseries has reached the left border of the plot
    float x_val_wrap_corrected_ms = static_cast<float>(x_ms) - static_cast<float>(_max_x_val_ms) * static_cast<float>(_number_of_wraps - 1);

    // calculate value after wrapping-> -1 because we start wrapped number at 1 -
    // put x on right position of screen
    float x_val_scaled_ms_S = static_cast<float>(screen_pos_x_S) + (x_val_wrap_corrected_ms * (static_cast<float>(_width_S) / (_max_x_val_ms - _min_x_val_ms)));
	//add modified(in range) data to tempbuffer
	//only needed if we are collecting data before sending it to ogl

    assert(_series_buffer_idx <= TEMP_BUFFER_SIZE && _series_buffer_idx >= 0 );

    float z_depth = 1.0;
    _data_series_buffer[_series_buffer_idx] = x_val_scaled_ms_S;
    _data_series_buffer[_series_buffer_idx + 1] = y_val_scaled_S;
    _data_series_buffer[_series_buffer_idx + 2] = z_depth;

    // std::cout << "Point #" << _pointCount << ": " << x_inRange_ms_S << ", " << y_inRange_S << ", " << z_depth << std::endl;
    _series_buffer_idx += 3;
}


void OGLChart::WriteSeriesToVBO()
{
    // If there was no new data added
    // => dont write to the vbo
    if(_series_buffer_idx == 0){
        return;
    }
    // Bind VBO before
    // create plot point
    QVector<float> additional_point_vertices;

    int number_of_new_points = 0;

    // data_lock->lock();
    // only create the new vertices if there was data added to the chart
    while( number_of_new_points < _series_buffer_idx ) {
        additional_point_vertices.append(_data_series_buffer[number_of_new_points]);
        additional_point_vertices.append(_data_series_buffer[number_of_new_points + 1]);
        additional_point_vertices.append(_data_series_buffer[number_of_new_points + 2]); // z-coord : 0
        number_of_new_points += 3;
        // check if the new data is bigger than the whole incoming buffer...
	}
	//std::cout << "number of points added: " << count << "chartID: " << _chartId << "\n\r";
	//data_lock->unlock();

    //reset series_buffer_idx which holds the data on cpu side now
    _series_buffer_idx = 0;

    if (number_of_new_points > 0) {	// only add data if there is new data which needs to be added
        if (_vbo_series_idx <= _bufferSize) {
            _chart_vbo.write(_vbo_series_idx, additional_point_vertices.data(), number_of_new_points * sizeof(float));
            _vbo_series_idx += additional_point_vertices.size() * sizeof(float);	//offset in bytes
            // stop counting points after one wrap (because after a wrap the point count stays the same)
            if (!_dataseries_wrapped_once) {
                _point_count += number_of_new_points / 3;
			}
        } else { //buffer is full...reset buffer and start overwriting data at the beginning
            std::cout << "chart data buffer full, number of datapoints(floats): " << _point_count << "chartID: " << _chartId <<"..restart writing at beginning" << "\n\r";
            _vbo_series_idx = 0;
		}
	}
}

void OGLChart::Draw()
{
    auto *f = QOpenGLContext::currentContext()->functions();

    // Bind buffer and send data to the gpu
    _chart_vbo.bind();
    this->WriteSeriesToVBO();

    // Draw
	f->glEnableVertexAttribArray(0);
	//f->glEnableVertexAttribArray(1);
    // each point (GL_POINT) consists of 3 components (x, y, z)
	f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    // to get the abs number of points-> divide through count of each Point
    f->glDrawArrays(GL_POINTS, 0, _point_count);
	//f->glDisableVertexAttribArray(1);
	f->glDisableVertexAttribArray(0);
    _chart_vbo.release();
    DrawXYAxes();
}


void OGLChart::SetupAxes() {
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    const auto axes_vertices = CreateAxesVertices();

    // Todo dont create another copy...this is a scope problem! because the struct does not exist outside the function
    auto x_copy = axes_vertices._x_axis_vertices;
    auto y_copy = axes_vertices._y_axis_vertices;

	//Setup OGL Chart buffer - empty 
    _x_axis_vbo.create();
    _x_axis_vbo.bind();
	f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    _x_axis_vbo.allocate(x_copy.constData(), x_copy.size() * sizeof(float));
    _x_axis_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	f->glDisableVertexAttribArray(0);
    _x_axis_vbo.release();

	//Setup OGL Chart buffer - empty 
    _y_axis_vbo.create();
    _y_axis_vbo.bind();
	f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);		//3 positions for x and y and z data coordinates
    _y_axis_vbo.allocate(y_copy.constData(), y_copy.size() * sizeof(float));
    _y_axis_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	f->glDisableVertexAttribArray(0);
    _y_axis_vbo.release();

}
void OGLChart::DrawXYAxes()
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

const XYAxisVertices_TP OGLChart::CreateAxesVertices()
{
    // Todo no fixed values..needs to be adjustable from the outside through access functions
    // The user of the chart does not need to know anything from opengl so i could make an abstraction
    // But the openGl code depends on the chart type (hist, lines, points,..)
    float x_axis_width_S = _width_S;
    float x_axis_height_S = 5.0;
    float y_axis_length = _height_S;
    float y_axis_width = 5.0;
    float axis_pos_z = 1.0f;

    // triangle approach
    QVector<float> x_axis_vertices;

    // 2d drawing of x axis
    //P1--------------------------------------------------- P4
    // |													| <- height
    //P2--------------------------------------------------- P3
    //						length

    // first triang P1-P2-P4
    x_axis_vertices.push_back(screen_pos_x_S);
    x_axis_vertices.push_back(_screen_pos_y_S);
    x_axis_vertices.push_back(axis_pos_z);

    x_axis_vertices.push_back(screen_pos_x_S);
    x_axis_vertices.push_back(_screen_pos_y_S + x_axis_height_S);
    x_axis_vertices.push_back(axis_pos_z);

    x_axis_vertices.push_back(screen_pos_x_S + x_axis_width_S);
    x_axis_vertices.push_back(_screen_pos_y_S);
    x_axis_vertices.push_back(axis_pos_z);

    //second triang P3 - P4 - P2
    x_axis_vertices.push_back(screen_pos_x_S + x_axis_width_S);
    x_axis_vertices.push_back(_screen_pos_y_S + x_axis_height_S);
    x_axis_vertices.push_back(axis_pos_z);

    x_axis_vertices.push_back(screen_pos_x_S + x_axis_width_S);
    x_axis_vertices.push_back(_screen_pos_y_S);
    x_axis_vertices.push_back(axis_pos_z);

    x_axis_vertices.push_back(screen_pos_x_S);
    x_axis_vertices.push_back(_screen_pos_y_S + x_axis_height_S);
    x_axis_vertices.push_back(axis_pos_z);


    // end of triangle approach
/* GL_QUAD
    QVector<float> xAxis_verts;
//
//  ---------------------------------------------------
// |													| <- width
//  ---------------------------------------------------
//						length
    xAxis_verts.append(_chartPositionX_S);
    xAxis_verts.append(_chartPositionY_S);
    xAxis_verts.append(10.0);

    xAxis_verts.append(_chartPositionX_S);
    xAxis_verts.append(_chartPositionY_S + xAxis_width);
    xAxis_verts.append(10.0);

    xAxis_verts.append(_chartPositionX_S + xAxis_length);
    xAxis_verts.append(_chartPositionY_S + xAxis_width);
    xAxis_verts.append(10.0);

    xAxis_verts.append(_chartPositionX_S + xAxis_length);
    xAxis_verts.append(_chartPositionY_S);
    xAxis_verts.append(10.0);
    */

    QVector<float> y_axis_vertices;
    // 2d drawing of y axes
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


    // Triangles:

    // first triang: P1 - P2 - P3
    // P1
    y_axis_vertices.push_back(screen_pos_x_S);
    y_axis_vertices.push_back(_screen_pos_y_S);
    y_axis_vertices.push_back(axis_pos_z);

    // P2
    y_axis_vertices.push_back(screen_pos_x_S);
    y_axis_vertices.push_back(_screen_pos_y_S + y_axis_length);
    y_axis_vertices.push_back(axis_pos_z);

    // P3
    y_axis_vertices.push_back(screen_pos_x_S + y_axis_length);
    y_axis_vertices.push_back(_screen_pos_y_S + y_axis_width);
    y_axis_vertices.push_back(axis_pos_z);

    // second triang: P3 - P4 - P2
    // P3
    y_axis_vertices.push_back(screen_pos_x_S + y_axis_length);
    y_axis_vertices.push_back(_screen_pos_y_S + y_axis_width);
    y_axis_vertices.push_back(axis_pos_z);

    // P4
    y_axis_vertices.push_back(screen_pos_x_S + y_axis_length);
    y_axis_vertices.push_back(_screen_pos_y_S);
    y_axis_vertices.push_back(axis_pos_z);

    // P2
    y_axis_vertices.push_back(screen_pos_x_S);
    y_axis_vertices.push_back(_screen_pos_y_S + y_axis_length);
    y_axis_vertices.push_back(axis_pos_z);

//    // Quad..
//	//p1..
//    y_axis_vertices.append(_chart_pos_x_S);
//    y_axis_vertices.append(_chart_position_y_S);
//    y_axis_vertices.append(axis_pos_z);

//    // p2
//    y_axis_vertices.append(_chart_pos_x_S);
//    y_axis_vertices.append(_chart_position_y_S + y_axis_length);
//    y_axis_vertices.append(axis_pos_z);
//    // p3
//    y_axis_vertices.append(_chart_pos_x_S + y_axis_length);
//    y_axis_vertices.append(_chart_position_y_S + y_axis_width);
//    y_axis_vertices.append(axis_pos_z);

//	//..p4
//    y_axis_vertices.append(_chart_pos_x_S + y_axis_length);
//    y_axis_vertices.append(_chart_position_y_S);
//    y_axis_vertices.append(axis_pos_z);

   return XYAxisVertices_TP(x_axis_vertices, y_axis_vertices);
}


void OGLChart::drawNewestData(){}
void OGLChart::deleteDataFromBeginning(int count) {}
void OGLChart::updateChart() {}


void OGLChart::addRange(int count, QVector<double> data)
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
