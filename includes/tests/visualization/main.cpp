// Project includes
#include "text_renderer_test.h"

// CPPUnit includes
#include "cppunit/extensions/TestFactoryRegistry.h"
#include "cppunit/ui/text/TestRunner.h"
#include "cppunit/TestRunner.h"
#include "cppunit/TestCaller.h"
#include "cppunit/TestResult.h"

int main(int argc, char **argv)
{
    // DOes not work because the test runner is not build because of missing vc-compontns ( MFC vor VC x86 and x64):
    //CppUnit::TextUi::TestRunner runner;
    //CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    //runner.addTest(registry.makeTest());
    //bool wasSuccessful = runner.run("", false);
    //return !wasSuccessful;

    CppUnit::TestCaller<TextRendererTest> test("testFiltering",
    &TextRendererTest::testFiltering);
    CppUnit::TestResult result;
    test.run(&result);
}