cmake_minimum_required(VERSION 2.8)


set(CMAKE_CXX_FLAGS "-std=c++11")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../firmware)

add_library(ct_driver 
	ct_driver.h 
	ct_driver.cpp
	ProtocolClient.h
	ProtocolClient.cpp
)
target_link_libraries(ct_driver serial)


add_executable(driver_dump1
	driver_dump1.cpp
)

target_link_libraries(driver_dump1 ct_driver)
