cmake_minimum_required(VERSION 3.0)

add_library(driver driver.cpp)
target_link_libraries(driver serial)

add_executable(simpletest1 simpletest1.cpp)

