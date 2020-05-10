#pragma once

// Project includes
#include <includes/chart_types.h>
#include <includes/CircularBuffer.h>

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

class OGLSweepChartBuffer_C {

    // Constructing / Copying / Destuction
public:
    OGLSweepChartBuffer_C(int buffer_size, 
                         double time_range_ms, 
                         RingBuffer_TC<ChartPoint_TP<Position3D_TC<float>>>& input_buffer);

    ~OGLSweepChartBuffer_C();

    // Public access functions 
public:
    //! Bind the chart buffer
    void Bind();

    //! Releases the chart buffer
    void Release();

    //! Draws the chart inside an active OpenGL context
    void Draw();

    //! Returns the x value last addded to the plot
    float GetLastPlottedXValue();

    //! Returns the y value last addded to the plot
    float GetLastPlottedYValue();

    //! Creates and allocates an empty OpenGL vertex buffer object used to store data for visualization
    void AllocateSeriesVbo();

    //! Returns the nuber of 3d positions stored inside the buffer
    //! You can do calculation to get the number of stored vertices or bytes
    //! return_value * 3 = num_of_vertices_in_buffer
    //! return_value * 3 * sizeof(float) = number of bytes in buffer
    int GetNumberOfPoints();

private:
    //! Updates the chart buffer with the newest data from the input_buffer
    void OnChartUpdate();

    //! Write data to the vbo for visualization of data points
    //!
    //! \param data the data to write to the vbo
    void WriteToVbo(const QVector<float>& data);

    //! Replaces data out of timerange inside the vertex buffer object with NAN-values to not visualize them.
    //! Writes NAN-data inside the vertex buffer to remove data
    //! older than the timerange, for the viewer.
    //! This function checks if the data inside the input array exceeds the time-range and
    //! disables visualization of outdated lines (which are out of timerange) via opengl 
    //! by replacing the data with NAN values ( only inside the vertex buffer, not the input ring-buffer )
    //! This function uses binary search ( std::upper_limit ) for fast searching. 
    //! Its possible to compare Timestamp_TP objects with ChartPoint_TP objects 
    //! because of an compare-function which compares the Timestamp_TP object with the Timestamp_TP object of the ChartPoint_TP
    void RemoveOutdatedDataInsideVBO();

    //! Increments the _point_count variable, which is used
    //! to tell opengl how many lines should be drawn from the vertex buffer
    void IncrementPointCount(size_t increment = 1);
 
    //!
    //! \param timestamp the timestamp to search
    //! \param data the data array of ChartPoint_TP to look for.
    //! \returns on success, the offset to the timestamp value inside the ogl chart input ring-buffer. 
    //!     If nothing was found the function returns -1.
    int FindIdxToTimestampInsideData(const Timestamp_TP & timestamp, const std::vector<ChartPoint_TP<Position3D_TC<float>>>& data);

    // Private attributes
private:
    //! write position for the vbo
    int64_t _vbo_current_series_idx = 0;

    //! remove position for the vbo
    int64_t _remove_series_idx = 0;

    //! size of the input buffer (positions)
    int64_t _buffer_size;

    //! size of the vbo (bytes)
    int64_t _vbo_buffer_size;

    //! number of positions (3d points) inside the chart
    int _point_count = 0;

    //! Vertex buffer object which contains the data series added by AddData..(..)
    QOpenGLBuffer _chart_vbo;

    //! Time range used for the OGLChart_C
    double _time_range_ms = 0;

    //! Indicates if the dataseries was already wrapped one time from the right to the left screen
    bool _dataseries_wrapped_once = false;

    QVector<float> _no_line_vertices;

    float _last_plotted_y_value_S = 0;

    float _last_plotted_x_value_S = 0;

    /*const*/ RingBuffer_TC<ChartPoint_TP<Position3D_TC<float>>>& _input_buffer;
};