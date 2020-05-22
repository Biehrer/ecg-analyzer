#pragma once

// Project includes
#include "ogl_sweep_chart.h"
#include "ShapeData.h"
#include "text_renderer_2d.h"
#include "ShapeGenerator.h"

// STL includes
#include <iostream>
#include <string>
#include <cstdlib>
#include <math.h>
#include <time.h>
#include <thread>
#include <mutex>
#include <chrono>

// Qt includes
#include <qmessagebox.h>
#include "qmouseeventtransition.h"
#include <QMouseEvent>
#include <qtimer.h>
#include <qelapsedtimer.h>
#include "qbuffer.h"
#include <qdir.h>
#include <qpainter.h>
#include <qopenglwidget.h>
#include <qopenglfunctions.h>
#include <qopenglextrafunctions.h>
#include <qopenglbuffer.h>
#include <qopenglshaderprogram.h>
#include <qopengl.h>
#include <qopengltexture.h>
#include <qopenglvertexarrayobject.h>
#include <qdatetime.h>
#include <qglobal.h>
#include <qglobalstatic.h>

// Project defines
#define SREENWIDTH 1440
#define SCREENHEIGHT 800

using ChartDataType_TP = float;

//! Definition of plot colors
struct PlotColors_TP {
    QVector3D _series = QVector3D(0.0f, 1.0f, 0.0f);
    QVector3D _axes = QVector3D(1.0f, 1.0f, 1.0f);
    QVector3D _lead_line = QVector3D(1.0f, 0.01f, 0.0f);
    QVector3D _surface_grid = QVector3D(static_cast<float>(235.0f / 255.0f),
                                        static_cast<float>(225.0f / 255.0f),
                                         static_cast<float>(27.0f / 255.0f) );
    QVector3D _text = QVector3D(1.0f, 1.0f, 1.0f);
    QVector3D _bounding_box = QVector3D(1.0f, 1.0f, 1.0f);
};

//! All the info required to initialize a plot
struct PlotDescription_TP
{
    std::string _label = "unnamed";
    OGLChartGeometry_C _geometry;
    PlotColors_TP _colors;
    DrawingStyle_TP _chart_type = DrawingStyle_TP::LINE_SERIES;
    RingBufferSize_TP _buffer_size = RingBufferSize_TP::Size32768;

    unsigned int _id = 0;
    int _time_range_ms = 1000.0;
    ChartDataType_TP _min_y = -5;
    ChartDataType_TP _max_y = 5;
    ChartDataType_TP _maj_tick_x = _time_range_ms / 4.0;
    ChartDataType_TP _maj_tick_y = (_max_y - _min_y) / 4.0;
};


class QOpenGLPlotRendererWidget : public QOpenGLWidget
{
	Q_OBJECT
        // Construction / Destruction / Copying
public:
    QOpenGLPlotRendererWidget(/*unsigned int number_of_plots,*/ 
                              QWidget* parent = 0);
    
    ~QOpenGLPlotRendererWidget();

    // Public access functions
public:
    //! Fast initialization of plots in a horizontal layout (shared timerange and max/min y values)
    //! Creates OGLSweepCharts
    bool FastInitializePlots(int number_of_plots, 
                            int time_range_ms,
                            ChartDataType_TP max_y, 
                            ChartDataType_TP min_y);

    //! Adds one plot
    void AddPlot(const PlotDescription_TP& plot_info);

    //! Removes one plot
    bool RemovePlot(const std::string& label);
    
    // Unused function
    void OnDataUpdateThreadFunction();

    //! Adds a datavalue to all active plots
    void AddDataToAllPlots(ChartDataType_TP value_x, ChartDataType_TP value_y);

    //! Returns the model view projection transform matrix
    const QMatrix4x4 GetModelViewProjection() const;

    //! Returns a ptr to a plot at a specific position inside the _plots vector
    OGLSweepChart_C<ChartDataType_TP>* GetPlotPtr(unsigned int plot_idx);

    //! Returns a ptr to a plot by label
    OGLSweepChart_C<ChartDataType_TP>* GetPlotPtr(const std::string& plot_label);

   // Protected functions
protected:
    //! Initialize shader and models for drawing
    virtual void initializeGL();

    //! Called when the window is resized
    virtual void resizeGL(int width, int height);

    //! "Render-loop" - draws the scene
    virtual void paintGL();
    
    //!
    virtual void mousePressEvent(QMouseEvent* evt);
    
    //!
    virtual void mouseMoveEvent(QMouseEvent* evt);

    // Private helper functions
private:
    
    //! Initializes OpenGL functions
    void InitializeGLParameters();

    //! Creates a light cube
    void CreateLightSource();

    //! Initializes all shader programs required for the OGLCharts
    bool InitializeShaderProgramms();

    //! Creates a shader program from a fragment and vertex shader
    //! 
    //! \param The shader which is created
    //! \param vertex_path filepath to the vertex shader
    //! \param fragment_path filepath to the fragment shader
    //! \param attribute_locations all attribute locations to bind. The position inside the vector determines the location.
    //!         e.g the first string at position [0] is bound to the first attribute location 'zero'
    //! \returns true on success, false if not
    bool CreateShader(QOpenGLShaderProgram& shader, 
                      QString& vertex_path,
                      QString& fragment_path, 
                      std::vector<QString>& attribute_locations);

    // Private attributes
private:
    //! Projection matrix
    QMatrix4x4* _projection_mat;

    //! Model matrix
    QMatrix4x4* _model_mat;

    //! View matrix
    QMatrix4x4* _view_mat;

    //! Model-View-Projection matrix
    QMatrix4x4* _MVP;

    //! OpenGL shader
    QOpenGLShaderProgram _prog;
    
    //! 2d light shader
    QOpenGLShaderProgram _light_2d_shader;

    //! OpenGl light shader ( objects which are illuminated with light )
    QOpenGLShaderProgram _light_shader;

    //! OpenGL shader for light sources
    QOpenGLShaderProgram _light_source_shader;

    QOpenGLShaderProgram _text_shader;

    //! All plots contained in the current instance of this widget
    std::vector<OGLSweepChart_C<ChartDataType_TP>*> _plots;

    // Access functions to modify the plots 
    OGLSweepChart_C<ChartDataType_TP>* GetPlot(int plot_id);

    OGLSweepChart_C<ChartDataType_TP>* GetPlot(const std::string& plot_label); // iterates all plots and checks if a plot has plot_label

    Quad _light_source;

    //! number of frames since the start of the programm
	int _framecounter;

    //! OpenGL near plane parameter
	float _nearZ;

    //! OpenGL far plane parameter
	float _farZ;

    //! Issues the opengl render call at around 30 - 60 Hz
    QTimer* _paint_update_timer;

    //! Variable used as pseudo-timestamp
    int _pointcount;

    unsigned int _number_of_plots;

    unsigned int _chart_idx = 0;

    bool _ogl_initialized = false;
};
