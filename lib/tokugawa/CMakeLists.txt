cmake_minimum_required(VERSION 2.8)
find_package( OpenCV REQUIRED )

set(CMAKE_BUILD_TYPE Release)

# set(serial "1.0.0")
# set(soserial "1")

add_library(tokugawa STATIC tokugawa.cpp matsuri/matsuri.cpp)

target_link_libraries(tokugawa imagawa)
target_link_libraries(tokugawa ${OPENCV_LIBS})
target_link_libraries( tokugawa polyclipping )

install(TARGETS tokugawa ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/lib)
install(FILES tokugawa.hpp DESTINATION ${CMAKE_SOURCE_DIR}/include)
configure_file(tokugawa.hpp ${CMAKE_SOURCE_DIR}/include)

add_subdirectory( tokugawaTester )

