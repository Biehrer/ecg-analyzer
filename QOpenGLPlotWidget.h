#pragma once

// Project includes
#include "OGLChart.h"

// STL includes
#include <iostream>
#include <string>
#include <cstdlib>

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
#include <qopengltexture.h>
#include <qopenglvertexarrayobject.h>

// Project includes

#include <math.h>
#include <time.h>
#include <thread>
#include <mutex>
#include <chrono>
#define SREENWIDTH 1680
#define SCREENHEIGHT 1050

//coding convention : _var = member Variable


class QOpenGLPlotWidget : public QOpenGLWidget
{
	Q_OBJECT
public:
    QOpenGLPlotWidget(QWidget* parent = 0);
    ~QOpenGLPlotWidget();

    bool InitializeShaderProgramms();
	void dataThreadFunc();
	
protected:
    // Initialize shader and models for drawing
	virtual void initializeGL();

    // Called when the window is resized
	virtual void resizeGL(int width, int height);

    // "Render-loop" - draws the scene
	virtual void paintGL();

	virtual void mousePressEvent(QMouseEvent* evt);
	virtual void mouseMoveEvent(QMouseEvent* evt);

private:
    //! Projection matrix
    QMatrix4x4* _projection_mat;

    //! Model matrix

    QMatrix4x4* _model_mat;
    //! View matrix
    QMatrix4x4* _view_mat;

    //! Model-View-Projection matrix
    QMatrix4x4* _MVP;

    // OpenGL shader
    QOpenGLShaderProgram _prog;

	OGLChart_C* plot1;
	OGLChart_C* plot2;
	OGLChart_C* plot3;

	OGLChart_C* plot4;
	OGLChart_C* plot5;
	OGLChart_C* plot6;

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
    QTimer* paint_update_timer;

    //! Adds new data to the chart through signal and slot system
    QTimer* dataUpdate_timer;

    //! Variable used as pseudo-timestamp
    int _pointcount;

public slots:
	void on_dataUpdate();
};


