#pragma once

// Project includes
#include "ogl_sweep_chart.h"
#include "text_renderer_2d.h"
#include "plot_model.h"

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
#include <qvector.h>
#include <qstring.h>
#include <qapplication.h>
// for the model
#include <QAbstractTableModel>

// Project defines
using ChartDataType_TP = float;

class QOpenGLPlotRendererWidget : public QOpenGLWidget
{
	Q_OBJECT
        // Construction / Destruction / Copying
public:
    QOpenGLPlotRendererWidget(QWidget* parent = 0);
    ~QOpenGLPlotRendererWidget();

    // Public access functions
public:    
    //! Returns the model view projection transform matrix
    const QMatrix4x4 GetModelViewProjection() const;

    //! \param true if InitializeGl() was called, false if not
    bool IsOpenGLInitialized() const;
    
    //! Set the plot model from which the data is rendered
    void SetPlotModel(PlotModel_C* model);

    //! Starts rendering (again)
    void StartPaint();
    
    //! Stops rendering
    void StopPaint();

    // signals & slots
public slots:
    void OnNewChangeRequest(int plot_id,
        const OGLPlotProperty_TP& type,
        const QVariant& value);

   // Protected functions
protected:
    //! Initialize shader and models for drawing
    virtual void initializeGL();

    //! Called when the window is resized
    virtual void resizeGL(int width, int height);

    //! "Render-loop" - draws the scene
    virtual void paintGL();
    
    // Private helper functions
private:
    //! Initializes OpenGL functions
    void InitializeGLParameters();

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
    QMatrix4x4 _projection_mat;

    //! Model matrix
    QMatrix4x4 _model_mat;

    //! View matrix
    QMatrix4x4 _view_mat;

    //! Model-View-Projection matrix
    QMatrix4x4 _MVP;

    //! OpenGL shader
    QOpenGLShaderProgram _prog;
    
    //! 2d light shader
    QOpenGLShaderProgram _light_2d_shader;

    //! OpenGl light shader ( objects which are illuminated with light )
    QOpenGLShaderProgram _light_shader;

    //! OpenGL shader for light sources
    QOpenGLShaderProgram _light_source_shader;

    //! Shader for font rendering
    QOpenGLShaderProgram _text_shader;

    //! number of frames since the start of the programm
	int _framecounter;

    //! OpenGL near plane parameter
	float _nearZ;

    //! OpenGL far plane parameter
	float _farZ;
    
    //! True, when opengl was initialized successfull
    bool _ogl_initialized = false;

    //! Issues the opengl render call at around 30 - 60 Hz
    QTimer* _paint_update_timer;

    //! (render)data model
    PlotModel_C* _model;  
};
