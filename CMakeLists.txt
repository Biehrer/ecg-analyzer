cmake_minimum_required(VERSION 3.14.5)

# build customization: 
# set USE_VISUALIZATION_LIB TRUE if the visualization project should be built; set FALSE otherwise

# cleanup CmakeCache.txt 
# CMake reads vars always first from cache (if the variable is set there)
# If this cache value is removed each time, Cmake will always read this var from CmakeLists.txt

unset(USE_VISUALIZATION CACHE) 
set(USE_VISUALIZATION TRUE)
unset(USE_VISUALIZATION CACHE)
#unset(USE_VISUALIZATION_LIB CACHE) 
#option(USE_VISUALIZATION_LIB "Use visualization lib" ON)
#unset(USE_VISUALIZATION_LIB CACHE) 

#if(USE_VISUALIZATION_LIB)
#set(USE_VISUALIZATION TRUE)
#else()
#set(USE_VISUALIZATION FALSE)
#endif()

# The main project
project(signalanalyzer)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)

# add visualization and signal proc lib 
if(USE_VISUALIZATION)
    add_subdirectory(includes/visualization)
endif()

add_subdirectory(includes/signal_proc_lib)

add_executable(${PROJECT_NAME} main.cpp 
                      # ${CMAKE_CURRENT_SOURCE_DIR}/includes/time_series_player.h
                        ${CMAKE_CURRENT_SOURCE_DIR}/includes/jones_plot_app.h
                        ${CMAKE_CURRENT_SOURCE_DIR}/includes/jones_plot_app.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/includes/JonesPlot.ui

                        ${CMAKE_CURRENT_SOURCE_DIR}/includes/load_signal_dialog.h
                        ${CMAKE_CURRENT_SOURCE_DIR}/includes/load_signal_dialog.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/includes/loadsignaldialog.ui

                        ${CMAKE_CURRENT_SOURCE_DIR}/includes/create_sig_from_file_widget.h
                        ${CMAKE_CURRENT_SOURCE_DIR}/includes/create_sig_from_file_widget.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/includes/create_sig_from_file_widget.ui
                      )


target_include_directories(signalanalyzer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes/)


if(USE_VISUALIZATION)
    # link to the visualization lib
    target_link_libraries(${PROJECT_NAME} PUBLIC visualization)

    # define the variable USE_VISUALIZATION_CPP inside the cpp programm
    # syntax: CPP_NAME =&{CMAKE_NAME}
    target_compile_definitions(${PROJECT_NAME} PUBLIC USE_VISUALIZATION_CPP=${USE_VISUALIZATION}) 

     # copy the glsl shaders into the build directory for the visualization project
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Resources/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Resources/)
endif()


 # link to signal_proc_lib and Qt5
target_link_libraries(${PROJECT_NAME} PUBLIC
                                      signal_proc_lib
                                      Qt5::Widgets
                                      Qt5::Gui
                                      Qt5::Core
                                     )


target_compile_definitions(signalanalyzer PRIVATE __cplusplus)
