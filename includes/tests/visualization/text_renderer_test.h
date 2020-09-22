#pragma once

// Project includes
#include "../../visualization/ogl_plot_renderer_widget.h"
#include "../../visualization/text_renderer_2d.h"

// CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

// Qt includes
#include <QtWidgets/QApplication>

// STL includes
#include <iostream>
#include <string>
class TextRendererTest : public CppUnit::TestFixture {

private:
    CPPUNIT_TEST_SUITE(TextRendererTest);
    CPPUNIT_TEST(TestSetText);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();

    void tearDown();

    void TestSetText();

private:
    PlotModel_C _model;

    QOpenGLPlotRendererWidget _renderer;

    const std::string _test_name = "TextRendererTest";
};

void
TextRendererTest::setUp()
{
    _renderer.SetPlotModel(&_model);
    _renderer.show();
    // requires an OpenGL context -> would it therefore be better to put the PlotModel_C inside the OpenGLPlotRendererWidget, so we can call
    // FastInitializePot() on the OpenGlPlotrenderWidget?
    _model.FastInitializePlots(2, 400, 600, 1000, { { -10, 10 },{ -10, 10 } });
    _renderer.StartPaint();

   CPPUNIT_ASSERT_EQUAL_MESSAGE("Could not initialize OpenGL renderer", true, _renderer.IsOpenGLInitialized());
}

void 
TextRendererTest::TestSetText() 
{
    auto* plot = _model.GetPlotPtr(0);
    plot->SetMaxValueYAxes(20.0);
    // 10.0 does work like a charm
    plot->SetMaxValueYAxes(10.0);
    // 20.0 does work like a charm
    plot->SetMaxValueYAxes(20.0);
    // 9.0 Does not Work (its an odd value)! 
    // -> messes up the surface grid and the text descriptions of the plot! (but not the textures themself, as in the app)
    // investigate this!
    plot->SetMaxValueYAxes(9.0); 
    
    double sample_freq_hz = 1000.0;

    for ( unsigned int num_sample = 0; num_sample < 700; ++num_sample ) {
        plot->AddDatapoint(10.0, Timestamp_TP(num_sample / sample_freq_hz));
    }

}



void TextRendererTest::tearDown() {

    std::cout << "tear down" << std::endl;
}

CPPUNIT_TEST_SUITE_REGISTRATION(TextRendererTest);