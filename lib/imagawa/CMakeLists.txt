cmake_minimum_required(VERSION 2.8)
find_package( OpenCV REQUIRED )

set(CMAKE_BUILD_TYPE Release)

# set(serial "1.0.0")
# set(soserial "1")

add_library(imagawa STATIC imagawa.cpp)
target_link_libraries(imagawa ${OPENCV_LIBS})

#install(TARGETS imagawa ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(TARGETS imagawa ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR})
install(FILES imagawa.hpp DESTINATION ${CMAKE_SOURCE_DIR}/include)
configure_file(imagawa.hpp ${CMAKE_SOURCE_DIR}/include)
