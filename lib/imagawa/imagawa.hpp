#pragma once

#include <vector>

namespace im {
  // ‚Æ‚è‚ ‚¦‚¸‘S•”public
  class Point {
  public:
    Point();
    Point(int x, int y);

    int x, y;
  };

  class Segment {
  public:
    Segment(const Point &p1, const Point &p2);

    Point p1, p2;
  };

  void hello();

  std::vector<Segment> detectSegments();
  std::vector<Point> detectVertexes();
}
