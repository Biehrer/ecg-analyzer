cmake_minimum_required(VERSION 2.8)

project(signalanalyzer)

# build customization: 
# set ON if the visualization project should be built; set OFF otherwise
set(USE_VISUALIZATION OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)

# add include directory (h/cpp files) 
if(USE_VISUALIZATION)
add_subdirectory(includes/visualization)
endif()

add_subdirectory(includes/signal_proc_lib)

add_executable(${PROJECT_NAME} main.cpp)

# link to the visualization lib
if(USE_VISUALIZATION)
target_link_libraries(${PROJECT_NAME} PUBLIC visualization)
endif()

 # link to sig_proc_lib and Qt5
target_link_libraries(${PROJECT_NAME} PUBLIC
                                      signal_proc_lib
                                      Qt5::Widgets
                                      Qt5::Gui
                                      Qt5::Core)

# copy the glsl shaders into the build directory for the visualization project
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Resources/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Resources/)
