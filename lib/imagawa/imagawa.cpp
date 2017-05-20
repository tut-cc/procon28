#include "imagawa.hpp"
#include <iostream>

im::Point::Point() : Point(0, 0) {

}

im::Point::Point(int x_, int y_) : x(x_), y(y_) {}

im::Segment::Segment(const im::Point &p1_, const im::Point &p2_) : p1(p1_), p2(p2_) {}

void im::hello() {
  std::cout << "hello 今川" << std::endl;
}

std::vector<im::Segment> im::detectSegments() {

}

std::vector<im::Point> im::detectVertexes() {

}
