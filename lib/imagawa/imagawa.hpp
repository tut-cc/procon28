#pragma once

#include <utility>
#include <vector>

namespace cv {
  class Mat;
  class Vec4i;
}

namespace im {
  class Point {
  public:
    Point();
    Point(int x, int y);

    int x, y;
  };

  class Piece {
  public:
    Piece();
    Piece(int id, const std::vector<Point> &vertexes,
      const std::vector<double> &edges, const std::vector<double> &degs);

    int id;
    std::vector<Point> vertexes;

    std::vector<double> edges;
    std::vector<double> degs;
  };

  class Answer {
    Answer();
    Answer(int id, const std::vector<Point> &vertexes);

    int id;
    std::vector<Point> vertexes;
  };

  void hello();

  std::vector<cv::Mat> devideImg(const cv::Mat &binaryImg);
  std::vector<cv::Vec4i> detectSegments(const cv::Mat &binaryImg);
  std::vector<Point> detectVertexes(const std::vector<cv::Vec4i> &segments);
}
