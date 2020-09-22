cmake_minimum_required(VERSION 3.14.5)

project(visualization_lib_test)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Qt library
find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)

# cpp unit
find_path(CPP_UNIT_INCLUDE_DIR cppunit.h)
find_library(CPP_UNIT_LIB cppunit_lib)


add_executable(visualization_lib_test   
                                    main.cpp
                                    text_renderer_test.h)



 # link libs
target_link_libraries(visualization_lib_test 
                                        #  PUBLIC
				                        Qt5::Widgets
                                        Qt5::Gui
                                        Qt5::Core
                                        ${CPP_UNIT_LIB} 
                                        visualization
			                            )

# include tests
target_include_directories(visualization_lib_test PUBLIC 
                                                         ${CMAKE_CURRENT_SOURCE_DIR}/includes/tests/visualization
                                                         ${CPP_UNIT_INCLUDE_DIR}
                                                         )

message(Source_test_dir= ${CMAKE_CURRENT_SOURCE_DIR})
message(try to read from=${CMAKE_CURRENT_SOURCE_DIR}/../../../Resources/)

message(Binary_test_dir=${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../../Resources/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Resources/)
