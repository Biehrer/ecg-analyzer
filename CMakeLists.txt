cmake_minimum_required(VERSION 2.4)

project(signalanalyzer)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core)
find_package(Qt5Widgets)

# temporary hack to include freetype
set(ENV{FREETYPE_DIR} "C:/Development/libs/freetype-build/")
find_package(Freetype REQUIRED)
message(STATUS "FREETYPE_INCLUDE_DIR_ft2build=${FREETYPE_INCLUDE_DIR_ft2build}")

add_executable(${PROJECT_NAME} "main.cpp"
                                "JonesPlot.h" "JonesPlot.cpp"
                                "OGLChart.h" "OGLChart.cpp"
                                "QOpenGLPlotWidget.h" "QOpenGLPlotWidget.cpp"
                                 "ShapeGenerator.h" 
                                 "ShapeData.h"
								 "CircularBuffer.h"
                                )

 # link libs
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

target_link_libraries(${PROJECT_NAME} Qt5::Core)

# link freetype
target_link_libraries(${PROJECT_NAME} ${FREETYPE_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${FREETYPE_INCLUDE_DIRS})
