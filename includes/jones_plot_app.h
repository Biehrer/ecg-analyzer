#pragma once

// Project includes
#include "ui_JonesPlot.h"
//#include "../includes/visualization/ogl_plot_renderer_widget.h"
#include "../includes/signal_proc_lib/time_signal.h"
//#include "../includes/signal_proc_lib/time_signal.h"

#include "plot_model.h"

// Qt includes
#include <QtWidgets/QMainWindow>
#include <qopenglfunctions.h>
#include <QString>
#include <qobject.h>

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
    //void AddDataTest(double y_val, double x_val);

    //void SendDatatToPlots();

    //template<typename YValDataType_TP, typename XValDataType_TP>
    //static
    //void AddDataTest(const YValDataType_TP & y_val, const XValDataType_TP & x_val);
    void Setup();
private:
    //QOpenGLPlotRendererWidget* _plot_widget;
    Ui::JonesPlotClass ui;

    PlotModel_C _plot_model;
public slots:

    void OnButtonSettingsPage();

    void OnButtonHomePage();
};
