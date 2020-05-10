cmake_minimum_required(VERSION 2.4)

project(signalanalyzer)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)

# add visualization code 
add_subdirectory(includes)

add_executable(${PROJECT_NAME} main.cpp)

# link to the visualization code
target_link_libraries(${PROJECT_NAME} PUBLIC visualization)

 # link to Qt
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} Qt5::Gui)
target_link_libraries(${PROJECT_NAME} Qt5::Core)

# copy the glsl shaders into the build directory for the visualization project
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Resources/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Resources/)
