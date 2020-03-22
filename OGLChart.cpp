#include <OGLChart.h>

static void shiftData(double *data, int len, double newValue)
{
	memmove(data, data + 1, sizeof(*data) * (len - 1));
	data[len - 1] = newValue;
}

bool timerange_wrapped = false;
	//We only need data form the latest time range.
	//double startTime = x_ms - _maxX_ms;//timestamps(current) - timerange - 20k
	//int startIndex = (int)ceil(Chart::bSearch(DoubleArray(m_timeStamps, m_currentIndex), startTime) - 0.1);
	// For a sweep chart, if the data overflow the right border (which should be a multipleof the time range), it will wrap back to the left.
	//double wrapTime = std::floor(startTime / _maxX_ms + 1) * _maxX_ms;
	//double wrapIndex = Chart::bSearch(DoubleArray(m_timeStamps, m_currentIndex), wrapTime);
	// First, draw the part of the line from the start time to the wrap time.
	//int lenA = (int)ceil(wrapIndex) - startIndex + 1;
	// Second, draw the part of the line from the wrap time to the latest time.
	//int wrapStart = (int)floor(wrapIndex);
	//int lenB = m_currentIndex - wrapStart;
	//double x_inRange_S = x + (float)_chartPositionX_S; //*  (_chartWidth_S / (_maxX - _minX)) + (float)_chartPositionX_S;
	//float x_inRange_S = x_ms + (float)_chartPositionX_S; //*  (_chartWidth_S / (_maxX - _minX)) + (float)_chartPositionX_S;

void OGLChart::addData(float y, float x_ms)
{
	//wrapp value around x-axis if dataTimestamp is bigger than max x-axes value
	if (needWrap) {
		wrapped_number++;
		needWrap = false;
	}

	//save raw data - or not? is this the duty of the plot itself or should the user be able to decide whether he wants to safe displayed data..
	//check if we need to wrap the data (when data series reached the right border of the screen)
    if (x_ms >= static_cast<float>(_maxX_ms) * static_cast<float>(wrapped_number) ) {
		//calculate new x value at most left chart position
		needWrap = true;
		_wrappedOnce = true;
	}
    // append new value
    // delete old values
    // map data to plot point to a specific position in the plot itself( not the window the plot is placed in)
    // modify data for visualization
    // - because then the positive y axis is directing at the top of the screen
    float y_inRange_S = static_cast<float>(_chartPositionY_S) - ( y *	 ( static_cast<float>(_chartHeight_S) / (_maxY - _minY) ) );

    // + so the data value runs from left to right side
    // calculate new x-index when the dataseries has reached the left border of the plot
    float x_ms_new = static_cast<float>(x_ms) - static_cast<float>(_maxX_ms) * static_cast<float>(wrapped_number - 1);

    //calculate value after wrapping-> -1 because we start wrapped number at 1 -
	//put x on right position of screen
    float x_inRange_ms_S = static_cast<float>(_chartPositionX_S) + (x_ms_new * (static_cast<float>(_chartWidth_S) / (_maxX_ms - _minX_ms)));
	//add modified(in range) data to tempbuffer
	//only needed if we are collecting data before sending it to ogl
	//data_lock->lock();
    assert(_tempBufferIndex <= TEMP_BUFFER_SIZE && _tempBufferIndex >= 0 );
    float z_depth = 1.0;

	_tempDataBuffer[_tempBufferIndex] = x_inRange_ms_S;
	_tempDataBuffer[_tempBufferIndex + 1] = y_inRange_S;
    _tempDataBuffer[_tempBufferIndex + 2] = z_depth;

    // std::cout << "Point #" << _pointCount << ": " << x_inRange_ms_S << ", " << y_inRange_S << ", " << z_depth << std::endl;

	_tempBufferIndex += 3;
}



void OGLChart::addRange(int count, QVector<double> data) 
{
	//void* pointerToData = chartVBO.mapRange(_bufferIndex, count, QOpenGLBuffer::RangeAccessFlag::RangeWrite);
	//chartVBO.write(_bufferIndex, data.constData(), count);
	//_bufferIndex += count;
	chartVBO.bind();
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
	auto point = chartVBO.map(QOpenGLBuffer::Access::ReadOnly);
	std::vector<float> dat; dat.resize(100);

	memcpy(&dat[0], &point, 100);
	chartVBO.release();
	//dat.toStdVector();
	//std::copy(pointerToData, *pointerToData+11,std::back_inserter(dat.toStdVector()));
}

void OGLChart::sendDataToOGL()
{
    // Bind VBO before
    // create plot point
	QVector<float> dataPoint;

	int count = 0;
	//data_lock->lock();
    while( count < _tempBufferIndex ) {
	dataPoint.append(_tempDataBuffer[count]);
	dataPoint.append(_tempDataBuffer[count + 1]);
	dataPoint.append(_tempDataBuffer[count + 2]); // z-coord : 0
	count += 3;
	//check if the new data is bigger than the whole incoming buffer...
	}
	//std::cout << "number of points added: " << count << "chartID: " << _chartId << "\n\r";
	//data_lock->unlock();

	//reset buffer(idx) now - all the data is now in dataPoint and in a few LOC's in OGL
	_tempBufferIndex = 0;
    if (count > 0) {	// only add data if there is new data which needs to be added
		if (_bufferIndex <= _bufferSize) {
			//chartVBO.bind();
			chartVBO.write(_bufferIndex, dataPoint.data(), count * sizeof(float));
			//chartVBO.release();
			_bufferIndex += dataPoint.size() * sizeof(float);	//offset in bytes
            // stop counting points after one wrap (because after a wrap the point count stays the same)
			if (!_wrappedOnce) {	
				_pointCount += count / 3;
			}
        } else { //buffer is full...reset buffer and start overwriting data at the beginning
			std::cout << "chart data buffer full, number of datapoints(floats): " << _pointCount << "chartID: " << _chartId <<"..restart writing at beginning" << "\n\r";
			_bufferIndex = 0;
		}
	}
}

void OGLChart::Draw()
{
    auto *f = QOpenGLContext::currentContext()->functions();

    chartVBO.bind();
	this->sendDataToOGL();//rename to: submit

	f->glEnableVertexAttribArray(0);
	//f->glEnableVertexAttribArray(1);
	f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	f->glDrawArrays(GL_POINTS, 0, _pointCount);	//each point consists of 3 items..to get the abs number of points-> divide through count of each Point
	//f->glDisableVertexAttribArray(1);
	f->glDisableVertexAttribArray(0);
	chartVBO.release();
	drawAxes();
}

OGLChart::~OGLChart() 
{
	chartVBO.destroy();
	_xAxisVBO.destroy();
	_yAxisVBO.destroy();
	delete _tempDataBuffer;
}

OGLChart::OGLChart(int bufferSize,
                   int screenPosX,
                   int screenPosY,
                   int chartWidth_S,
                   int chartHeight_S,
                   int chartID)
    : _bufferSize(bufferSize*3*sizeof(float)),
      _bufferIndex(0),
      chartVBO(QOpenGLBuffer::VertexBuffer),
      _xAxisVBO(QOpenGLBuffer::VertexBuffer),
      _yAxisVBO(QOpenGLBuffer::VertexBuffer),
      _tempBufferIndex(0)
{
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

	//Sweep-Chart parameters
	wrapped_number = 1;
	needWrap = false;
	_wrappedOnce = false;

	_chartId = chartID;
	//data_lock = new std::mutex();
	//buffer which collects data at CPU side for OGL
	_tempDataBuffer = new float[TEMP_BUFFER_SIZE];

	//number of visualized points in the graph
	_pointCount = 0;
	//where to place Chart in respect to the OGL Viewport
	_chartPositionX_S = screenPosX;
	_chartPositionY_S = screenPosY;

	_chartWidth_S = chartWidth_S;
	_chartHeight_S = chartHeight_S;
	//chart values x and y axes 
	_minY = 0;
	_maxY = 10;
	_minX_ms = 0;
	_maxX_ms = 10000;	//10 secs of data - buffersize(OGL buffer) needs to be this big!NEEDS TO MATCH WITH THE BUFFER SIZE THE USER USES AS INPUT(NUMBER OF VALUES IN BUFFER)
	//_maxX_ms = _bufferSize;

	_x_dist = 1;
	_y_dist = 1;

	//Setup OGL Chart buffer - empty 
	chartVBO.create();
	chartVBO.bind();
	f->glEnableVertexAttribArray(0);
	f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);		//3 positions for x and y and z data coordinates
	chartVBO.allocate(nullptr, _bufferSize);
	chartVBO.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	f->glDisableVertexAttribArray(0);
	chartVBO.release();

	setupAxes();
}

void OGLChart::setupAxes() {
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

	float xAxis_length = _chartWidth_S;
	float xAxis_width = 5.0;
	float yAxis_length = _chartHeight_S;
	float yAxis_width = 5.0;
	//triangle approach
	QVector<float> xAxis_verts;
	//
	//P1--------------------------------------------------- P4
	// |													| <- width
	//P2--------------------------------------------------- P3
	//						length

    float axis_pos_z = 1.0f;
	//first triang P1-P2-P4
	xAxis_verts.append(_chartPositionX_S);
	xAxis_verts.append(_chartPositionY_S);
    xAxis_verts.append(axis_pos_z);

	xAxis_verts.append(_chartPositionX_S);
	xAxis_verts.append(_chartPositionY_S + xAxis_width);
    xAxis_verts.append(axis_pos_z);

    xAxis_verts.append(_chartPositionX_S + xAxis_length);
	xAxis_verts.append(_chartPositionY_S);
    xAxis_verts.append(axis_pos_z);

	//second triang P3 - P4 - P2
	xAxis_verts.append(_chartPositionX_S + xAxis_length);
	xAxis_verts.append(_chartPositionY_S + xAxis_width);
    xAxis_verts.append(axis_pos_z);

	xAxis_verts.append(_chartPositionX_S + xAxis_length);
	xAxis_verts.append(_chartPositionY_S);
    xAxis_verts.append(axis_pos_z);

	xAxis_verts.append(_chartPositionX_S);
	xAxis_verts.append(_chartPositionY_S + xAxis_width);
    xAxis_verts.append(axis_pos_z);


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

	QVector<float> yAxis_verts;

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

	//p1..
	yAxis_verts.append(_chartPositionX_S);
	yAxis_verts.append(_chartPositionY_S);
    yAxis_verts.append(axis_pos_z);

	yAxis_verts.append(_chartPositionX_S);
	yAxis_verts.append(_chartPositionY_S + yAxis_length);
    yAxis_verts.append(axis_pos_z);

	yAxis_verts.append(_chartPositionX_S + yAxis_length);
	yAxis_verts.append(_chartPositionY_S + yAxis_width);
    yAxis_verts.append(axis_pos_z);
	//..p4
	yAxis_verts.append(_chartPositionX_S + yAxis_length);
	yAxis_verts.append(_chartPositionY_S);
    yAxis_verts.append(axis_pos_z);


	//Setup OGL Chart buffer - empty 
	_xAxisVBO.create();
	_xAxisVBO.bind();
	f->glEnableVertexAttribArray(0);
	f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);		//3 positions for x and y and z data coordinates
	_xAxisVBO.allocate(xAxis_verts.constData(), xAxis_verts.size() * sizeof(float));
	_xAxisVBO.setUsagePattern(QOpenGLBuffer::StaticDraw);
	f->glDisableVertexAttribArray(0);
	_xAxisVBO.release();

	//Setup OGL Chart buffer - empty 
	_yAxisVBO.create();
	_yAxisVBO.bind();
	f->glEnableVertexAttribArray(0);
	f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);		//3 positions for x and y and z data coordinates
	_yAxisVBO.allocate(yAxis_verts.constData(), yAxis_verts.size()* sizeof(float));
	_yAxisVBO.setUsagePattern(QOpenGLBuffer::StaticDraw);
	f->glDisableVertexAttribArray(0);
	_yAxisVBO.release();

}
void OGLChart::drawAxes(){
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

	_xAxisVBO.bind();
	f->glEnableVertexAttribArray(0);
	f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	f->glDrawArrays(GL_TRIANGLES, 0, 6);
	f->glDisableVertexAttribArray(0);
	_xAxisVBO.release();

	_yAxisVBO.bind();
	f->glEnableVertexAttribArray(0);
	f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	f->glDrawArrays(GL_QUADS, 0, 4);
	f->glDisableVertexAttribArray(0);
	_yAxisVBO.release();
}


void OGLChart::drawNewestData(){}
void OGLChart::deleteDataFromBeginning(int count) {}
void OGLChart::updateChart() {}
//void OGLChart::sendDataToOGL(QOpenGLBuffer& chartVBO)//makes the function better for reuse..(we can use at as general buffer write function)
