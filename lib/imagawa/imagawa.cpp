#include "imagawa.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>

Point::Point() : Point(0, 0) {}

Point::Point(int xx, int yy) : x(xx), y(yy) {}

void im::hello() {
  std::cout << "hello 今川" << std::endl;
}

std::vector<cv::Vec4i> im::detectSegments(const cv::Mat &binaryImg) {

}

std::vector<Point> im::detectVertexes(const std::vector<cv::Vec4i> &segments) {

}
