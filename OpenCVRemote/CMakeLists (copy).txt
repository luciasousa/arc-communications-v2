cmake_minimum_required(VERSION 3.15)
project(OpenCVRemote)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV)

set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED) 

find_package(Threads)


message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS}")



add_executable(OpenCVRemote main.cpp)

target_link_libraries(OpenCVRemote ${OpenCV_LIBS})
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(OpenCVRemote ${CURL_LIBRARIES})
target_link_libraries(OpenCVRemote ${CMAKE_THREAD_LIBS_INIT})
