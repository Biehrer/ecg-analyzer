// Project includes
//#include "text_renderer_test.h"
#include "text_renderer_test_independent.h"
// testing
//#include "../../visualization/ogl_plot_renderer_widget.h"

// CPPUnit includes
#include "cppunit/CompilerOutputter.h"
#include "cppunit/XmlOutputter.h"
#include "cppunit/extensions/TestFactoryRegistry.h"
#include "cppunit/ui/text/TestRunner.h"
#include "cppunit/TestResult.h"
#include "cppunit/TestResultCollector.h"


int main(int argc, char **argv) {
    // Do this ogl stuff before the line: QApplication a(argc, arv)
    //auto format = QSurfaceFormat::defaultFormat();
    //format.setSwapInterval(0);
    //QSurfaceFormat::setDefaultFormat(format);
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QApplication a(argc, argv);

    TextRendererTestIndependent test;
    test.Initialize();

    //CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    //CppUnit::TextUi::TestRunner runner;
    //runner.addTest(suite);

    //CppUnit::TestResult test_result;

    //CppUnit::TestResultCollector result_collector;
    //test_result.addListener(&result_collector);

    //runner.run(test_result);

    //// Output results 
    //CppUnit::CompilerOutputter comp_out(&result_collector, std::cerr);
    //comp_out.write();

    ////# pragma warning(push)
    ////# pragma warning(disable:4996)
    ////    char * env = std::getenv("SIGNALROCLIB_TEST_XML_OUTPUT_FILE");
    ////# pragma warning(pop)
    ////
    ////    std::string xml_fname = (nullptr == env) ? "signalproclib.test.results.xml" : std::string(env);
    ////
    ////    if ( xml_fname != "" ) {
    ////        std::cout << "Writing XML output to: " << xml_fname << "\n";
    ////
    ////        // In XML format
    ////        std::ofstream xml_os(xml_fname);
    ////        if ( !xml_os )
    ////            std::cerr << "WARNING: XML test output requested to " << xml_fname << ", but can't create file.\n";
    ////        else {
    ////            CppUnit::XmlOutputter xml_out(&result_collector, xml_os);
    ////            xml_out.write();
    ////        }
    ////    }
    ////
    //// Run the tests.
    //bool success = result_collector.wasSuccessful();

    //// Return error code 1 if the one of test failed.
    //std::cout << std::endl << success ? "success" : "fail";

    return a.exec();
}