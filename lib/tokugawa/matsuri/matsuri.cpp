#include "matsuri.hpp"
#include "polyclipping/clipper.hpp"

const double PI = 4 * std::atan(1);

namespace cl = ClipperLib;

struct State {
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
static double degree(const cl::IntPoint& a, const cl::IntPoint& b, const cl::IntPoint& c, bool orientation)
{
  const cl::IntPoint va(a.X - b.X, a.Y - b.Y);
  const cl::IntPoint vb(c.X - b.X, c.Y - b.Y);
  auto len = [](auto v) {
    return std::sqrt(v.X * v.X + v.Y * v.Y);
  };
  const double cos = [](auto v, auto w) {
    return v.X * w.X + v.Y * w.Y;
  }(va, vb) / (len(va) * len(vb));
  double theta = std::acos(cos);
  const int op = [](auto v, auto w) {
    return v.X * w.Y - v.Y * w.X;
  }(va, vb);
  // CW で外積が負だったら OR CCW で外積が正だったら
  if ((orientation && op < 0)
      || (!orientation && op > 0)) {
    theta = 2 * PI - theta;
  }
  return theta;
}

// とんがり具合を返す。
// 値が大きいほどとんがっている。
// 角度の分散を求めることにした。
// O(n) : nが角の数
static double cal_togari(const State& s)
{
  const auto path = s.waku.front();
  const int n = path.size();
  const bool orientation = cl::Orientation(path);
  double ave = 0;
  std::vector<double> memo(n);
  for (int i = 0; i < n; ++i) {
    const auto left = path[(i - 1 + n) % n];
    const auto me = path[i];
    const auto right = path[(i + 1) % n];
    const double theta = degree(left, me, right, orientation);
    ave += theta;
    memo[i] = theta;
  }
  ave /= n;
  double sum = 0;
  for (const auto v : memo) {
    sum += (v - ave) * (v - ave);
  }
  sum /= n;
  return sum;
}

class MatsuriCompare
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
    if (vs.waku.front().size() != atom.waku.front().size()) {
      return vs.waku.front().size() < atom.waku.front().size();
    }
    // 角度の分散が小さい方を優先
    const auto ta = cal_togari(atom);
    const auto tb = cal_togari(vs);
    if (ta != tb) {
      return tb < ta;
    }
    return false;
  }
};

std::vector<im::Answer> tk::matsuri_search(const im::Piece& waku, const std::vector<im::Piece>& problem, const std::vector<im::Answer>& hint)
{
  std::vector<std::priority_queue<State, std::vector<State>, MatsuriCompare>> stacks(problem.size());
  return std::vector<im::Answer>();
}
