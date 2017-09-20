#pragma once
#include <vector>
#include "imagawa.hpp"

namespace tk {
  void tk_hello();

  std::vector<im::Answer> search(std::vector<std::vector<im::Piece>> problem, std::vector<im::Answer> hint, int algo);
}
