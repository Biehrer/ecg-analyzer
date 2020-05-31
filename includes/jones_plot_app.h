#pragma once

// Project includes
#include "ui_JonesPlot.h"
#include "signal_model.h"
#include "list_view_dialog.h"

// Qt includes
#include <QtWidgets/QMainWindow>
#include <qopenglfunctions.h>
#include <QString>
#include <qobject.h>
#include <qmessagebox.h>
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

public slots:
    // Pus
    void OnButtonSettingsPage();

    void OnButtonHomePage();

    void OnButtonSignalsPage();

    void OnBtnSelectSignal();

    // Select channel
    void OnBtnPlaySignal();

    void OnBtnPauseSignal();
    
    void OnBtnStopSignal();

    void OnNewSignalSelected(unsigned int signal_id);

    void OnGainChanged(int new_gain);

    void OnNewSignal(TimeSignal_C<int>);
    void OnNewSignal(TimeSignal_C<float>);
    void OnNewSignal(TimeSignal_C<double>);

private:
    Ui::JonesPlotClass ui;

    PlotModel_C _plot_model;

    SignalModel_C _signal_model;

    ListViewDialog_C* _list_view_signals;

    // The ID of the signal inside the _signal_model which will be plotted when the Start button is clicked
    unsigned int _current_signal_id = 0;

   std::atomic<bool> _is_signal_playing = false;
   std::atomic<bool> _is_stop_requested = false;

};
