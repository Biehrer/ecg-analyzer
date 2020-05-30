#pragma once

// Project includes
#include "ui_JonesPlot.h"
//#include "../includes/signal_proc_lib/time_signal.h"

#include "load_signal_dialog.h"

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
    void Setup();
private:
    Ui::JonesPlotClass ui;

    PlotModel_C _plot_model;

public slots:

    void OnButtonSettingsPage();

    void OnButtonHomePage();

    void OnButtonSignalsPage();

    void OnGainChanged(int new_gain);
};
