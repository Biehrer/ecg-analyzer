#pragma once


#include <iostream>
#include <string>
#include <mutex>

#include <qlist.h>
#include <qvector.h>
#include "qbuffer.h"
#include <qtimer.h>


#include <qopenglfunctions_3_3_core.h>
//#include <qopenglfunctions.h>
#include <qopenglbuffer.h>
#include <qopenglwidget.h>
//#include <qopenglfunctions_3_2_core.h>
#include <qopenglextrafunctions.h>
//#include <qopenglfunctions_3_1.h>
#include <qopenglshaderprogram.h>

#define TEMP_BUFFER_SIZE 10000

class OGLChart
{
public:

	OGLChart(int bufferSize, int screenPosX_S, int screenPosY_S, int chartWidth_S,int  chartHeight_S, int chartId = 0);
    ~OGLChart();

	void addData(float y, float x_ms);
	void updateChart();
	void drawNewestData();
	void drawAxes();
	void setupAxes();
	void deleteDataFromBeginning(int);
	void Draw();
	void sendDataToOGL();
	void addRange(int, QVector<double>);


private:
	QList<double>* _dataBuffer;
	QVector<double>* _dataBufferdouble_X;
	QVector<double>* _dataBufferdouble_Y;
	std::mutex* data_lock;

	QOpenGLBuffer chartVBO;
	QOpenGLBuffer _xAxisVBO;
	QOpenGLBuffer _yAxisVBO;

	int _chartPositionX_S;
	int _chartPositionY_S;
	int _chartWidth_S;
	int _chartHeight_S;

	int _bufferIndex;
	int _bufferSize;
	int _pointCount;
	
	int _maxY;
	int _minY;

    int _minX_ms;
    int _maxX_ms;

	int _x_dist;
	int _y_dist;
	//Input Buffer
	float* _tempDataBuffer;	//save values before adding them so we can add them at once and not each point for itself( this means less calls to glbuffersubdata)
	int _tempBufferIndex;

	int wrapped_number;
	bool needWrap;
	bool _wrappedOnce;


	int _chartId;
};


//this would be a waste of ressources..each plot means 10000 plotPoints and t
class plotPoint
{
public:

private:
	QOpenGLBuffer pointVBO;
};
