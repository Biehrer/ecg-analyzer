cmake_minimum_required(VERSION 2.4)

project(visualization)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)

# temporary hack to include freetype
set(ENV{FREETYPE_DIR} "C:/Development/libs/freetype-build/")
find_package(Freetype REQUIRED)
message(STATUS "FREETYPE_INCLUDE_DIR_ft2build=${FREETYPE_INCLUDE_DIR_ft2build}")

add_library(${PROJECT_NAME} 
"JonesPlot.h" 
"JonesPlot.cpp"
"ogl_plot_renderer_widget.h"
"ogl_plot_renderer_widget.cpp"
"ogl_sweep_chart.h" 
"ogl_sweep_chart.cpp"
"ogl_sweep_chart_buffer.h"
"ogl_sweep_chart_buffer.cpp"
"ShapeGenerator.h" 
"ShapeData.h"
"CircularBuffer.h"
"text_renderer_2d.h"
"chart_types.h"
"JonesPlot.ui"
"ogl_chart_geometry_c.h"
"ogl_chart_geometry_c.cpp"
"chart_shapes_c.h"

)

 # link libs
target_link_libraries(visualization Qt5::Widgets)
target_link_libraries(visualization Qt5::Gui)
target_link_libraries(visualization Qt5::Core)

# link freetype
target_link_libraries(visualization ${FREETYPE_LIBRARIES})
target_include_directories(visualization PUBLIC ${FREETYPE_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME} PUBLIC includes)
