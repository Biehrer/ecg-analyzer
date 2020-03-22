#pragma once

// Qt includes
#include <QtWidgets/QMainWindow>
//#include <qopenglfunctions_3_3_core.h>
#include <qopenglfunctions.h>

// Project includes
#include "ui_JonesPlot.h"
#include "QOpenGLPlotter.h"

// stl includes
#include <thread>

class JonesPlot : public QMainWindow
{
	Q_OBJECT

public:
	JonesPlot(QWidget *parent = Q_NULLPTR);
	~JonesPlot();

private:
	QOpenGLPlotter* singlePlot;
	Ui::JonesPlotClass ui;
};
