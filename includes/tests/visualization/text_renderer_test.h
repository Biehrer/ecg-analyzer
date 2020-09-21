#pragma once

// Project includes
//#include "../visualization/text_renderer_2d.h"
//#include "text_renderer_2d.h"
#include "../../visualization/text_renderer_2d.h"
//#include "text_renderer_2d.h"

// STL includes
#include <iostream>

#include "cppunit/extensions/HelperMacros.h"
//#include "cppunit/TestFixture.h"

class TextRendererTest : public CppUnit::TestFixture  {
 
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