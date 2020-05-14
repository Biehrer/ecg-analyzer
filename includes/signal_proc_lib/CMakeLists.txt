cmake_minimum_required(VERSION 2.8)

project(signal_proc_lib)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)

add_library( signal_proc_lib #${PROJECT_NAME} 
time_signal.h
time_signal.cpp
)

 # link libs
target_link_libraries(signal_proc_lib Qt5::Widgets)
target_link_libraries(signal_proc_lib Qt5::Gui)
target_link_libraries(signal_proc_lib Qt5::Core)

# should this be PUBLIC or better INTERFACE? 
target_include_directories(${PROJECT_NAME} INTERFACE includes/signal_proc_lib)
