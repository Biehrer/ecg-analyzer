#pragma once

// Project includes
#include "ui_JonesPlot.h"
#include "signal_model.h"

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

    SignalModel_C _signal_model;

public slots:

    void OnButtonSettingsPage();

    void OnButtonHomePage();

    void OnButtonSignalsPage();

    void OnBtnSelectSignal();

    void OnBtnPlaySignal();
    //void OnBtnPauseSignal();
    //void OnBtnStopSignal();

    void OnGainChanged(int new_gain);

    void OnNewSignal(TimeSignal_C<int>);
    void OnNewSignal(TimeSignal_C<float>);
    void OnNewSignal(TimeSignal_C<double>);

};
