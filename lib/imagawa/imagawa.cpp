#include "imagawa.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>

im::Point::Point() : Point(0, 0) {}

im::Point::Point(int x_, int y_) : x(x_), y(y_) {}

im::Piece::Piece() : Piece(0, {}, {}, {}) {}

im::Piece::Piece(int id_, const std::vector<im::Point> &vertexes_, const std::vector<double> &edges_,
  const std::vector<double> &degs_) : id(id_), vertexes(vertexes_), edges(edges_), degs(degs_) {}

im::Answer::Answer() : Answer(0, {}) {}

im::Answer::Answer(int id_, const std::vector<im::Point> &vertexes_) : id(id_), vertexes(vertexes_) {}

void im::hello() {
  std::cout << "hello 今川" << std::endl;
}

std::vector<cv::Vec4i> im::detectSegments(const cv::Mat &binaryImg) {
  std::vector<cv::Vec4i> segments;
  auto lsd = cv::createLineSegmentDetector();
  lsd->detect(binaryImg, segments);
}

std::vector<im::Point> im::detectVertexes(const std::vector<cv::Vec4i> &segments) {

}
