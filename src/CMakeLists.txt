cmake_minimum_required(VERSION 2.8)
project( main CXX )
add_executable( main main.cpp )
target_link_libraries( main ${OpenCV_LIBS} )

