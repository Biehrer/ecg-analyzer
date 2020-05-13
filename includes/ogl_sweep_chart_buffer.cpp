////#include <includes/ogl_sweep_chart_buffer.h>
//
//template<DrawingStyle_TP DrawingStyle>
//OGLSweepChartBuffer_C<DrawingStyle>::OGLSweepChartBuffer_C(int buffer_size,
//                                             double time_range_ms,
//                                             RingBuffer_TC<ChartPoint_TP<Position3D_TC<float>>>& input_buffer)
//    :
//    _vbo_buffer_size(buffer_size * 3 * sizeof(float)),
//    _input_buffer(input_buffer),
//    _buffer_size(buffer_size),Inc
//    _time_range_ms(time_range_ms), 
//    _no_line_vertices(buffer_size),
//    _chart_vbo(QOpenGLBuffer::VertexBuffer)
//{
//    _no_line_vertices.fill(NAN, buffer_size);
//}
//
//template<DrawingStyle_TP DrawingStyle>
//OGLSweepChartBuffer_C<DrawingStyle>::~OGLSweepChartBuffer_C(){
//    _chart_vbo.destroy();
//}
//
////void OGLSweepChartBuffer_C::Bind() {
////    _chart_vbo.bind();
////}
////
////void OGLSweepChartBuffer_C::Release() {
////    _chart_vbo.release();
////}
//
//template<>
//void OGLSweepChartBuffer_C<DrawingStyle_TP::LINE_STRIP>::Draw()
//{
//    auto* f = QOpenGLContext::currentContext()->functions();
//    // Bind buffer and send data to the gpu
//    _chart_vbo.bind();
//    OnChartUpdate();
//    // Draw inside the current context
//    f->glEnableVertexAttribArray(0);
//    //f->glEnableVertexAttribArray(1);
//    // each point (GL_POINT) consists of 3 components (x, y, z)
//    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
//    // to get the abs number of points-> divide through count of each Point
//    f->glDrawArrays(GL_LINE_STRIP, 0, _point_count);
//    f->glDisableVertexAttribArray(0);
//    _chart_vbo.release();
//}
//
//template<>
//void OGLSweepChartBuffer_C<DrawingStyle_TP::POINT_STRIP>::Draw()
//{
//    auto* f = QOpenGLContext::currentContext()->functions();
//    // Bind buffer and send data to the gpu
//    _chart_vbo.bind();
//    OnChartUpdate();
//    // Draw inside the current context
//    f->glEnableVertexAttribArray(0);
//    //f->glEnableVertexAttribArray(1);
//    // each point (GL_POINT) consists of 3 components (x, y, z)
//    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
//    // to get the abs number of points-> divide through count of each Point
//    f->glDrawArrays(GL_POINTS, 0, _point_count);
//    f->glDisableVertexAttribArray(0);
//    _chart_vbo.release();
//}
//
//
//template<DrawingStyle_TP DrawingStyle>
//float 
//OGLSweepChartBuffer_C<DrawingStyle>::GetLastPlottedXValue()
//{
//    return _last_plotted_x_value_S;
//}
//
//template<DrawingStyle_TP DrawingStyle>
//float 
//OGLSweepChartBuffer_C<DrawingStyle>::GetLastPlottedYValue()
//{
//    return _last_plotted_y_value_S;
//}
//
//template<DrawingStyle_TP DrawingStyle>
//void
//OGLSweepChartBuffer_C<DrawingStyle>::OnChartUpdate()
//{
//    if ( _input_buffer.IsBufferEmpty() ) {
//        return;
//    }
//
//    // Get latest data from the input buffer
//    auto latest_data = _input_buffer.PopLatest();
//
//    if ( !latest_data.empty() ) {
//        QVector<float> additional_point_vertices;
//        for ( const auto& element : latest_data ) {
//            // Check if its neccessary to end the line strip,
//            // due to a wrap of the series from the right to the left screen border
//            if ( element._value._x < _last_plotted_x_value_S ) {
//                additional_point_vertices.append(NAN);
//                additional_point_vertices.append(NAN);
//                additional_point_vertices.append(NAN);
//            }
//            //DEBUG(element);
//            additional_point_vertices.append(element._value._x);
//            additional_point_vertices.append(element._value._y);
//            additional_point_vertices.append(element._value._z);
//            _last_plotted_x_value_S = element._value._x;
//        }
//
//        _last_plotted_y_value_S = (latest_data.end() - 1)->_value._y;
//
//        WriteToVbo(additional_point_vertices);
//
//        RemoveOutdatedDataInsideVBO();
//    }
//}
//
//template<DrawingStyle_TP DrawingStyle>
//int OGLSweepChartBuffer_C<DrawingStyle>::GetNumberOfPoints() {
//    return _point_count;
//}
//
//template<DrawingStyle_TP DrawingStyle>
//void OGLSweepChartBuffer_C<DrawingStyle>::AllocateSeriesVbo()
//{
//    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
//    // Setup OGL-Chart buffer - empty
//    _chart_vbo.create();
//    _chart_vbo.bind();
//    f->glEnableVertexAttribArray(0);
//    // 3 coordinates make one point  (x, y, z)
//    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
//    _chart_vbo.allocate(nullptr, _vbo_buffer_size);
//    _chart_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
//    f->glDisableVertexAttribArray(0);
//    _chart_vbo.release();
//}
//
//// Todo : Refactor this function in the RingBuffer itself ! Then we can call this function on the ringbuffer -> should work!
//template<DrawingStyle_TP DrawingStyle>
//inline
//int
//OGLSweepChartBuffer_C<DrawingStyle>::FindIdxToTimestampInsideData(const Timestamp_TP& timestamp,
//    const std::vector<ChartPoint_TP<Position3D_TC<float>>>& data)
//{
//    int current_idx = _vbo_current_series_idx / 3 / sizeof(float);
//    //if( current_idx > _remove_series_idx ){
//        // case 1
//    auto it = std::lower_bound(data.begin(), data.end(), timestamp, CmpTimestamps);
//    if ( it != data.end() ) {
//        std::size_t index = std::distance(data.begin(), it);
//        return index;
//    }
//    else {
//        //return -1;
//    }
//    //} else {
//        // if _remove_idx > head_idx the data to remove is most probably at the end of the buffer => check first the end
//    auto latest_raw_data_begin = data.begin() + _remove_series_idx;
//    auto it_interesting_data_range_until_end = std::lower_bound(latest_raw_data_begin, data.end(), timestamp, CmpTimestamps); // search inside the old data
//    if ( it_interesting_data_range_until_end != data.end() ) {
//        std::size_t index = std::distance(data.begin(), it_interesting_data_range_until_end);
//        return index;
//    }
//
//    auto current_data_ptr = data.begin() + current_idx;
//    auto it_interesting_data_range_until_current = std::lower_bound(data.begin(), current_data_ptr, timestamp, CmpTimestamps); // search inside the old data
//    if ( it_interesting_data_range_until_current != current_data_ptr ) {
//        std::size_t index = std::distance(data.begin(), it_interesting_data_range_until_current);
//        return index;
//    }
//    else {
//        return -1;
//    }
//    //}
//}
//
//
//// Make sure the buffer is bound to the current context before calling this function
//template<DrawingStyle_TP DrawingStyle>
//inline
//void 
//OGLSweepChartBuffer_C<DrawingStyle>::WriteToVbo(const QVector<float>& data)
//{
//    int number_of_new_data_bytes = static_cast<int>(data.size()) * static_cast<int>(sizeof(float));
//
//    if ( _vbo_current_series_idx + number_of_new_data_bytes <= _vbo_buffer_size ) {
//        // The data can completely fit into the vbo 
//        _chart_vbo.write(static_cast<int>(_vbo_current_series_idx), data.constData(), number_of_new_data_bytes);
//        // new write offset in bytes
//        _vbo_current_series_idx += number_of_new_data_bytes;
//        IncrementPointCount(data.size() / 3);
//    }
//    else {
//        // buffer is full or not all new data can fit into it; 
//        // reset buffer index and start overwriting data at the beginning
//        // Calculate how much bytes can fit into the buffer until the end is reached
//        int number_of_free_bytes_until_end = _vbo_buffer_size - _vbo_current_series_idx;
//        int bytes_to_write_at_beginning = number_of_new_data_bytes - number_of_free_bytes_until_end;
//        int bytes_to_write_until_end = number_of_new_data_bytes - bytes_to_write_at_beginning;
//
//        if ( number_of_free_bytes_until_end > 0 ) {
//            // Write data until the end of the buffer is reached
//            _chart_vbo.write(static_cast<int>(_vbo_current_series_idx), data.data(), bytes_to_write_until_end);
//            IncrementPointCount(number_of_free_bytes_until_end / sizeof(float) / 3);
//        }
//
//        _dataseries_wrapped_once = true;
//        // Reset the index to continue writing the rest of the data at the beginning
//        _vbo_current_series_idx = 0;
//        if ( bytes_to_write_at_beginning > 0 ) {
//            int data_memory_offset = bytes_to_write_until_end / sizeof(float);
//            _chart_vbo.write(static_cast<int>(_vbo_current_series_idx), (data.constData() + data_memory_offset), bytes_to_write_at_beginning);
//            _vbo_current_series_idx += bytes_to_write_at_beginning;
//        }
//    }
//}
//
//// inline 
//template<DrawingStyle_TP DrawingStyle>
//void 
//OGLSweepChartBuffer_C<DrawingStyle>::IncrementPointCount(size_t increment)
//{
//    // Count points; stop counting points after one wrap
//    // (because after a wrap the point count stays the same-> NOT ANYMORE)
//    if ( !_dataseries_wrapped_once ) {
//        _point_count += increment;
//    }
//}
//
//template<DrawingStyle_TP DrawingStyle>
//inline
//void
//OGLSweepChartBuffer_C<DrawingStyle>::RemoveOutdatedDataInsideVBO()
//{
//    size_t last_added_tstamp_ms = _input_buffer.GetLatestItem()._timestamp.GetSeconds();
//
//    double start_time_ms = static_cast<double>(last_added_tstamp_ms) - _time_range_ms;
//    int start_time_idx = FindIdxToTimestampInsideData(Timestamp_TP(start_time_ms), _input_buffer.constData()) - 1;
//
//    if ( start_time_idx > -1 ) {
//        int bytes_to_remove = (start_time_idx + 1 - _remove_series_idx) * 3 * sizeof(float);
//
//        if ( bytes_to_remove > 0 ) {
//            _chart_vbo.write(_remove_series_idx * 3 * sizeof(float), _no_line_vertices.constData(), bytes_to_remove);
//            _remove_series_idx = start_time_idx;
//
//        } else {
//            // recalculate
//            int remove_series_idx_byte = _remove_series_idx * 3 * sizeof(float);
//            int number_of_free_bytes_until_end = _vbo_buffer_size - remove_series_idx_byte;
//            _chart_vbo.write(_remove_series_idx * 3 * sizeof(float), _no_line_vertices.constData(), number_of_free_bytes_until_end);
//            int bytes_to_remove_at_beginning = (start_time_idx + 1) * 3 * sizeof(float);
//            _chart_vbo.write(0, _no_line_vertices.constData(), bytes_to_remove_at_beginning);
//            _remove_series_idx = start_time_idx;
//        }
//    }
//}
//
////template class OGLSweepChartBuffer_C<DrawingStyle_TP>;
