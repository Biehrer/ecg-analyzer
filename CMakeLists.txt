cmake_minimum_required(VERSION 2.8)

project(signalanalyzer)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)

# add include directory (h/cpp files) 
add_subdirectory(includes/visualization)
add_subdirectory(includes/signal_proc_lib)

add_executable(${PROJECT_NAME} main.cpp)

# link to the visualization code
target_link_libraries(${PROJECT_NAME} INTERFACE visualization)
#target_link_libraries(${PROJECT_NAME} PUBLIC signal_proc_lib)

 # link to Qt
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} Qt5::Gui)
target_link_libraries(${PROJECT_NAME} Qt5::Core)

# copy the glsl shaders into the build directory for the visualization project
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Resources/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Resources/)
