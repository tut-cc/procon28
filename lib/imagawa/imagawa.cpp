#include "imagawa.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>

Point::Point() : Point(0, 0) {}

Point::Point(int x_, int y_) : x(x_), y(y_) {}

Piece::Piece() : Piece(0, {}, {}, {}) {}

Piece::Piece(int id_, const std::vector<Point> &vertexes_, const std::vector<double> &edges_,
  const std::vector<double> &degs_) : id(id_), vertexes(vertexes_), edges(edges_), degs(degs_) {}

Answer::Answer() : Answer(0, {}) {}

Answer::Answer(int id_, const std::vector<Point> &vertexes_) : id(id_), vertexes(vertexes_) {}

void im::hello() {
  std::cout << "hello 今川" << std::endl;
}

std::vector<cv::Vec4i> im::detectSegments(const cv::Mat &binaryImg) {
  std::vector<cv::Vec4i> segments;
  auto lsd = cv::createLineSegmentDetector();
  lsd->detect(binaryImg, segments);
}

std::vector<Point> im::detectVertexes(const std::vector<cv::Vec4i> &segments) {

}
