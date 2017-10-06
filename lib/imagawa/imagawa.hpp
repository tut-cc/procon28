#pragma once

#include <utility>
#include <vector>

namespace cv {
  class Mat;
  template<typename _Tp, int cn> class Vec;
  typedef Vec<int, 4> Vec4i;
}

namespace im {
  class Point {
  public:
    Point();
    Point(int x, int y);

    int x, y;
  };

  class Pointd {
  public:
    Pointd();
    Pointd(double x, double y);

    double x, y;
  };

  class Edge {
  public:
    Edge();

    Pointd ends[3];
    int ptns[3];
    int pLR[3];
  };

  class Piece {
  public:
    Piece();
    Piece(int id, const std::vector<Point> &vertexes,
      const std::vector<int> &edges2, const std::vector<double> &degs);

    int id;
    std::vector<Point> vertexes;

    std::vector<int> edges2;
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
  std::vector<cv::Vec4i> detectSegments(const cv::Mat &edgeImg);
  std::vector<Pointd> detectVertexes(const std::vector<cv::Vec4i> &segments);
  std::vector<std::vector<im::Point>> roll(const std::vector<cv::Vec4i> segments, const std::vector<im::Pointd> shape);
}
