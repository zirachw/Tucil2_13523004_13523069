cmake_minimum_required(VERSION 4.0)

project(quadtree)

set(OpenCV_DIR C:/opencv/build)
find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})