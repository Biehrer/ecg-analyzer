#pragma once

// Project includes
#include "ogl_chart_geometry_c.h"
#include "chart_shapes_c.h"
#include "chart_types.h"
#include "ogl_text_label_c.h"

// STL includes
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <chrono>
#include <time.h>
#include <atomic>
#include <ctime>

// Qt includes
#include <QDesktopWidget>
#include <QApplication>
#include <qguiapplication.h>
#include <QScreen>

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
                       const QObject* parent = nullptr);

        // Copy initialization
        OGLBaseChart_C(const OGLBaseChart_C& other) = delete;
        
        // Copy assignment
        OGLBaseChart_C& operator=(const OGLBaseChart_C& other) = delete;

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

        //! Set the color of the fiducial markers
        void SetFiducialMarkColor(const QVector3D& color);

        //! Sets the model view projection matrix used for text rendering (e.g. axes units)
        void SetModelViewProjection(const QMatrix4x4& model_view_projection);

        float GetScreenCoordsFromChartXValue(float y_value_unit);
        float GetScreenCoordsFromChartYValue(double x_value_ms);

        virtual float GetMaxValueYAxes();
        virtual float GetMinValueYAxes();

        virtual void SetMaxValueYAxes(float max_y_val);
        virtual void SetMinValueYAxes(float min_y_val);

        virtual double GetTimerangeMs();
        virtual void SetTimerangeMs(double timerange_ms);

        virtual void SetMinorTickValueXAxes(float minor_tick_x);
        virtual void SetMajorTickValueXAxes(float major_tick_x);

        virtual float GetMajorTickValueXAxes();
        virtual float GetMinorTickValueXAxes();

        virtual void SetMinorTickValueYAxes(float minor_tick_y);
        virtual void SetMajorTickValueYAxes(float major_tick_y);

        virtual float GetMajorTickValueYAxes();
        virtual float GetMinorTickValueYAxes();

        const OGLChartGeometry_C& GetBoundingBox();
        
        const OGLChartGeometry_C& GetPlotSurfaceArea();

        const std::string& GetLabel();

        unsigned int GetID();

        // Protected helper functions
    protected:
        //! Create the text for displaying axes units
        void InitializeAxesDescription(const QVector<float>& horizontal_grid_vertices,
                                       const QVector<float>& vertical_grid_vertices,
                                       float scale);

        //! Draws the x- and y-axis inside the opengl context
        //! from which the function is called
        void DrawTextLabels(QOpenGLShaderProgram& shader, QOpenGLShaderProgram& text_shader);

        //! Draws the border bounding box of the plot area inside the opengl context
        void DrawBoundingBox(QOpenGLShaderProgram& shader);

        //! Draws the surface grid
        void DrawSurfaceGrid(QOpenGLShaderProgram& shader);

        //! Creates the vbo used to draw the bounding box of the chart
        void CreateBoundingBox();

        //! Creates a vbo used to draw the grid of the chart
        std::pair<QVector<float>, QVector<float>> CreateSurfaceGridVertices();

        // Protected attributes
    protected:
        //! Vertex buffer object for the bounding box
        QOpenGLBuffer _bb_vbo;

        //! Vertex buffer object for the background vertical and horizontal grid lines
        QOpenGLBuffer _surface_grid_vbo;

        //! The bounding box geometry of the chart.
        //! Stores where to place to chart inside the opengl viewport
        OGLChartGeometry_C _bounding_box;

        //! The plot area is the area in which the data series is drawn
        OGLChartGeometry_C _plot_area;

        //! Model view projection transform matrix for text rendering
        QMatrix4x4 _chart_mvp;

        // Todo: Does this color stuff really belong in here? 
        // Sure it helps, but this should belong inside the appropriate clases of these objects, I guess

        //! color of the data series
        QVector3D _series_color;
        
        //! color of fiducial marks
        QVector3D _fiducial_marks_color;
        
        //! color of the bounding box
        QVector3D _bounding_box_color;
        
        //! color of the surface grid
        QVector3D _surface_grid_color;
        
        //! color of x and y axes
        QVector3D _axes_color;

        //! color of text(axes scale and unit)
        QVector3D _text_color;

        //! unit descriptions for the x and y axes
        std::vector<OGLTextBox> _plot_axes;

        //! A textbox for the label
        OGLTextBox _plot_label;

        // the label (name) of the plot (avR, I, II,...)
        std::string _label = "";

        // plot id
        unsigned int _id = 0;

        //! Number of vertices used to draw the vertical surface grid lines
        int _num_of_grid_positions = 0;

        //! Major tick value for the x-axes 
        //! In this distance, text labels with axes units are drawn on the x axis
        float _major_tick_x_axes = 0.0;

        //! Major tick value for the y-axes 
        //! In this distance, text labels with axes units are drawn on the y axis
        float _major_tick_y_axes = 0.0;

        //! Minor tick value for the y-axes
        float _minor_tick_x_axes = 100.0;

        //! Minor tick value for the y-axes 
        float _minor_tick_y_axes = 0.0;

        //! Timerange of the x axis in milliseconds 
        //! (_max_x_axis_val_ms - _min_x_axis_val_ms)
        double _time_range_ms;

        //! The maximum value of the y axis 
        float _max_y_axis_value;

        //! The minimum value of the y axis 
        float _min_y_axis_value;

        //! The parent widget with the opengl context
        const QObject* _parent_widget;
};

