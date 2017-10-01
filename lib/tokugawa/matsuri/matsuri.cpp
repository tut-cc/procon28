#include "matsuri.hpp"
#include "polyclipping/clipper.hpp"

namespace cl = ClipperLib;

static struct State {
  cl::Paths waku;
  cl::Paths uni;

  State(const cl::Paths& waku, const cl::Paths& uni = cl::Paths()) {
    this->waku = waku;
    this->uni = uni;
  }
};

static cl::Paths piece2paths(const im::Piece& piece)
{
  cl::Path path;
  for (im::Point p : piece.vertexes) {
    path << cl::IntPoint(p.x, p.y);
  }
  cl::Paths dst;
  dst << path;
  return dst;
}

// <abcを返す
static double degree(const cl::IntPoint& a, const cl::IntPoint& b, const cl::IntPoint& c)
{

}

// とんがり具合を返す。
// 値が大きいほどとんがっている。
// O(n) : nが角の数
static double cal_togari(const State& s)
{
  int n = s.uni.front.size();
  double ave = 0;
}

static class MatsuriCompare
{
public:
  // vsの優先度が高ければtrueを返す
  bool operator() (State atom, State vs)
  {
    // 全消しを優先
    if (vs.waku.size() == 0) {
      return true;
    }
    if (atom.waku.size() == 0) {
      return false;
    }
    // 角数が少ない方を優先
    if (vs.waku.front.size() != atom.waku.front.size()) {
      return vs.waku.front.size() < atom.waku.front.size();
    }
    return false;
  }
};

std::vector<im::Answer> tk::matsuri_search(const im::Piece& waku, const std::vector<im::Piece>& problem, const std::vector<im::Answer>& hint)
{
  std::vector<std::priority_queue<State, std::vector<State>, MatsuriCompare>> stacks(problem.size());
  return std::vector<im::Answer>();
}
