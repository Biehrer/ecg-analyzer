cmake_minimum_required(VERSION 3.14.5)

project(visualization)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)

# temporary hack to include freetype
set(ENV{FREETYPE_DIR} "C:/Development/libs/freetype-build/")
find_package(Freetype REQUIRED)
#find_package(Freetype)
message(STATUS "FREETYPE_INCLUDE_DIR_ft2build=${FREETYPE_INCLUDE_DIR_ft2build}")

add_library( visualization 
                            "ogl_plot_renderer_widget.h"
                            "ogl_plot_renderer_widget.cpp"
                            "ogl_base_chart.h"
                            "ogl_base_chart.cpp"
                            "ogl_sweep_chart.h" 
                            "ogl_sweep_chart_buffer.h"
                            "ogl_chart_geometry_c.h"
                            "ogl_chart_geometry_c.cpp"
                            "chart_shapes_c.h"
                            "circular_buffer.h"
                            "text_renderer_2d.h"
                            "chart_types.h"
                            #"ShapeGenerator.h" 
                            #"ShapeData.h"
                            "plot_model.h"
                            "plot_model.cpp"
                            "plot_notation.h"
                            "line_notation.h"
           )

 # link libs
target_link_libraries(visualization Qt5::Widgets
                                    Qt5::Gui
                                    Qt5::Core
                                    )

# link freetype to this project
target_link_libraries(visualization ${FREETYPE_LIBRARIES} )

target_include_directories(${PROJECT_NAME} PUBLIC ${FREETYPE_INCLUDE_DIRS})

# should this be PUBLIC or better INTERFACE? 
target_include_directories(${PROJECT_NAME} PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/includes/visualization)
