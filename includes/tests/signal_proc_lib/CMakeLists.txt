cmake_minimum_required(VERSION 3.14.5)

project(signal_proc_lib_test)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Qt library
find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)

# signal proc library
#find_path(SIG_PROC_LIB_INCLUDE_DIR sig_proc_lib.h)
#find_library(SIG_PROC_LIB sig_proc_lib)

#if (SIG_PROC_LIB_INCLUDE_DIR AND SIG_PROC_LIB)
    # you may need that if further action in your CMakeLists.txt depends
    # on detecting your library
#    set(SIG_PROC_LIB_FOUND TRUE)
#    message(SIG_PROC_LIB found = ${SIG_PROC_LIB_FOUND})
    # you may need that if you want to conditionally compile some parts
    # of your code depending on library availability
    #add_definitions-DHAVE_LIBWFDB=1)

#    set(SIG_PROC_LIB_LIBRARY_LOC ${SIB_PROC_LIB})
#endif ()

# cpp unit
find_path(CPP_UNIT_INCLUDE_DIR cppunit.h)
find_library(CPP_UNIT_LIB cppunit_lib)
#find_package()
#find_package(Cppunit REQUIRED)

add_executable(signal_proc_lib_test   
                                    main.cpp
                                    pan_topkins_qrs_detector_test.h)

#add_library(signal_proc_lib_test # Alternative: Put pan_topkins_qrs_detector_test just inside the add_executable statement when this does not work
#                           pan_topkins_qrs_detector_test.h )

 # link libs
target_link_libraries(signal_proc_lib_test PUBLIC
				                          Qt5::Widgets
                                          Qt5::Gui
                                          Qt5::Core
                                     #     INTERFACE                                
                                       #   ${SIG_PROC_LIB_LIBRARY_LOC}
                                       signal_proc_lib
                                       
                                       ${CPP_UNIT_LIB} 

                                          )

# inlclude signal proc lib
if(SIG_PROC_LIB_FOUND )
#target_include_directories(signal_proc_lib_test PUBLIC ${SIG_PROC_LIB_INCLUDE_DIR}) #interface-> The test project does no?
endif()

# inlcude cppunit sources
#target_include_directories(signal_proc_lib_test PUBLIC ${CPP_UNIT_INCLUDE_DIR})


# include tests
target_include_directories(signal_proc_lib_test PUBLIC 
                                                     ${CMAKE_CURRENT_SOURCE_DIR}/includes/tests/signal_proc_lib
                                                     #${SIG_PROC_LIB_INCLUDE_DIR}
                                                      ${CPP_UNIT_INCLUDE_DIR}
                                                     )

