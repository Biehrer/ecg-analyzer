#pragma once

// Project includes
#include "circular_buffer.h"
#include "ogl_chart_geometry_c.h"
#include "chart_shapes_c.h"
#include "chart_types.h"
#include "ogl_sweep_chart_buffer.h"
#include "text_renderer_2d.h"

// STL includes
#include <iostream>
#include <string>
#include <mutex>
#include <algorithm>
#include <vector>
#include <chrono>
#include <time.h>
#include <ctime>

// Qt includes
#include <qlist.h>
#include <qvector.h>
#include "qbuffer.h"
#include <qtimer.h>
#include <qopenglfunctions.h>
#include <qopenglbuffer.h>
#include <qopenglwidget.h>
#include <qopenglextrafunctions.h>
#include <qopenglshaderprogram.h>
#include <qpainter.h>
#include <qopenglwidget.h>
#include <qdatetime.h>
#include <qvector>

class OGLBaseChart_C {

        // Constructor / Destructor / Copying..
    public:
        OGLBaseChart_C(const OGLChartGeometry_C& geometry,
                       const QOpenGLWidget& parent);

        ~OGLBaseChart_C();

        // Public access functions
    public:
        //! Sets the label of the plot
        void SetLabel(const std::string& label);

        //! Sets the unique ID of the plot
        void SetID(unsigned int id);

        //! Draws the chart inside the opengl context from which this function is called
        //template<DrawingStyle_TP type = DrawingStyle_TP::LINE_SERIES >
        ///*virtual*/ void Draw(QOpenGLShaderProgram& shader, QOpenGLShaderProgram& text_shader);

        //! Returns the y-screen coordinates of a given plot y-value
        //virtual float GetScreenCoordsFromYChartValue(float y_value);

        //! Returns the y-screen coordinates of a given plot x-value
        //float GetScreenCoordsFromXChartValue(float x_value);
      
        //! Set the major tick value for the x-axes.
        //void SetMajorTickValueXAxes(float tick_value_ms);

        //! Set the major tick value for the y axis
        //! The major tick value is used to draw the horizontal grid lines
        //void SetMajorTickValueYAxes(float tick_value_unit);

        //! Set the color of the x- and y-axes
        void SetAxesColor(const QVector3D& color);

        //! Set color for text rendering (e.g axes units)
        void SetTextColor(const QVector3D & color);

        //! Set the data series color
        void SetSeriesColor(const QVector3D& color);

        //! Set the bounding box color
        void SetBoundingBoxColor(const QVector3D& color);

        //! Set the color of the surface grid
        void SetSurfaceGridColor(const QVector3D& color);

        //! Set the color of the lead line
        //void SetLeadLineColor(const QVector3D& color);

        //! Sets the model view projection matrix used for text rendering (e.g. axes units)
        void SetModelViewProjection(QMatrix4x4 model_view_projection);

        const std::string& GetLabel();

        // Protected helper functions
    protected:
        //! Create the text for displaying axes units
        void InitializeAxesDescription(const QVector<float>& horizontal_grid_vertices,
                                       const QVector<float>& vertical_grid_vertices,
                                       float scale, int time_range_ms, float max_y_val,
                                       float maj_tick_x, float maj_tick_y);

        //! Draws the x- and y-axis inside the opengl context
        //! from which the function is called
        void DrawXYAxes(QOpenGLShaderProgram& shader, QOpenGLShaderProgram& text_shader);

        //! Draws the border bounding box of the plot area inside the opengl context
        void DrawBoundingBox(QOpenGLShaderProgram& shader);

        //! Draws the data series to the opengl context inside the plot-area
        //void DrawSeries(QOpenGLShaderProgram& shader);

        //! Draws the surface grid
        void DrawSurfaceGrid(QOpenGLShaderProgram& shader);

        //! Draws the lead line
        //void DrawLeadLine(QOpenGLShaderProgram& shader);

        //! Creates the vbo used to draw the bounding box of the chart
        void CreateBoundingBox();

        //! Creates a vbo used to draw the grid of the chart
        std::pair<QVector<float>, QVector<float>> CreateSurfaceGrid(int x_dist_unit, int y_dist_unit,
            int time_range_ms, float max_y, float min_y);

        //! Creates the vbo used to draw the lead line indicating the most current datapoint
        //void CreateLeadLineVbo();

        //! Creates and fills vertex buffer objects used for the axes of the chart
        void SetupAxes();

        //! Update the current position of the lead line. 
        //! Used to assign the last visualized point as lead-line position
        //void UpdateLeadLinePosition(float x_value_new);

        // Protected attributes
    protected:
        //! Vertex buffer object for the x axis vertices
        QOpenGLBuffer _x_axis_vbo;

        //! Vertex buffer object for the y axis vertices
        QOpenGLBuffer _y_axis_vbo;

        //! Vertex buffer object for the bounding box
        QOpenGLBuffer _bb_vbo;

        //! Vertex buffer object for the background vertical and horizontal grid lines
        QOpenGLBuffer _surface_grid_vbo;

        //! Vertex buffer object for the lead line 
        //QOpenGLBuffer _lead_line_vbo;

        //! Vertices for the lead line
        //QVector<float> _lead_line_vertices;

        //! Number of bytes for the lead line used by the vbo. 
        //! This should be equal to six, when the lead line is a line and no point or other shape.
        //int _number_of_bytes_lead_line;

        //! The maximum value of the y axis 
        //int _max_y_axis_value;

        //! The minimum value of the y axis 
        //int _min_y_axis_value;

        //! Timerange of the x axis in milliseconds 
        //! (_max_x_axis_val_ms - _min_x_axis_val_ms)
        //double _time_range_ms;

        //! Number of vertices used to draw the vertical surface grid lines
        int _num_of_surface_grid_positions;

        //! The bounding box geometry of the chart.
        //! Stores where to place to chart inside the opengl viewport
        OGLChartGeometry_C _bounding_box;

        //! The plot area is the area in which the data series is drawn
        OGLChartGeometry_C _plot_area;

        ////! Input buffer used to store user data
        //RingBuffer_TC<ChartPoint_TP<Position3D_TC<float>>> _input_buffer;

        ////! Buffer for visualization - the user does not know this one
        //OGLSweepChartBuffer_C _ogl_data_series;

        ////! The y component of the last value plotted
        //float _last_plotted_y_value_S = 0;

        ////! The x component of the last value plotted
        //float _last_plotted_x_value_S = 0;

        // Colors for the shader
        QVector3D _lead_line_color;

        QVector3D _series_color;

        QVector3D _bounding_box_color;

        QVector3D _surface_grid_color;

        QVector3D _axes_color;

        QVector3D _text_color;

        //! unit descriptions for the x and y axes
        std::vector<OGLTextBox> _plot_axes;

        //! Model view projection transform matrix for text rendering
        QMatrix4x4 _chart_mvp;

        //! The parent widget with the opengl context
        const QOpenGLWidget& _parent_widget;

        // the label (name) of the plot (avR, I, II,...)
        std::string _label = "";

        // plot id
        unsigned int _id;

};

// intended use ( safes space and improves readability ) :
//class OGLSweepChart_C : public OGLBaseChart_C{};