#pragma once

// Project includes
#include "OGLChart.h"

#include <ShapeData.h>
#include <ShapeGenerator.h>

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
#include <qopenglfunctions_3_2_core.h>
#include <qopenglfunctions_3_3_core.h>
#include <qopenglextrafunctions.h>
#include <qopenglfunctions_3_1.h>
#include <qopenglbuffer.h>
#include <qopenglshaderprogram.h>
#include <qopengltexture.h>
#include <qopenglvertexarrayobject.h>


class QOpenGLPlotter : public QOpenGLWidget
{
	Q_OBJECT
public:
	QOpenGLPlotter(QWidget* parent = 0);
	~QOpenGLPlotter();

	QMatrix4x4* _projection_mat;
	QMatrix4x4* _model_mat;
	QMatrix4x4* _view_mat;
	QMatrix4x4* _MVP;

	//Shader Programms
	QOpenGLShaderProgram m_prog;
	QOpenGLShaderProgram m_shaderBullets;
	QOpenGLShaderProgram m_shaderTexture;
	QOpenGLShaderProgram m_shaderSkybox;
	QOpenGLShaderProgram m_shaderSphereTexture;

	QOpenGLTexture* texture;
	QOpenGLTexture* sphereTex;
	bool initShaders();
	void dataThreadFunc();
	
protected:
	//initialize shader and models for drawing
	virtual void initializeGL();
	//is called when the window is resized
	virtual void resizeGL(int width, int height);
	//draws the scene
	virtual void paintGL();

	virtual void mousePressEvent(QMouseEvent* evt);
	virtual void mouseMoveEvent(QMouseEvent* evt);

private:

	OGLChart* plot1;
	OGLChart* plot2;
	OGLChart* plot3;

	OGLChart* plot4;
	OGLChart* plot5;
	OGLChart* plot6;

	float _fov;

	//to keep track of game timing and to implement bullets with different flying speed
	double last_time = 0;
	double current_time = 0;
	double delta_time;
	double deltaTime = 0;
	int _framecounter;

	//Near/Far Plane
	float _nearZ;
	float _farZ;

	int _pointcount;

	unsigned int targetBufferOffset;
	int nFrames;
	//QElapsedTimer* ingameTimer;	 //keeps track of ingame time
	QTimer* update_timer;		// updates screen at around 30 - 60 Hz
	QTimer* dataUpdate_timer;
	QElapsedTimer* timestampsGen_ms;

public slots:
	void on_dataUpdate();
};


