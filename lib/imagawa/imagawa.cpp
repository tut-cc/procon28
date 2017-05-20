#include "imagawa.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <algorithm>
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

std::vector<cv::Mat> im::devideImg(const cv::Mat &binaryImg) {
  cv::Mat labels, stats, centroids;
  auto labelNum = cv::connectedComponentsWithStats(binaryImg, labels, stats, centroids);

  std::vector<cv::Mat> pieceImgs;
  for (auto i = 1; i < labelNum; i++) {
    auto *stat = stats.ptr<int>(i);

    auto l = std::max(stat[cv::ConnectedComponentsTypes::CC_STAT_LEFT] - 5, 0);
    auto t = std::max(stat[cv::ConnectedComponentsTypes::CC_STAT_TOP] - 5, 0);
    auto w = std::min(stat[cv::ConnectedComponentsTypes::CC_STAT_WIDTH] + 10, binaryImg.cols - l);
    auto h = std::min(stat[cv::ConnectedComponentsTypes::CC_STAT_HEIGHT] + 10, binaryImg.rows - t);
    auto s = stat[cv::ConnectedComponentsTypes::CC_STAT_AREA];

    if (s < 100) {
      continue;
    }

    auto pieceImg = binaryImg(cv::Rect(l, t, w, h)).clone();
    for (auto y = 0; y < h; y++) {
      auto *labelsRow = labels.ptr<int>(t + y);
      for (auto x = 0; x < w; x++) {
        if (labelsRow[l + x] != i) {
          pieceImg.data[w * y + x] = 0;
        }
      }
    }

    pieceImgs.push_back(pieceImg);
  }

  return pieceImgs;
}

std::vector<cv::Vec4i> im::detectSegments(const cv::Mat &binaryImg) {
  std::vector<cv::Vec4i> segments;
  auto lsd = cv::createLineSegmentDetector();
  lsd->detect(binaryImg, segments);

  return segments;
}

std::vector<im::Point> im::detectVertexes(const std::vector<cv::Vec4i> &segments) {
  return {};
}
