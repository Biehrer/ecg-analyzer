#pragma once

// Project includes
#include "includes/ogl_sweep_chart.h"
#include "includes/ShapeData.h"
#include <includes/text_renderer_2d.h>
#include <includes/ShapeGenerator.h>

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

class QOpenGLPlotRendererWidget : public QOpenGLWidget
{
	Q_OBJECT

        // Construction / Destruction / Copying
public:
    QOpenGLPlotRendererWidget(QWidget* parent = 0);
    
    ~QOpenGLPlotRendererWidget();

    // Public access functions
public:
    void OnDataUpdateThreadFunction();

    void AddDataToAllPlots(float value_x, float value_y);

    const QMatrix4x4 GetModelViewProjection() const;

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
    //! Creates the OGL charts
    void InitializePlots(int number_of_plots);

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
    std::vector<OGLSweepChart_C<DrawingStyle_TP::LINE_STRIP>*> _plots;

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
};
