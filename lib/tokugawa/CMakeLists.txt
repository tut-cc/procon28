cmake_minimum_required(VERSION 2.8)
find_package( OpenCV REQUIRED )

set(CMAKE_BUILD_TYPE Release)

# set(serial "1.0.0")
# set(soserial "1")

add_library(tokugawa STATIC tokugawa.cpp)
target_link_libraries(tokugawa ${OPENCV_LIBS})

install(TARGETS tokugawa ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(FILES tokugawa.hpp DESTINATION ${CMAKE_SOURCE_DIR}/include)
