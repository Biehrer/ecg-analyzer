// CPPUnit includes
#include "cppunit/CompilerOutputter.h"
#include "cppunit/XmlOutputter.h"
#include "cppunit/extensions/TestFactoryRegistry.h"
#include "cppunit/ui/text/TestRunner.h"
#include "cppunit/TestResult.h"
#include "cppunit/TestResultCollector.h"

// STL includes
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>

int main(int argc, char **argv)
{
    CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    CppUnit::TextUi::TestRunner runner;
    runner.addTest(suite);

    CppUnit::TestResult test_result;

    CppUnit::TestResultCollector result_collector;
    test_result.addListener(&result_collector);

    runner.run(test_result);

    // Output results 
    CppUnit::CompilerOutputter comp_out(&result_collector, std::cerr);
    comp_out.write();


    //# pragma warning(push)
//# pragma warning(disable:4996)
//    char * env = std::getenv("SIGNALROCLIB_TEST_XML_OUTPUT_FILE");
//# pragma warning(pop)
//
//    std::string xml_fname = (nullptr == env) ? "signalproclib.test.results.xml" : std::string(env);
//
//    if ( xml_fname != "" ) {
//        std::cout << "Writing XML output to: " << xml_fname << "\n";
//
//        // In XML format
//        std::ofstream xml_os(xml_fname);
//        if ( !xml_os )
//            std::cerr << "WARNING: XML test output requested to " << xml_fname << ",  can't create file.\n";
//        else {
//            CppUnit::XmlOutputter xml_out(&result_collector, xml_os);
//            xml_out.write();
//        }
//    }
//
    // Run the tests.
    bool wasSucessful = result_collector.wasSuccessful();

    // Return error code 1 if the one of test failed.
    return wasSucessful ? 0 : 1;

    //std::cout << "press key to close.." << std::endl;
    //int a = 0;
    //std::cin >> a;
}