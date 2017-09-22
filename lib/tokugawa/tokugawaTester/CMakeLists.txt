cmake_minimum_required(VERSION 2.8)
project( tokugawa_tester CXX )
add_executable( tokugawa_tester main.cpp )
target_link_libraries( tokugawa_tester ${OpenCV_LIBS} )
target_link_libraries( tokugawa_tester imagawa )
target_link_libraries( tokugawa_tester tokugawa )
target_link_libraries( tokugawa_tester polyclipping )

