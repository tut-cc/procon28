cmake_minimum_required(VERSION 2.8)
project( drtut CXX )
find_package( OpenCV REQUIRED )

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include)
#set(CMAKE_CXX_FLAGS "-std=c++14 -O2")
#My Setting
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG, "-Wall")
set(CMAKE_BUILD_TYPE Release)
#My Setting
set(CMAKE_C_COMPILE "gcc-6")
set(CMAKE_CXX_COMPILE "g++-6")

add_subdirectory( lib/clipper/cpp )
add_subdirectory( lib/imagawa )
add_subdirectory( lib/tokugawa )
add_subdirectory( src )
