#pragma once
// Project includes
#include "ui_JonesPlot.h"
#include "ogl_plot_renderer_widget.h"

// Qt includes
#include <QtWidgets/QMainWindow>
#include <qopenglfunctions.h>
#include <QString>

// stl includes
#include <thread>

class JonesPlotApplication_C : public QMainWindow
{
	Q_OBJECT

public:
    JonesPlotApplication_C(QWidget *parent = Q_NULLPTR);
    ~JonesPlotApplication_C();

protected:
    // Called when the window is resized
    virtual void resizeEvent(QResizeEvent* event);

public:
    void SendDatatToPlots();

private:
    QOpenGLPlotRendererWidget* plot_widget;
    Ui::JonesPlotClass ui;

};
