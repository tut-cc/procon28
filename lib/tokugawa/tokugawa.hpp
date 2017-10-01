#pragma once
#include <vector>
#include "imagawa.hpp"

namespace tk {
  void tk_hello();

  std::vector<im::Answer> search(const im::Piece& waku, const std::vector<im::Piece>& problem, const std::vector<im::Answer>& hint, const int algo);
}
