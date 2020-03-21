cmake_minimum_required(VERSION 2.4)

project(signalanalyzer)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)
find_package(Qt5Widgets)

add_executable(${PROJECT_NAME} "main.cpp")

# link libs
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

target_link_libraries(${PROJECT_NAME} Qt5::Core)
