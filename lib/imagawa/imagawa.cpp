#include "imagawa.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>

im::Point::Point() : Point(0, 0) {}

im::Point::Point(int xx, int yy) : x(xx), y(yy) {}

void im::hello() {
  std::cout << "hello 今川" << std::endl;
}

std::vector<std::pair<im::Point, im::Point>> im::detectSegments(const cv::Mat &img) {

}

std::vector<im::Point> im::detectVertexes(const std::vector<std::pair<Point, Point>> &segments) {

}
