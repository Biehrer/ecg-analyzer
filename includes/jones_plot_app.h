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
#include <functional>

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
    void AddDataTest(double y_val, double x_val);

    void SendDatatToPlots();

    //template<typename YValDataType_TP, typename XValDataType_TP>
    //static
    //void AddDataTest(const YValDataType_TP & y_val, const XValDataType_TP & x_val);

private:
    QOpenGLPlotRendererWidget* _plot_widget;
    Ui::JonesPlotClass ui;
};