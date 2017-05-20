#pragma once

#include <utility>
#include <vector>

namespace cv {
  class Mat;
}

namespace im {
  class Point {
  public:
    Point();
    Point(int x, int y);

    int x, y;
  };

  void hello();

  std::vector<std::pair<Point, Point>> detectSegments(const cv::Mat &img);
  std::vector<Point> detectVertexes(const std::vector<std::pair<Point, Point>> &segments);
}
