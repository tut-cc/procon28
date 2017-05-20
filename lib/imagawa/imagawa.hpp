#pragma once

#include <vector>

namespace im {
  class Point {};
  class Segment {};

  void hello();

  std::vector<Segment> detectSegments();
  std::vector<Point> detectVertexes();
}
