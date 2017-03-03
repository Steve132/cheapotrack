cmake_minimum_required(VERSION 2.8)


set(CMAKE_CXX_FLAGS "-std=c++11 -pthread")

add_executable(screentracker screentracker.cpp)
target_link_libraries(screentracker X11)
