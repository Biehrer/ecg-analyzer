cmake_minimum_required(VERSION 3.14.5)

project(signal_proc_lib)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)

add_library( signal_proc_lib
                            file_io.h
                            file_io.cpp
                            time_signal.h
                            time_signal.cpp
                            time_series_player.h
                            time_series_player.cpp
)

 # link libs
target_link_libraries(signal_proc_lib Qt5::Widgets
                                      Qt5::Gui
                                      Qt5::Core)

# should this be PUBLIC or better INTERFACE? 
#target_include_directories(${PROJECT_NAME} INTERFACE includes/signal_proc_lib)
target_include_directories(signal_proc_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes/signal_proc_lib)
