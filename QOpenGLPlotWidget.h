#pragma once

// Project includes
#include "OGLChart.h"
#include "ShapeData.h"
#include <text_renderer_2d.h>
#include <ShapeGenerator.h>

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

// Project defines
#define SREENWIDTH 1440
#define SCREENHEIGHT 800

class QOpenGLPlotWidget : public QOpenGLWidget
{
	Q_OBJECT
public:
    QOpenGLPlotWidget(QWidget* parent = 0);
    
    ~QOpenGLPlotWidget();

    // Public access functions
public:
    void OnDataUpdateThreadFunction();

    void AddDataToAllPlots(float value_x, float value_y);

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

    void InitializePlots(int number_of_plots);

    void InitializeGLParameters();

    void CreateLightSource();

    bool InitializeShaderProgramms();

    bool CreateShader(QOpenGLShaderProgram& shader, QString vertex_path, QString fragment_path, std::vector<QString>& uniforms);

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
    std::vector<OGLSweepChart_C*> _plots;

    Quad _light_source;

    // Keep track of game timing and to implement bullets with different flying speed
	double last_time = 0;
	double current_time = 0;
	double delta_time;
	double deltaTime = 0;

    //! number of frames since the start of the programm
	int _framecounter;

    //! OpenGL near plane parameter
	float _nearZ;

    //! OpenGL far plane parameter
	float _farZ;

    //! Issues the opengl render call at around 30 - 60 Hz
    QTimer* _paint_update_timer;

    //! Adds new data to the chart through signal and slot system
    QTimer* _data_update_timer;

    //! Variable used as pseudo-timestamp
    int _pointcount;

    OGLTextBox _text_box;

};


