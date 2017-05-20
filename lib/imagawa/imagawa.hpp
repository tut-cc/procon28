#pragma once

#include <utility>
#include <vector>

class Point {
public:
  Point();
  Point(int x, int y);

  int x, y;
};

namespace cv {
  class Mat;
  class Vec4i;
}

namespace im {
  void hello();

  std::vector<cv::Vec4i> detectSegments(const cv::Mat &binaryImg);
  std::vector<Point> detectVertexes(const std::vector<cv::Vec4i> &segments);
}
