#include "tokugawa.hpp"
#include "matsuri/matsuri.hpp"
#include <iostream>
#include <stdexcept>
#include <sstream>

void tk::tk_hello()
{
  std::cout << "hello 徳川" << std::endl;
}

std::vector<im::Answer> tk::search(const im::Piece& waku, const std::vector<im::Piece>& problem, const std::vector<im::Hint>& hint = {}, const int algo = 0)
{
  switch (algo) {
  case 0:
    return tk::matsuri_search(waku, problem, hint);
  default:
    std::stringstream ss;
    ss << "No such a algorithm number : " << algo;
    throw std::invalid_argument(ss.str());
  }
}
