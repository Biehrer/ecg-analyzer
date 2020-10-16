#pragma once

// Project includes
#include "../../visualization/ogl_plot_renderer_widget.h"

// Qt includes
#include <QtWidgets/QApplication>
#include <QObject>
#include <qtimer>
#include <qtimer.h>
#include <qobject.h>

// STL includes
#include <iostream>
#include <string>
class TextRendererTestIndependent : public QObject {
    Q_OBJECT

public:
        TextRendererTestIndependent();
        ~TextRendererTestIndependent();

public:

    void Initialize();
    void TestSetText();

signals:
    void NewTimerEvent();

public slots:
    void OnNewMajortickYAxesValue();

private:
    QTimer* _timer;

    PlotModel_C _model;

    QOpenGLPlotRendererWidget _renderer;

    const std::string _test_name = "TextRendererTest";
};

// Instead of inline: Make a own .cpp file
inline
TextRendererTestIndependent::TextRendererTestIndependent() 
{
    _timer = new QTimer();
    connect(_timer, SIGNAL(timeout()), this, SLOT(OnNewMajortickYAxesValue()) );
}


inline
TextRendererTestIndependent::~TextRendererTestIndependent()
{
    delete _timer;
}

inline
void
TextRendererTestIndependent::Initialize()
{ 

    _renderer.SetPlotModel(&_model);
    _renderer.show();
    // requires an OpenGL context -> would it therefore be better to put the PlotModel_C inside the OpenGLPlotRendererWidget, so we can call
    // FastInitializePot() on the OpenGlPlotrenderWidget?
    _model.InitializePlots(2, 400, 600, 1000, { { -10, 10 },{ -10, 10 } });
    _renderer.StartPaint();

    auto* plot = _model.GetPlotPtr(0);
    plot->SetMajorTickValueYAxes(1.0);

    plot->SetMajorTickValueYAxes(5.0);


    //// Reproduces the bug
    //_timer->setInterval(5000);
    //_timer->start();
}

inline
void 
TextRendererTestIndependent::OnNewMajortickYAxesValue()
{
    //auto* plot = _model.GetPlotPtr(0);
    //plot->SetMajorTickValueYAxes(1.0);
    //_renderer.StopPaint();
    //_model.SetMajorTickValueYAxes(0, 1.0);
    //_renderer.StartPaint();
}

inline
void 
TextRendererTestIndependent::TestSetText()
{
    auto* plot = _model.GetPlotPtr(0);
    plot->SetMajorTickValueYAxes(1.0);
    //plot->SetMaxValueYAxes(20.0);
    // 10.0 does work like a charm
    //plot->SetMaxValueYAxes(10.0);
    // 20.0 does work like a charm
    //plot->SetMaxValueYAxes(20.0);
    // 9.0 Does not Work (its an odd value)! 
    // -> messes up the surface grid and the text descriptions of the plot! (but not the textures themself, as in the app)
    // investigate this!
    plot->SetMaxValueYAxes(9.0); 
    
    double sample_freq_hz = 1000.0;

    for ( unsigned int num_sample = 0; num_sample < 700; ++num_sample ) {
        plot->AddDatapoint(10.0, Timestamp_TP(num_sample / sample_freq_hz));
    }

}

