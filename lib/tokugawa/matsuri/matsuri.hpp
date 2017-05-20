#pragma once

#include "../tokugawa.hpp"

namespace tk {
  std::vector<im::Answer> matsuriSearch(std::vector<std::vector<im::Piece>> problem, std::vector<im::Answer> hint);
}
