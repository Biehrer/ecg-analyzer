#pragma once

// Project includes
#include "circular_buffer.h"
#include "chart_types.h"

// STL includes
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

// Qt includes
#include <qvector.h>
#include <qopenglfunctions.h>
#include <qopenglbuffer.h>
#include <qopenglextrafunctions.h>
#include <qopenglwidget.h>
#include <qopenglshaderprogram.h>

enum class DrawingStyle_TP {
    LINES,
    LINE_SERIES,
    POINT_SERIES
};

template<typename DataType_TP>
class OGLSweepChartBuffer_C {

public:
    friend class OGLChartRingBufferTest_C;

    // Constructing / Copying / Destuction
public:

    OGLSweepChartBuffer_C(int buffer_size, 
                          double time_range_ms, 
                          RingBufferOptimized_TC<ChartPoint_TP<Position3D_TC<DataType_TP>>>& input_buffer);

    // Copy constructor / assignment
    OGLSweepChartBuffer_C(const OGLSweepChartBuffer_C& other) = delete;
    OGLSweepChartBuffer_C& operator=(const OGLSweepChartBuffer_C& other) = delete;
    
    // Move constructor / assignment
    OGLSweepChartBuffer_C(OGLSweepChartBuffer_C &&) = delete;
    OGLSweepChartBuffer_C& operator=(OGLSweepChartBuffer_C &&) = delete;

    ~OGLSweepChartBuffer_C();

    // Public access functions 
public:
    //! Draws the chart inside an active OpenGL context
    void Draw();

    //! Set the timerange of the buffer
    //! Determines which values are removed inside the OnChartUpdate() method.
    //! All values, which are older than time_range_ms, will be removed from the buffer
    void SetTimeRange(double time_range_ms);

    //! Sets the Drawing behavior (glDrawElements/Arrays)
    //! currently supported: GL_LINE_STRIP and GL_POINTS and GL_LINES
    void SetPrimitiveType(DrawingStyle_TP primitive_type);

    DrawingStyle_TP GetDrawingStyle();

    //! Returns the x value last addded to the plot
    DataType_TP GetLastPlottedXValue();

    //! Returns the y value last addded to the plot
    DataType_TP GetLastPlottedYValue();

    //! Returns the nuber of 3d positions stored inside the buffer
    //! You can do calculation to get the number of stored vertices or bytes
    //! return_value * 3 = num_of_vertices_in_buffer
    //! return_value * 3 * sizeof(float) = number of bytes in buffer
    int GetNumberOfPoints();

    //! Destroy the content of the buffer 
    void Clear();

    //! Creates and allocates an empty OpenGL vertex buffer object used to store data for visualization
    void AllocateSeriesVbo();

private:
    //! Updates the chart buffer with the newest data from the input_buffer
    void OnChartUpdate();

    //! Write data to the vbo for visualization of data points
    //!
    //! \param data the data to write to the vbo
    void WriteToVbo(const QVector<DataType_TP>& data);

    //! Writes NAN-data inside the vertex buffer to remove data
    //! older than the timerange, for the viewer.
    //! This function checks if the data inside the input array exceeds the time-range and
    //! disables visualization of outdated lines (which are out of timerange) 
    //! by replacing the data with NAN values ( only inside the vertex buffer, not the input ring-buffer )
    void RemoveOutdatedDataInsideVBO();

    //! Increments the _point_count variable, which is used
    //! to tell opengl how many lines should be drawn from the vertex buffer
    void IncrementPointCount(size_t increment = 1);

    //! This function returns the index of the position of the 'timestamp' inside the input_buffer. 
    //! The function returns the next index, bigger than the timestamp value inside 'timestamp'
    //! The function uses binary search (std::upper_limit) for fast searching. 
    //!
    //! \param timestamp the timestamp to which the index is needed
    //! \param data the data array in which the timestamp is searched
    //! \returns on success, the index to the timestamp value inside the ogl chart input ring-buffer. 
    //!     If nothing was found the function returns -1.
    int FindIdxToTimestampInsideData(const Timestamp_TP & timestamp,
                                     const std::vector<ChartPoint_TP<Position3D_TC<DataType_TP>>>& data);

    // Private attributes
private:
    //! write position for the vbo
    int64_t _head_idx = 0;

    //! remove position for the vbo
    int64_t _tail_idx = 0;

    //! size of the input buffer (positions)
    int64_t _input_buffer_size;

    //! size of the vbo (bytes)
    int64_t _vbo_size;

    //! number of positions (3d points) inside the chart
    int _point_count = 0;

    //! Vertex buffer object which contains the data series added by AddData..(..)
    QOpenGLBuffer _chart_vbo;

    //! Time range used for the OGLChart_C
    double _time_range_ms = 0;

    //! Indicates if the dataseries was already wrapped one time from the right to the left screen
    bool _dataseries_wrapped_once = false;

    //! A vector filled with NAN values 
    //! -> these values are send to opengl to interrupt the line strip, 
    //! when it reached the left screen border)
    QVector<DataType_TP> _no_line_vertices;

    DataType_TP _last_plotted_y_value_S = 0;

    DataType_TP _last_plotted_x_value_S = 0;

    //! Input buffer, from which new data is read, each time when the Draw() method was called.
    RingBufferOptimized_TC<ChartPoint_TP<Position3D_TC<DataType_TP>>>& _input_buffer;

    //! Determines how vertices added to the buffer are drawn on screen (GL_LINES, GL_POINTS, GL_LINE_STRIP)
    GLenum _primitive_type = GL_LINE_STRIP;
};


template<typename DataType_TP>
OGLSweepChartBuffer_C<DataType_TP>::OGLSweepChartBuffer_C(int buffer_size,
                                                        double time_range_ms,
                                                        RingBufferOptimized_TC<ChartPoint_TP<Position3D_TC<DataType_TP>>>& input_buffer)
    :
    _vbo_size(buffer_size * 3 * sizeof(float)),
    _input_buffer(input_buffer),
    _input_buffer_size(buffer_size),
    _time_range_ms(time_range_ms),
    _no_line_vertices(buffer_size * 3),
    _chart_vbo(QOpenGLBuffer::VertexBuffer)
{
    _no_line_vertices.fill(NAN, buffer_size * 3);
}


template<typename DataType_TP>
OGLSweepChartBuffer_C<DataType_TP>::~OGLSweepChartBuffer_C() 
{
    _chart_vbo.destroy();
}

template<typename DataType_TP>
void
OGLSweepChartBuffer_C<DataType_TP>::Draw()
{
    auto* f = QOpenGLContext::currentContext()->functions();
    //Bind buffer and send data to the gpu
    _chart_vbo.bind();
    OnChartUpdate();
    //Draw inside the current context
    f->glEnableVertexAttribArray(0);
    f->glEnableVertexAttribArray(1);
    // each point (GL_POINT) consists of 3 components (x, y, z)
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    f->glDrawArrays(_primitive_type, 0, _point_count);
    f->glDisableVertexAttribArray(0);
    _chart_vbo.release();

}

template<typename DataType_TP>
inline
void 
OGLSweepChartBuffer_C<DataType_TP>::SetTimeRange(double time_range_ms)
{
    _time_range_ms = time_range_ms;
}


template<typename DataType_TP>
DataType_TP
OGLSweepChartBuffer_C<DataType_TP>::GetLastPlottedXValue()
{
    return _last_plotted_x_value_S;
}


template<typename DataType_TP>
DataType_TP
OGLSweepChartBuffer_C<DataType_TP>::GetLastPlottedYValue()
{
    return _last_plotted_y_value_S;
}

template<typename DataType_TP>
inline 
void OGLSweepChartBuffer_C<DataType_TP>::Clear()
{
     _last_plotted_y_value_S = 0;
     _last_plotted_x_value_S = 0;
     //_chart_vbo.write(0, _no_line_vertices.constData(), _input_buffer_size * 3 * sizeof(DataType_TP));
     _tail_idx = 0;
     _head_idx = 0;
     _point_count = 0;
}


template<typename DataType_TP>
void
OGLSweepChartBuffer_C<DataType_TP>::OnChartUpdate()
{
    if ( _input_buffer.IsBufferEmpty() ) {
        return;
    }

    //Get latest data from the input buffer
    auto latest_data = _input_buffer.PopLatest();
    //const auto latest_data = _input_buffer.PopLatestRef();

    //if ( !latest_data.empty() ) {
     if ( latest_data.size() > 0 ) {
        QVector<float> additional_point_vertices; 
        // TODO: Preallocate for performance - do not use append, but use an idx to iterate and insert new values
        additional_point_vertices.reserve(latest_data.size());
        //additional_point_vertices.insert()
        for ( const auto& element : latest_data ) {
            //Check if its neccessary to end the line strip,
            //due to a wrap of the series from the right to the left screen border
            // The 'break' is achieved by sending NAN values to OpenGl.
            if ( _primitive_type == GL_LINE_STRIP && 
                element._value._x < _last_plotted_x_value_S )
            {
                additional_point_vertices.append(NAN);
                additional_point_vertices.append(NAN);
                additional_point_vertices.append(NAN);
            }
            // DEBUG(element);
            // force construction inplace via std::move (because QVector is missing emplace_back() -> but has a move override since Qt5.)
            //additional_point_vertices.append(std::move(element._value._x));
            //additional_point_vertices.append(std::move(element._value._y));
            //additional_point_vertices.append(std::move(element._value._z));
            additional_point_vertices.append(element._value._x);
            additional_point_vertices.append(element._value._y);
            additional_point_vertices.append(element._value._z);
            _last_plotted_x_value_S = element._value._x;
        }
        // Does not work if the items weree moved, because latest_data is empty
        _last_plotted_y_value_S =  (latest_data.end() - 1)->_value._y;

        WriteToVbo(additional_point_vertices);

        RemoveOutdatedDataInsideVBO();
     }
}

template<typename DataType_TP>
int
OGLSweepChartBuffer_C<DataType_TP>::GetNumberOfPoints() {
    return _point_count;
}

template<typename DataType_TP>
void
OGLSweepChartBuffer_C<DataType_TP>::SetPrimitiveType(DrawingStyle_TP primitive_type)
{
    switch ( primitive_type ) {

    case DrawingStyle_TP::LINES:
        _primitive_type = GL_LINES; 
        break;

    case DrawingStyle_TP::LINE_SERIES:
        _primitive_type = GL_LINE_STRIP;
        break;

    case DrawingStyle_TP::POINT_SERIES:
        _primitive_type = GL_POINTS;
        break;
    }
}

template<typename DataType_TP>
inline
DrawingStyle_TP OGLSweepChartBuffer_C<DataType_TP>::GetDrawingStyle()
{
    DrawingStyle_TP drawing_style;

    switch ( _primitive_type ) {

    case GL_LINES:
        drawing_style = DrawingStyle_TP::LINES;
        break;

    case GL_LINE_STRIP:
        drawing_style = DrawingStyle_TP::LINE_SERIES;
        break;

    case GL_POINTS:
        drawing_style = DrawingStyle_TP::POINT_SERIES;
        break;
    }

    return drawing_style;
}


template<typename DataType_TP>
void
OGLSweepChartBuffer_C<DataType_TP>::AllocateSeriesVbo()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    // create empty chart buffer
    _chart_vbo.create();
    _chart_vbo.bind();
    f->glEnableVertexAttribArray(0);
    // position coordinates: 3 -> (x, y, z)
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    _chart_vbo.allocate(nullptr, _vbo_size);
    _chart_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    f->glDisableVertexAttribArray(0);
    _chart_vbo.release();

}

// Write test for this function => Make the Test class a friend, so it can call this function directly!
template<typename DataType_TP>
inline
int
OGLSweepChartBuffer_C<DataType_TP>::FindIdxToTimestampInsideData(const Timestamp_TP& timestamp,
    const std::vector<ChartPoint_TP<Position3D_TC<DataType_TP>>>& data)
{
    
    // We can not scan the whole buffer, because binary search requires that the data is sorted.
    // therefore, in the worst case scenario, two binary searches are necessary, to get the index of the timestamp
    auto latest_raw_data_begin = data.begin() + _tail_idx;
    auto it_interesting_data_range_until_end
        = std::lower_bound(latest_raw_data_begin, data.end(), timestamp, CmpTimestamps); // Makes no sense, because data until .end() is not sorted, from latest_raw_data_begin
    //TODO: (Because it was wrong all the time, but the problem did not show itself until 
    // I choose a buffer which was way too big for the timerange -> I just got luck the last times, because the buffer was approximately the right size)
    //
    // check not until end, but check from latest timerange it
    //= std::lower_bound(latest_raw_data_begin, data.begin() + _head_idx, timestamp, CmpTimestamps);
    // Some fixed value(100) -> makes problem when the buffer is smaller than this value because we access invalid memory locations
    //= std::lower_bound(latest_raw_data_begin, latest_raw_data_begin+ 100 , timestamp, CmpTimestamps);

    if ( it_interesting_data_range_until_end != data.end() ) {
        std::size_t index = std::distance(data.begin(), it_interesting_data_range_until_end);
        return index;
    }

    int current_idx = _head_idx / 3 / sizeof(float); // /6 with GL_LINE?

    auto current_data_ptr = data.begin() + current_idx +1;// +1 ? -> because .end() does also point to one position after the end
    auto it_interesting_data_range_until_current =
        std::lower_bound(data.begin(), current_data_ptr, timestamp, CmpTimestamps);

    if ( it_interesting_data_range_until_current != current_data_ptr  ) {
        std::size_t index = std::distance(data.begin(), it_interesting_data_range_until_current);
        return index;
    }else {
        return -1;
        std::cout << "could not find idx to timestamp. return -1 " << std::endl;
    }

    // Possibilitys: 
    // (Values added -> ++_tail)
    // (Values poped -> ++_head)

    //- begin() until _head => these are old values which are not inside the timerange of the buffer -> But they should be sorted
    //- begin() until _tail => Should be sorted(NOT ITS NOT) -> I Have to check timeranges from the beginning if noting succeeds
    //- _head until _tail   => this is the latest timerange-> makes sense! -> is sorted

    //- begin() until end() => this is not sorted and makes no sense
    //- _head until end()   => not sorted
    // _tail until end()    => not sorted

    //// Not good, because binary search does only work when everything is sorted. this is not the case with the ring buffer (only two parts are sorted)
    //auto complete_range =
    //    std::lower_bound(data.begin(), data.end(), timestamp, CmpTimestamps);

    //if ( complete_range != data.end() ) {
    //    std::size_t index = std::distance(data.begin(), complete_range);
    //    return index;
    //}
    //else {
    //    return -1;
    //}
}

template<typename DataType_TP>
inline
void
OGLSweepChartBuffer_C<DataType_TP>::WriteToVbo(const QVector<DataType_TP>& data)
{
    int number_of_new_data_bytes = static_cast<int>(data.size()) * static_cast<int>(sizeof(float));

    if ( _head_idx + number_of_new_data_bytes <= _vbo_size ) {
        //The data can completely fit into the vbo 
        _chart_vbo.write(static_cast<int>(_head_idx), data.constData(), number_of_new_data_bytes);
        // increment write offset in bytes
        _head_idx += number_of_new_data_bytes;
        // TODO:  When we draw GL_LINES, we need to increment the pointcount different? data.size() / 6, because each line is made of 6 vertices?
        // =>Nope, that is wrong, because when we render GL_LINES, we already pushed twice as much data inside data, as when we render GL_LINE_STRIP
        //if ( _primitive_type == GL_LINES ) {
        //    IncrementPointCount(data.size() / 6);
        //}else{
        //    IncrementPointCount(data.size() / 3); 
        //}
        IncrementPointCount(data.size() / 3);
    }
    else {
        // currently the buffer is full or not all the new data can fit into it; 
        // reset buffer index and start overwriting data at the beginning.
        // Calculate how much bytes can fit into the buffer until the end is reached:
        int number_of_free_bytes_until_end = _vbo_size - _head_idx;
        int bytes_to_write_at_beginning = number_of_new_data_bytes - number_of_free_bytes_until_end;
        int bytes_to_write_until_end = number_of_new_data_bytes - bytes_to_write_at_beginning;

        if ( number_of_free_bytes_until_end > 0 ) {
            //Write data until the end of the buffer is reached
            _chart_vbo.write(static_cast<int>(_head_idx),
                data.data(),
                bytes_to_write_until_end);

            //if ( _primitive_type == GL_LINES ) {
            //    IncrementPointCount(number_of_free_bytes_until_end / sizeof(float) / 6);
            //} else {
            //    IncrementPointCount(number_of_free_bytes_until_end / sizeof(float) / 3);
            //}
            IncrementPointCount(number_of_free_bytes_until_end / sizeof(float) / 3);
        }

        _dataseries_wrapped_once = true;
        //Reset the index to continue writing the rest of the data at the beginning
        _head_idx = 0;
        if ( bytes_to_write_at_beginning > 0 ) {

            int data_memory_offset = bytes_to_write_until_end / sizeof(float);
            _chart_vbo.write(static_cast<int>(_head_idx),
                (data.constData() + data_memory_offset),
                bytes_to_write_at_beginning);

            _head_idx += bytes_to_write_at_beginning;
        }
    }
}


template<typename DataType_TP>
inline
void
OGLSweepChartBuffer_C<DataType_TP>::IncrementPointCount(size_t increment)
{
    // Count points; stop counting points after one wrap
    if ( !_dataseries_wrapped_once ) {
        _point_count += increment;
    }
}


template<typename DataType_TP>
inline
void
OGLSweepChartBuffer_C<DataType_TP>::RemoveOutdatedDataInsideVBO()
{
    size_t last_added_tstamp_ms = _input_buffer.GetLatestItem()._timestamp.GetSeconds();

    double start_time_ms = static_cast<double>(last_added_tstamp_ms) - _time_range_ms;
    int start_time_idx = FindIdxToTimestampInsideData(Timestamp_TP(start_time_ms), _input_buffer.constData()) - 1;

    if ( start_time_idx > -1 ) {
        uint32_t bytes_to_remove = (start_time_idx + 1 - _tail_idx) * 3 * sizeof(float);

        if ( bytes_to_remove > 0 ) {
            _chart_vbo.write(_tail_idx * 3 * sizeof(float), _no_line_vertices.constData(), bytes_to_remove);
            // _line_engine.RemoveUntil(_tail_idx * 3 * sizeof(float), bytes_to_remove);
        } else {
            // recalculate the index when the current write_index wrapped (this means its near zero of the time axis) and 
           // the remove-index is still removing data at the end of the buffer(this means its at the end of the time axis):
           // ==> in this else-case, the remove index is bigger than the write index,
           // and the calcuation of how much bytes should be removed, like its done above, will return a negative value.
           // Of course we can't write negative amounts of bytes. 
            // This problem is solved by writing two times to the buffer (NAN-values):
           // 1. time: data is removed until the end of the buffer is reached (write_end_index (=vbo_buffersize) )
           // 2. time: data is removed from the beginning up to the first timestamp of the latest timerange
            int remove_series_idx_byte = _tail_idx * 3 * sizeof(float);
            int number_of_free_bytes_until_end = _vbo_size - remove_series_idx_byte;
            _chart_vbo.write(_tail_idx * 3 * sizeof(float), _no_line_vertices.constData(), number_of_free_bytes_until_end);
            int bytes_to_remove_at_beginning = (start_time_idx + 1) * 3 * sizeof(float);
            _chart_vbo.write(0, _no_line_vertices.constData(), bytes_to_remove_at_beginning);
        }

        _tail_idx = start_time_idx;
    }
}

