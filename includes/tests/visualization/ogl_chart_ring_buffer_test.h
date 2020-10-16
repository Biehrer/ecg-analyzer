#pragma once
// Static macros cppunit
#include <cppunit/extensions/HelperMacros.h>

// Project includes
#include "../../visualization/ogl_sweep_chart_buffer.h"
#include "../../visualization/circular_buffer.h"

// STL includes
#include <iostream>

using DataType_TP = float;

class OGLChartRingBufferTest_C : public CPPUNIT_NS::TestFixture {

private:
    CPPUNIT_TEST_SUITE(OGLChartRingBufferTest_C);
    CPPUNIT_TEST(TestFindIdxByTimestamp);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        std::cout << "setup" << std::endl;
    }

    void tearDown()
    {
        std::cout << "tear down" << std::endl;
    }

    void TestFindIdxByTimestamp()
    {
        int buffer_size = 128;
        int timerange_ms = 1000.0;
        int y_val = 1.0;
        double tstamp = 0.0;
        // Create the buffers
        RingBufferOptimized_TC<ChartPoint_TP<Position3D_TC<DataType_TP>>> _input_buffer(RingBufferSize_TP::Size128);
        OGLSweepChartBuffer_C<DataType_TP> _buffer(buffer_size, timerange_ms, _input_buffer);

        // Add some data
        _input_buffer.InsertAtTail(ChartPoint_TP<Position3D_TC<DataType_TP>>(Position3D_TC<DataType_TP>(0.0,
            0.0,
            0.0),
            tstamp));
        ++tstamp;

        _input_buffer.InsertAtTail(ChartPoint_TP<Position3D_TC<DataType_TP>>(Position3D_TC<DataType_TP>(0.0,
            0.0,
            0.0),
            tstamp));
        ++tstamp;
        
        // Check if we can find the idx of the first timestamp
        auto idx = _buffer.FindIdxToTimestampInsideData(0.0, _input_buffer.constData());

        CPPUNIT_ASSERT_MESSAGE("FindIdxByTimestamp", idx == 0);
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(OGLChartRingBufferTest_C);