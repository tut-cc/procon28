#include "matsuri.hpp"
#include "polyclipping/clipper.hpp"

#include <bitset>

const int MAX_NUM_OF_PEICES = 50;
const int TATE = 65;
const int YOKO = 101;
const double PI = 4 * std::atan(1);

namespace cl = ClipperLib;

struct Info {
  std::bitset<MAX_NUM_OF_PEICES> set;
  std::vector<im::Point> haiti;

  Info() : set(), haiti(MAX_NUM_OF_PEICES, im::Point(-1, -1)) {}
  Info(const std::bitset<MAX_NUM_OF_PEICES>& set, const std::vector<im::Point>& haiti) : set(set), haiti(haiti) {}
};

struct State {
  cl::Paths waku;
  cl::Paths uni;
  Info info;
  int bornus;
  int edges;

  State(const cl::Paths& waku, const cl::Paths& uni = cl::Paths(), const Info& info = Info(),
      int bornus = 0, int edges = 0)
      : waku(waku), uni(uni), info(info),
        bornus(bornus), edges(edges) {}
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
  }(cl::IntPoint(b.X - a.X, b.Y - a.Y), vb);
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
  bool operator() (State vs, State atom)
  {
    // 全消しを優先
    if (vs.waku.size() == 0) {
      return true;
    }
    if (atom.waku.size() == 0) {
      return false;
    }
    // bornus が多い方を優先
    if (vs.bornus != atom.bornus) {
      return vs.bornus < atom.bornus;
    }
    // 角数が少なくなっている方を優先
    if (vs.uni.size() && atom.uni.size()) {
      if (vs.edges - vs.uni.front().size() != atom.edges - atom.uni.front().size()) {
        return vs.edges - vs.uni.front().size() < atom.edges - atom.uni.front().size();
      }
    }
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

#include <iostream>
#include <string>
static void test_degree()
{
  cl::Paths test(1);
  test[0] << cl::IntPoint(0, 0) << cl::IntPoint(50, 100) << cl::IntPoint(100, 0);
  //test[0] << cl::IntPoint(0, 0) << cl::IntPoint(0, 2) << cl::IntPoint(1, 2)
  //  << cl::IntPoint(1, 1) << cl::IntPoint(2, 1) << cl::IntPoint(2, 2)
  //  << cl::IntPoint(3, 2) << cl::IntPoint(3, 0);
  State atom(test);
  const auto& v = atom.waku.front();
  const int n = v.size();
  const bool orientation = cl::Orientation(atom.waku.front());
  auto p2s = [](cl::IntPoint p) {
    return "(" + std::to_string(p.X) + ", " + std::to_string(p.Y) + ")";
  };
  std::cerr << "atom orientation : " << orientation << std::endl;
  for (int i = 0; i < n; ++i) {
    cl::IntPoint left(v[(i - 1 + n) % n].X, v[(i - 1 + n) % n].Y);
    cl::IntPoint me(v[i].X, v[i].Y);
    cl::IntPoint right(v[(i + 1) % n].X, v[(i + 1) % n].Y);
    auto theta = degree(left, me, right, orientation);
    std::cerr << "<" << p2s(left) << " " << p2s(me) << " " << p2s(right) << " : " << (theta * 180 / PI) << std::endl;
  }
}

static auto cut_waku(const cl::Paths& waku, const cl::Path& path)
{
  // waku から path を削り取る
  cl::Clipper clipper;
  clipper.AddPaths(waku, cl::ptSubject, true);
  clipper.AddPath(path, cl::ptClip, true);
  cl::Paths next_waku;
  clipper.Execute(cl::ctDifference, next_waku, cl::pftNonZero, cl::pftNonZero);
  // 穴が空いたり、2つ以上に分割したりしたらdameフラグを立てる
  bool dame = false;
  if (next_waku.size() > 1) {
    std::sort(next_waku.begin(), next_waku.end(), [](const auto& l, const auto& r) {
      return std::abs(cl::Area(l)) > std::abs(cl::Area(r));
    });
    // 誤差を考え、面積15以下は許容する。
    dame |= std::abs(cl::Area(next_waku[1])) > 15;
    next_waku.erase(next_waku.begin() + 1, next_waku.end());
  }
  // ぴったりハマるところがあればボーナスとしてカウント
  int count = 0;
  if (waku.size()) {
    std::set<std::vector<std::pair<cl::cInt, cl::cInt>>> set;
    for (int i = 0; i < path.size(); ++i) {
      auto left = path[i];
      auto right = path[(i + 1) % path.size()];
      auto lp = std::make_pair(left.X, left.Y);
      auto rp = std::make_pair(right.X, right.Y);
      if (lp > rp) {
        std::swap(lp, rp);
      }
      std::vector<std::pair<cl::cInt, cl::cInt>> vec(2);
      vec.push_back(lp);
      vec.push_back(rp);
      set.insert(vec);
    }
    for (int i = 0; i < waku.front().size(); ++i) {
      auto left = waku.front()[i];
      auto right = waku.front()[(i + 1) % waku.front().size()];
      auto lp = std::make_pair(left.X, left.Y);
      auto rp = std::make_pair(right.X, right.Y);
      if (lp > rp) {
        std::swap(lp, rp);
      }
      std::vector<std::pair<cl::cInt, cl::cInt>> vec(2);
      vec.push_back(lp);
      vec.push_back(rp);
      if (set.count(vec)) {
        ++count;
      }
    }
  }
  return std::tuple <bool, int, cl::Paths>(dame, count, next_waku);
}

static auto patch_uni(const cl::Paths& uni, const cl::Path& path)
{
  // uni に path を追加する
  cl::Clipper unioner;
  unioner.AddPaths(uni, cl::ptSubject, true);
  unioner.AddPath(path, cl::ptClip, true);
  cl::Paths next_uni;
  unioner.Execute(cl::ctUnion, next_uni, cl::pftNonZero, cl::pftNonZero);
  // 穴が空いたり、2つ以上の島ができたりしたらdameフラグを立てる
  bool dame = false;
  if (next_uni.size() > 1) {
    std::sort(next_uni.begin(), next_uni.end(), [](const auto& l, const auto& r) {
      return std::abs(cl::Area(l)) > std::abs(cl::Area(r));
    });
    // 誤差を考え、面積15以下は許容する。
    dame |= std::abs(cl::Area(next_uni[1])) > 15;
    next_uni.erase(next_uni.begin() + 1, next_uni.end());
  }
  // 一辺の長さが4グリッド弱以下ならdameフラグ
  if (next_uni.size() == 1) {
    auto v = next_uni.front();
    int n = v.size();
    for (int i = 0; i < n; ++i) {
      dame |= [](const cl::IntPoint& a, const cl::IntPoint& b) {
        return std::sqrt(std::pow(b.X - a.X, 2) + std::pow(b.Y - a.Y, 2));
      }(v[i], v[(i + 1) % n]) < 3.5;
    }
  }
  // 共通部分が大きかったらdameフラグを立てる
  cl::Clipper intersecter;
  intersecter.AddPaths(uni, cl::ptSubject, true);
  intersecter.AddPath(path, cl::ptClip, true);
  cl::Paths dst;
  intersecter.Execute(cl::ctIntersection, dst, cl::pftNonZero, cl::pftNonZero);
  for (const auto& v : dst) {
    // 誤差を考え、面積15以下は許容する。
    dame |= std::abs(cl::Area(v)) > 15;
  }
  // ぴったりハマるところがあればボーナスとしてカウント
  int count = 0;
  if (uni.size()) {
    std::set<std::vector<std::pair<cl::cInt, cl::cInt>>> set;
    for (int i = 0; i < path.size(); ++i) {
      auto left = path[i];
      auto right = path[(i + 1) % path.size()];
      auto lp = std::make_pair(left.X, left.Y);
      auto rp = std::make_pair(right.X, right.Y);
      if (lp > rp) {
        std::swap(lp, rp);
      }
      std::vector<std::pair<cl::cInt, cl::cInt>> vec(2);
      vec.push_back(lp);
      vec.push_back(rp);
      set.insert(vec);
    }
    for (int i = 0; i < uni.front().size(); ++i) {
      auto left = uni.front()[i];
      auto right = uni.front()[(i + 1) % uni.front().size()];
      auto lp = std::make_pair(left.X, left.Y);
      auto rp = std::make_pair(right.X, right.Y);
      if (lp > rp) {
        std::swap(lp, rp);
      }
      std::vector<std::pair<cl::cInt, cl::cInt>> vec(2);
      vec.push_back(lp);
      vec.push_back(rp);
      if (set.count(vec)) {
        ++count;
      }
    }
  }
  return std::tuple<bool, int, cl::Paths>(dame, count, next_uni);
}

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

union Color {
  unsigned int raw;
  unsigned char bytes[4];
};

static cv::Scalar uint2scalar(unsigned int _color)
{
  Color color = Color{ _color };
  return cv::Scalar(color.bytes[0], color.bytes[1], color.bytes[2], color.bytes[3]);
}

static void DrawPolygons(const cl::Paths& _paths, unsigned int fill_color, unsigned int line_color)
{
  if (_paths.size() == 0) {
    std::cerr << "nothing to draw" << std::endl;
  }
  cv::Mat img = cv::Mat::zeros(cv::Size(300, 300), CV_8UC4);
  std::vector<std::vector<cv::Point>> paths;
  std::vector<int> npts;

  img = uint2scalar(0x00FFFFFF);

  for (const auto& path : _paths) {
    std::vector<cv::Point> points;
    for (const auto& point : path) {
      points.push_back(cv::Point(point.X, point.Y));
    }
    paths.push_back(points);
    npts.push_back(points.size());
  }
  std::vector<cv::Point *> raw_paths(paths.size());
  for (int i = 0; i < paths.size(); ++i) {
    raw_paths[i] = &paths[i][0];
  }

  cv::fillPoly(img, (const cv::Point **) &raw_paths[0], &npts[0], paths.size(), uint2scalar(fill_color));
  cv::polylines(img, (const cv::Point **) &raw_paths[0], &npts[0], paths.size(), true, uint2scalar(line_color));

  cv::imshow("clipper sample", img);

  cv::waitKey(0);
  cv::destroyAllWindows();
}

class PointVecCompare {
public:
  bool operator()(const std::vector<im::Point> &left, const std::vector<im::Point> &right) const {
    if (left.size() != right.size()) {
      return left.size() < right.size();
    }
    const int n = left.size();
    for (int i = 0; i < n; ++i) {
      if (left[i].x != right[i].x) {
        return left[i].x < right[i].x;
      }
      if (left[i].y != right[i].y) {
        return left[i].y < right[i].y;
      }
    }
    return false;
  }
};

std::vector<im::Answer> tk::matsuri_search(const im::Piece& waku, const std::vector<im::Piece>& problem, const std::vector<im::Answer>& hint)
{
  const int n = problem.size();
  std::vector<std::priority_queue<State, std::vector<State>, MatsuriCompare>> stacks(n + 1);
  State atom(piece2paths(waku));
  stacks[0].push(atom);
  std::vector<cl::Path> paths;
  for (const auto& piece : problem) {
    paths.push_back(piece2paths(piece).front());
  }
  double best_score = 1 << 28;;
  Info best_info;
  int count = 0;
  std::set<std::vector<im::Point>, PointVecCompare> done;
  done.insert(atom.info.haiti);
  bool end = false;
  for (int g = 0;; ++g) {
    std::cerr << "---- " << g << " GENERATION ----" << std::endl;
    for (int i = 0; i <= n; ++i) {
      if (stacks[i].size() == 0) {
        continue;
      }
      State node = stacks[i].top();
      const int uni_size = node.uni.size() == 0 ? 0 : node.uni.front().size();
      const double waku_area = node.waku.size() == 0 ? 0 : std::abs(cl::Area(node.waku.front()));
      std::cerr << "[" << i << "] : " << node.bornus << ", " << node.edges << " - " << uni_size
        << " = " << node.edges - uni_size << " ... " << waku_area << std::endl;
      double score = node.waku.size() == 0 ? 0 : std::abs(cl::Area(node.waku.front()));
      if (best_score > score) {
        best_score = score;
        best_info = node.info;
        ++count;
        std::cerr << "(" << count << ") best score is renewed! : " << best_score << " at " << i << std::endl;
        if (count % 10 == 0 || i == n) {
          //DrawPolygons(node.waku, 0x160000FF, 0x600000FF); //blue
          //DrawPolygons(node.uni, 0x20FFFF00, 0x30FF0000); //orange
        }
        if (score < 1e-6) {
          end = true;
        }
      }
      stacks[i].pop();
      for (int j = 0; j < n; ++j) {
        //std::cerr << "\tusing " << j << "-th path" << std::endl;
        if (node.info.set[j]) {
          //std::cerr << "\t\tused, skip" << std::endl;
          continue;
        }
        auto next_set = node.info.set;
        next_set[j] = true;
        int awawa = 0;
        // pathsを当てはめる
        std::set<std::pair<int, int>> targets;
        for (const auto& wp : node.waku.front()) {
          for (const auto &pp : paths[j]) {
            int x = wp.X - pp.X;
            int y = wp.Y - pp.Y;
            targets.insert(std::make_pair(x, y));
          }
        }
        for (const auto& p : targets) {
          const int x = p.first;
          const int y = p.second;
          //for (int x = 0; x < YOKO; ++x) {
            //for (int y = 0; y < TATE; ++y) {
            const auto& path = paths[j];
            // はみ出しがある配置ならNG
            bool ok = true;
            for (const auto& point : path) {
              ok &= 0 <= x + point.X && x + point.X < YOKO
                && 0 <= y + point.Y && y + point.Y < TATE;
            }
            if (!ok) {
              continue;
            }
            cl::Path zurasied_path = path;
            for (auto& point : zurasied_path) {
              point.X += x;
              point.Y += y;
            }
            bool dame = false;
            auto ret_waku = cut_waku(node.waku, zurasied_path);
            dame |= std::get<0>(ret_waku);
            auto ret_uni = patch_uni(node.uni, zurasied_path);
            dame |= std::get<0>(ret_uni);
            if (dame) {
              continue;
            }
            auto bornus = std::get<1>(ret_waku) + std::get<1>(ret_uni);
            auto next_waku = std::get<2>(ret_waku);
            auto next_uni = std::get<2>(ret_uni);
            auto next_haiti = node.info.haiti;
            next_haiti[j] = im::Point(x, y);
            Info next_info(next_set, next_haiti);
            State next(next_waku, next_uni, next_info, node.bornus + bornus,  node.edges + path.size());
            if (done.count(next_haiti) > 0) {
              continue;
            }
            done.insert(next_haiti);
            stacks[i + 1].push(next);
            //++awawa;
            //cl::Paths tmp;
            //tmp << zurasied_path;
            //std::cerr << "zurashi" << x << ", " << y << std::endl;
            //DrawPolygons(node.waku, 0x160000FF, 0x600000FF); //blue
            //DrawPolygons(tmp, 0x20FFFF00, 0x30FF0000); //orange
            //DrawPolygons(next_waku, 0x3000FF00, 0xFF006600); //solution shaded green
          //}
        //}
        }
        //std::cerr << "\t\t" << awawa << " transed to next" << std::endl;
      }
    }
    if (end) {
      break;
    }
  }
  for (int i = 0; i < problem.size(); ++i) {
    std::cerr << "(" << i << ") [" << (best_info.set[i] ? "x" : " ") << "] : " << best_info.haiti[i].x << " " << best_info.haiti[i].y << std::endl;
  }
  return std::vector<im::Answer>();
}
