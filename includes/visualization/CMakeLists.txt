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
message(STATUS "FREETYPE_INCLUDE_DIR_ft2build=${FREETYPE_INCLUDE_DIR_ft2build}")
message(STATUS "Freetype library location (dot lib file)=${FREETYPE_LIBRARIES}")
message(STATUS "Freetype include location=${FREETYPE_INCLUDE_DIRS}")

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
                            "plot_model.h"
                            "plot_model.cpp"
                            "plot_notation.h"
                            "line_notation.h"
           )

 # link libs
target_link_libraries(visualization PUBLIC # should be private
                                        Qt5::Widgets
                                        Qt5::Gui
                                        Qt5::Core
                                    #PUBLIC # should be public
                                         ${FREETYPE_LIBRARIES}
                                    )

# should this be PUBLIC or better INTERFACE? 
target_include_directories(visualization PUBLIC  
                                                   ${CMAKE_CURRENT_SOURCE_DIR}/includes/visualization
                                                   ${FREETYPE_INCLUDE_DIRS}
                                                   ${FREETYPE_DIR} # not necessary
                                                   )
