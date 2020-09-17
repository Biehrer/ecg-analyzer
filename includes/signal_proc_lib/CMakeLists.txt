cmake_minimum_required(VERSION 3.14.5)

project(signal_proc_lib)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Qt library
find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)

# WFDP library
find_path(WFDB_INCLUDE_DIR wfdb.h)
find_library(WFDB_LIBRARY wfdb)

if (WFDB_INCLUDE_DIR AND WFDB_LIBRARY)
    # you may need that if further action in your CMakeLists.txt depends
    # on detecting your library
    set(WFDB_FOUND TRUE)
    message(WFDB found = ${WFDB_FOUND})
    # you may need that if you want to conditionally compile some parts
    # of your code depending on library availability
    #add_definitions-DHAVE_LIBWFDB=1)

    set(WFDB_LIBRARY_LOC ${WFDB_LIBRARY})
endif ()

## KFR DSP library
find_path(KFR_INCLUDE_DIR kfr.h)
find_library(KFR_LIBRARY kfr)
if (KFR_INCLUDE_DIR AND KFR_LIBRARY)
    set(KFR_FOUND TRUE)
    message(KFR found = ${KFR_FOUND})
endif()

add_library(signal_proc_lib
                            file_io.h
                            mit_file_io.h
                            time_signal.h
                            rt_state_filters.h
                            pan_topkins_qrs_detector.h )

target_link_libraries(signal_proc_lib
                                      Qt5::Widgets
                                      Qt5::Gui
                                      Qt5::Core
                                      ${WFDB_LIBRARY_LOC}
                                      ${KFR_LIBRARY}
)

target_include_directories(signal_proc_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes/signal_proc_lib)

if(WFDB_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${WFDB_INCLUDE_DIR})
endif()

if(KFR_FOUND)
    target_include_directories(signal_proc_lib PUBLIC ${KFR_INCLUDE_DIR})
endif()

target_compile_definitions(signal_proc_lib PRIVATE __cplusplus)
