#include "tokugawa.hpp"
#include "matsuri/matsuri.hpp"
#include <iostream>
#include <stdexcept>
#include <sstream>

void tk::hello()
{
  std::cout << "hello 徳川" << std::endl;
}

std::vector<im::Answer> tk::search(std::vector<std::vector<im::Piece>> problem, std::vector<im::Answer> hint = {}, int algo = 0)
{
  switch (algo) {
  case 0:
    return tk::matsuriSearch(problem, hint);
  default:
    std::stringstream ss;
    ss << "No such a algorithm number : " << algo;
    throw std::invalid_argument(ss.str());
  }
}
