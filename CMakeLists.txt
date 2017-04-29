cmake_minimum_required(VERSION 2.8)
project( drtut CXX )
find_package( OpenCV REQUIRED )

include_directories(${CMAKE_SOURCE_DIR}/include)
set(CMAKE_CXX_FLAGS "-std=c++11 -O2")
set(CMAKE_CXX_FLAGS_DEBUG, "-Wall")
set(CMAKE_BUILD_TYPE Release)

add_subdirectory( src )
