cmake_minimum_required(VERSION 2.8.4)
project(guo-hall-thinning)

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread")

set(SOURCE_FILES main.cpp thinning.h)
add_executable(guo-hall-thinning ${SOURCE_FILES})
target_link_libraries(guo-hall-thinning ${OpenCV_LIBS})
target_link_libraries(guo-hall-thinning ${Boost_LIBRARIES})