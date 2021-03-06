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

  class Inter {
  public:
    Inter();

    Pointd p;
    double d2;
    int lr;
    bool f;
  };

  class Piece {
  public:
    Piece();
    Piece(int id, const std::vector<std::vector<Point>> &vertexes);

    int id;
    std::vector<std::vector<Point>> vertexes;
  };

  class Answer {
  public:
    Answer();
    Answer(int id, int index, const im::Point& point);

    const int id;
    const int index;
    const im::Point point;
  };

  class Hint {
  public:
    Hint(const std::vector<im::Point>& vertexes);

    const std::vector<im::Point> vertexes;
  };

  void hello();

  std::vector<cv::Mat> devideImg(const cv::Mat &binaryImg, std::vector<Point> &ps);
  std::vector<cv::Vec4i> detectSegments(const cv::Mat &edgeImg);
  std::vector<Pointd> detectVertexes(const std::vector<cv::Vec4i> &segments);
  Piece roll(const int id, const std::vector<im::Pointd>& _shape);
  Piece hint_roll(const int id, const std::vector<im::Pointd>& _shape);
  Piece easy_roll(const int id, const std::vector<im::Point>& _shape);
  void writeIDs(const std::vector<Point> &ps, cv::Mat &img, int firstID);
  void showAnswer(const std::vector<Answer> &anses, const std::vector<Piece> &pieces);

  bool cmpPieces(const std::vector<Point> &p1, const std::vector<Point> &p2);
  std::vector<Point> readShape();
}
