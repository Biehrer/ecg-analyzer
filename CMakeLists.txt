cmake_minimum_required(VERSION 2.4)

project(signalanalyzer)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core)
find_package(Qt5Widgets)

add_executable(${PROJECT_NAME} "main.cpp"
                                "JonesPlot.h" "JonesPlot.cpp"
                                "OGLChart.h" "OGLChart.cpp"
                                "QOpenGLPlotWidget.h" "QOpenGLPlotWidget.cpp"
                                # "ShapeGenerator.h" "ShapeGenerator.cpp"
                                # "ShapeData.h" "ShapeData.cpp"
                                "CircularBuffer.h"
                                )

 # link libs
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

target_link_libraries(${PROJECT_NAME} Qt5::Core)
