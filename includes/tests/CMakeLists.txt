cmake_minimum_required(VERSION 3.14.5)

project(proj_tests)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Qt library
find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)

# cpp unit
find_path(CPP_UNIT_INCLUDE_DIR cppunit.h)
find_library(CPP_UNIT_LIB cppunit_lib)

# add test subdirs
add_subdirectory(signal_proc_lib)
add_subdirectory(visualization)

add_test(NAME MyTest COMMAND Test)

