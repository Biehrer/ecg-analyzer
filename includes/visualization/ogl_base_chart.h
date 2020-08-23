#pragma once

// Project includes
#include "ogl_chart_geometry_c.h"
#include "chart_shapes_c.h"
#include "chart_types.h"
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
                       const QObject& parent);

        ~OGLBaseChart_C();

        // Public access functions
    public:
        //! Sets the label of the plot
        void SetLabel(const std::string& label);

        //! Sets the unique ID of the plot
        void SetID(unsigned int id);

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

        //! Sets the model view projection matrix used for text rendering (e.g. axes units)
        void SetModelViewProjection(QMatrix4x4 model_view_projection);

        const std::string& GetLabel();

        unsigned int GetID();

        // Protected helper functions
    protected:
        //! Create the text for displaying axes units
        void InitializeAxesDescription(const QVector<float>& horizontal_grid_vertices,
                                       const QVector<float>& vertical_grid_vertices,
                                       float scale, 
                                       int time_range_ms, 
                                       double max_y_val,
                                       float maj_tick_x, 
                                       float maj_tick_y);

        //! Draws the x- and y-axis inside the opengl context
        //! from which the function is called
        void DrawXYAxes(QOpenGLShaderProgram& shader, QOpenGLShaderProgram& text_shader);

        //! Draws the border bounding box of the plot area inside the opengl context
        void DrawBoundingBox(QOpenGLShaderProgram& shader);

        //! Draws the surface grid
        void DrawSurfaceGrid(QOpenGLShaderProgram& shader);

        //! Creates the vbo used to draw the bounding box of the chart
        void CreateBoundingBox();

        //! Creates a vbo used to draw the grid of the chart
        std::pair<QVector<float>, QVector<float>> CreateSurfaceGrid(int x_dist_unit, 
                                                                    float y_dist_unit,
                                                                    int time_range_ms, 
                                                                    float max_y,
                                                                    float min_y);

        //! Creates and fills vertex buffer objects used for the axes of the chart
        void SetupAxes();

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

        //! Number of vertices used to draw the vertical surface grid lines
        int _num_of_surface_grid_positions;

        //! The bounding box geometry of the chart.
        //! Stores where to place to chart inside the opengl viewport
        OGLChartGeometry_C _bounding_box;

        //! The plot area is the area in which the data series is drawn
        OGLChartGeometry_C _plot_area;

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
        const QObject& _parent_widget;

        // the label (name) of the plot (avR, I, II,...)
        std::string _label = "";

        // plot id
        unsigned int _id;

};

