#pragma once

#include "../tokugawa.hpp"

namespace tk {
  std::vector<im::Answer> matsuri_search(const im::Piece& waku, const std::vector<im::Piece>& problem, const std::vector<im::Answer>& hint);
}
