cmake_minimum_required(VERSION 2.8)
project( drtut CXX )
find_package( OpenCV REQUIRED )

include_directories(${CMAKE_SOURCE_DIR}/include)
set(CMAKE_CXX_FLAGS "-std=c++11 -O2")
set(CMAKE_CXX_FLAGS_DEBUG, "-Wall")
set(CMAKE_BUILD_TYPE Release)

add_subdirectory( lib )

# find_library(zhangsuen NAMES libzhangsuen.a PATHS ${CMAKE_SOURCE_DIR}/lib)
# find_library(preprocess NAMES libpreprocess.a PATHS ${CMAKE_SOUCE_DIR}/lib)

add_subdirectory( src )
link_directories( ${CMAKE_SOURCE_DIR}/lib )
