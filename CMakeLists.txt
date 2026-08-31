cmake_minimum_required(VERSION 3.28)
project(Lanelet2Project VERSION 1.0 DESCRIPTION "Learning Handling maps" LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(lanelet2_io REQUIRED)
find_package(lanelet2_projection REQUIRED)

add_executable(Lanelet2Project src/main.cpp)
target_link_libraries(Lanelet2Project PRIVATE ${lanelet2_io_LIBRARIES} ${lanelet2_projection_LIBRARIES})