#pragma once
// Static macros cppunit
#include <cppunit/extensions/HelperMacros.h>

// Project includes
#include "../../signal_proc_lib/pan_topkins_qrs_detector.h"


// STL includes
#include <iostream>

class PanTokpinsQRSDetectorTest : public CPPUNIT_NS::TestFixture{

private:
    CPPUNIT_TEST_SUITE(PanTokpinsQRSDetectorTest);
    CPPUNIT_TEST(testFiltering);
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

    void testFiltering()
    {
        CPPUNIT_ASSERT(0 == 0);
        CPPUNIT_ASSERT_MESSAGE("compare test", 0 == 1);
        CPPUNIT_ASSERT(1== 2);
    }

    //void testAddition()
    //{
    //    CPPUNIT_ASSERT(*m_10_1 + *m_1_1 == *m_11_2);
    //}
};

CPPUNIT_TEST_SUITE_REGISTRATION(PanTokpinsQRSDetectorTest);