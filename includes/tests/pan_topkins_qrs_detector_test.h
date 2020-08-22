#pragma once

// Project includes
#include "../signal_proc_lib/pan_topkins_qrs_detector.h"

// STL includes
#include <iostream>

class PanTokpinsQRSDetectorTest : public CppUnit::TestFixture  {
 
public:
    void setUp()
    {
        std::cout << "setup";
    }

    void tearDown()
    {
        std::cout << "tear down";
    }

    void testFiltering()
    {
        CPPUNIT_ASSERT(0 == 0);
        CPPUNIT_ASSERT(1== 2);
    }

    //void testAddition()
    //{
    //    CPPUNIT_ASSERT(*m_10_1 + *m_1_1 == *m_11_2);
    //}
};