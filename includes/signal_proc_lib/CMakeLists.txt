cmake_minimum_required(VERSION 3.14.5)

project(signal_proc_lib)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)

find_path(WFDB_INCLUDE_DIR wfdb.h)
find_library(WFDB_LIBRARY wfdb)

if (WFDB_INCLUDE_DIR AND WFDB_LIBRARY)
    # you may need that if further action in your CMakeLists.txt depends
    # on detecting your library
    set(WFDB_FOUND TRUE)

    # you may need that if you want to conditionally compile some parts
    # of your code depending on library availability
    #add_definitions-DHAVE_LIBWFDB=1)

    #include_directories(${WFDB_INCLUDE_DIR})
    set(WFDB_LIBRARY_LOC ${WFDB_LIBRARY})
endif ()


add_library( signal_proc_lib
                            file_io.h
                            mit_file_io.h
                            time_signal.h
)

 # link libs
target_link_libraries(signal_proc_lib Qt5::Widgets
                                      Qt5::Gui
                                      Qt5::Core
                                      ${WFDB_LIBRARY_LOC}
                       )

# should this be PUBLIC or better INTERFACE? 
#target_include_directories(${PROJECT_NAME} INTERFACE includes/signal_proc_lib)
target_include_directories(signal_proc_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes/signal_proc_lib)

if(WFDB_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${WFDB_INCLUDE_DIR})
endif()