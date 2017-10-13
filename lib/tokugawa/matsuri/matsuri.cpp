#include "matsuri.hpp"
#include "polyclipping/clipper.hpp"

#include <bitset>
#include <cmath>
#include <fstream>
#include <map>
#include <iostream>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>


const int MAX_NUM_OF_PEICES = 50;
const int TATE = 65;
const int YOKO = 101;
const double PI = 4 * std::atan(1);

namespace cl = ClipperLib;



static cv::Scalar uint2scalar(unsigned int _color);
static void DrawPolygons(const cl::Paths& _paths, unsigned int fill_color, unsigned int line_color);

struct Info {
  std::bitset<MAX_NUM_OF_PEICES> set;
  std::vector<im::Point> haiti;
  std::vector<int> indexes;

  Info() : set(), haiti(MAX_NUM_OF_PEICES, im::Point(-1, -1)), indexes(MAX_NUM_OF_PEICES, -1) {}
  Info(const std::bitset<MAX_NUM_OF_PEICES>& set, const std::vector<im::Point>& haiti, const std::vector<int>& indexes) : set(set), haiti(haiti), indexes(indexes) {}
};

struct State {
  cl::Paths waku;
  cl::Paths uni;
  Info info;
  int bornus;
  int edges;
  double area;

  State(const cl::Paths& waku, const cl::Paths& uni = cl::Paths(), const Info& info = Info(),
    int bornus = 0, int edges = 0, double area = 0)
    : waku(waku), uni(uni), info(info),
    bornus(bornus), edges(edges), area(area) {}
};

static auto piece2paths(const im::Piece& piece)
{
  cl::Paths paths;
  for (const std::vector<im::Point>& vec : piece.vertexes) {
    cl::Path path;
    for (const im::Point& p : vec) {
      path << cl::IntPoint(p.x, p.y);
    }
    paths << path;
  }
  return paths;
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
  bool operator() (const State& vs, const State& atom) const
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
    /*if (vs.area != atom.area) {
      return vs.area < atom.area;
    }*/
    if (vs.waku.front().size() != atom.waku.front().size()) {
      return vs.waku.front().size() < atom.waku.front().size();
    }
    // 角度の分散が小さい方を優先
    //const auto ta = cal_togari(atom);
    //const auto tb = cal_togari(vs);
    //if (ta != tb) {
    //  return tb < ta;
    //}
    return false;
  }
};


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

const double EPS = 1e-6;
static double min_deg = 2 * PI;

class PathCompare {
public:
  bool operator()(const cl::Path &left, const cl::Path &right) const {
    if (left.size() != right.size()) {
      return left.size() < right.size();
    }
    const int n = left.size();
    for (int i = 0; i < n; ++i) {
      if (left[i].X != right[i].X) {
        return left[i].X < right[i].X;
      }
      if (left[i].Y != right[i].Y) {
        return left[i].Y < right[i].Y;
      }
    }
    return false;
  }
};

std::map<cl::Path, std::vector<double>, PathCompare> memo_degree;
std::map<cl::Path, std::set<std::vector<std::pair<cl::cInt, cl::cInt>>>, PathCompare> memo_line;

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

  auto gui = std::vector< cl::Path > ({path});
  DrawPolygons(gui, 0x20FFFF00, 0x30FF0000);

  if (next_waku.size() > 1) {
    std::sort(next_waku.begin(), next_waku.end(), [](const auto& l, const auto& r) {
      return std::abs(cl::Area(l)) > std::abs(cl::Area(r));
    });

    /*for( auto &p : next_waku ) {
      cl::Paths printed = std::vector< cl::Path >({p});
      DrawPolygons(printed, 0x20FFFF00, 0x30FF0000);
    }*/

    // 誤差を考え、面積x以下は許容する。
    dame |= std::abs(cl::Area(next_waku[1])) > EPS;
    next_waku.erase(next_waku.begin() + 1, next_waku.end());
  }

  cl::Clipper intersecter;
  intersecter.AddPaths(waku, cl::ptSubject, true);
  intersecter.AddPath(path, cl::ptClip, true);
  cl::Paths dst;
  intersecter.Execute(cl::ctIntersection, dst, cl::pftNonZero, cl::pftNonZero);
  double trg = 0;
  for (auto p : dst) {
    trg += cl::Area(p);
  }
  double area = cl::Area(path);
  if (fabs(area - trg) > 1e-3) {
    dame |= true;
  }

  bool flag = 0;
  int count = 0;
  if(!dame) {
    if (waku.size()) {
      std::map< std::pair< int, int >, double > degs;
      bool ori = cl::Orientation(waku.front());
      auto v = waku.front();
      int n = v.size();
      for (int i = 0; i < v.size(); i++) {
        degs[std::make_pair(v[i].X, v[i].Y)] = degree(v[(i - 1 + n) % n], v[i], v[(i + 1) % n], ori);
      }
      if (!memo_degree.count(path)) {
        std::vector<double> memo;
        int m = path.size();
        ori = cl::Orientation(path);
        for (int i = 0; i < m; i++) {
          double deg = degree(path[(i - 1 + m) % m], path[i], path[(i + 1) % m], ori);
          memo.push_back(deg);
        }
        memo_degree[path] = memo;
      }
      int m = path.size();
      for (int i = 0; i < m; i++) {
        auto p = std::make_pair(path[i].X, path[i].Y);
        if (degs.count(p)) {
          auto deg = memo_degree[path][i];
          if (fabs(deg - degs[p]) < 1e-3) {
            //count += 1;
          }
        }
      }
    }

    // ぴったりハマるところがあればボーナスとしてカウン
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
        /*  if(flag) count += 10;
          else count -= 100;*/
          count += 2;
        }
      }
    }

    // ピースの最小角よりも小さな角ができたらdameフラグを立てる
    const auto& _path = next_waku.front();
    const int n = _path.size();
    auto orientation = cl::Orientation(_path);
    for (int i = 0; i < n; ++i) {
      auto prev = _path[(i - 1 + n) % n];
      auto vert = _path[i];
      auto next = _path[(i + 1) % n];
      auto deg = degree(prev, vert, next, orientation);
      if (deg < min_deg) {
        dame = true;
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
    // 誤差を考え、面積x以下は許容する。
    dame |= std::abs(cl::Area(next_uni[1])) > EPS;
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
    // 誤差を考え、面積x以下は許容する。
    dame |= std::abs(cl::Area(v)) > EPS;
  }
  return std::tuple<bool, int, cl::Paths>(dame, 0, next_uni);
}



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
  cv::Mat img = cv::Mat::zeros(cv::Size(1200, 900), CV_8UC4);
  std::vector<std::vector<cv::Point>> paths;
  std::vector<int> npts;

  img = uint2scalar(0x00FFFFFF);

  for (const auto& path : _paths) {
    std::vector<cv::Point> points;
    for (const auto& point : path) {
      points.push_back(cv::Point(point.X * 7, point.Y * 7));
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

static void DrawPolygons2(const cl::Paths& _paths, bool fill = false)
{
  if (_paths.size() == 0) {
    std::cerr << "nothing to draw" << std::endl;
  }
  cv::Mat img = cv::Mat::zeros(cv::Size(1500, 1200), CV_8UC4);
  img = uint2scalar(0x00FFFFFF);
  srand((unsigned)time(NULL));
  for (const auto& _path : _paths) {
    const cl::Paths one = { _path };
    std::vector<std::vector<cv::Point>> paths;
    std::vector<int> npts;

    for (const auto& path : one) {
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

    if (fill)
      cv::fillPoly(img, (const cv::Point **) &raw_paths[0], &npts[0], paths.size(), uint2scalar(rand()));
    cv::polylines(img, (const cv::Point **) &raw_paths[0], &npts[0], paths.size(), true, uint2scalar(rand()));
  }

  cv::imshow("clipper sample", img);
  cv::imwrite("drawn.bmp", img);

  cv::waitKey(0);
  cv::destroyAllWindows();
}

void draw(const std::vector<im::Piece>& problem, const Info& info)
{
  /*----- output answer by GUI -----*/
  cl::Paths gui;
  for (int i = 0; i < problem.size(); i++) {
    cl::Path ps;
    //for(auto p: problem) {

    if (info.indexes[i] < 0) continue;
    for (auto v : problem[i].vertexes[info.indexes[i]]) {
      v.x += info.haiti[i].x;
      v.y += info.haiti[i].y;
      ps.push_back(cl::IntPoint(v.x * 6, v.y * 6));
    }
    //}
    gui.push_back(ps);

  }
  DrawPolygons2(gui, true);
}

static void save(const std::string& name, const cl::Paths& _paths, unsigned int fill_color, unsigned int line_color)
{
  if (_paths.size() == 0) {
    std::cerr << "nothing to draw" << std::endl;
  }
  cv::Mat img = cv::Mat::zeros(cv::Size(1200, 900), CV_8UC4);
  std::vector<std::vector<cv::Point>> paths;
  std::vector<int> npts;

  img = uint2scalar(0x00FFFFFF);

  for (const auto& path : _paths) {
    std::vector<cv::Point> points;
    for (const auto& point : path) {
      points.push_back(cv::Point(point.X * 7, point.Y * 7));
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

  cv::imwrite(name, img);
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

std::vector<im::Answer> tk::matsuri_search(const im::Piece& waku, const std::vector<im::Piece>& problem, const std::vector<im::Hint>& hint)
{
  const int n = problem.size();
  std::vector<std::multiset<State, MatsuriCompare>> stacks(n + 1);
  State atom(piece2paths(waku));
  stacks[0].insert(atom);
  std::vector<cl::Paths> paths;
  for (const auto& piece : problem) {
    paths.push_back(piece2paths(piece));
  }
  for (const auto &path : paths) {
    const cl::Path& one = path.front();
    auto orientation = cl::Orientation(one);
    const int m = one.size();
    for (int i = 0; i < m; ++i) {
      auto prev = one[(i - 1 + m) % m];
      auto vert = one[i];
      auto next = one[(i + 1) % m];
      auto deg = degree(prev, vert, next, orientation);
      min_deg = std::min(min_deg, deg);
    }
  }
  std::cerr << "min_deg : " << min_deg << std::endl;
  //std::cerr << "paths size : " << paths.size() << std::endl;
  //int num = 0;
  //for (const auto& path : paths) {
  //  int cnt = 0;
  //  for (const auto& aaa : path) {
  //    save("info/" + std::to_string(num) + "-" + std::to_string(cnt) + ".bmp",{ aaa }, 114514, 8931919);
  //    ++cnt;
  //  }
  //  ++num;
  //}
  double best_score = 1 << 28;
  Info best_info;
  cl::Paths best_waku;
  cl::Paths best_uni;
  int bornus;
  int count = 0;
  std::set<std::vector<im::Point>, PointVecCompare> done;
  done.insert(atom.info.haiti);
  bool end = false;
  for (int g = 0;; ++g) {
    std::cerr << "---- " << g << " GENERATION ----" << std::endl;
    bornus = -1;
    for (int i = 0; i <= n; ++i) {
      if (stacks[i].size() == 0) {
        continue;
      }
      State node = *stacks[i].rbegin();
      if (node.bornus == bornus) {
        break;
      }
      bornus = node.bornus;
      const int uni_size = node.uni.size() == 0 ? 0 : node.uni.front().size();
      const double waku_area = node.waku.size() == 0 ? 0 : std::abs(cl::Area(node.waku.front()));
      std::cerr << "[" << i << "] : " << node.bornus << ", " << node.edges << " - " << uni_size
        << " = " << node.edges - uni_size << " ... " << waku_area << std::endl;
      double score = node.waku.size() == 0 ? 0 : std::abs(cl::Area(node.waku.front()));
      if (best_score > score) {
        best_score = score;
        best_info = node.info;
        best_waku = node.waku;
        best_uni = node.uni;
        ++count;
        std::cerr << "(" << count << ") best score is renewed! : " << best_score << " at " << i << std::endl;
        ///if (count % 10 == 0 || i == n) {
          DrawPolygons(node.waku, 0x160000FF, 0x600000FF); //blue
          //DrawPolygons(node.uni, 0x20FFFF00, 0x30FF0000); //orange
        //}
        if (score < 1e-3) {
          end = true;
        }
      }
      stacks[i].erase(std::prev(stacks[i].end()));
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
        const int m = paths[j].size();
        for (int k = 0; k < m; ++k) {
          const auto& path = paths[j][k];
          std::set<std::pair<int, int>> targets;
          for (const auto& wp : node.waku.front()) {
            for (const auto &pp : path) {
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
            // check reserved area
            bool nonon = false;
            for (const im::Hint& h : hint) {
              cl::Path path;
              for (const auto& p : h.vertexes) {
                path << cl::IntPoint(p.x, p.y);
              }
              cl::Clipper clipper;
              clipper.AddPaths(next_uni, cl::ptSubject, true);
              clipper.AddPath(path, cl::ptClip, true);
              cl::Paths dst;
              clipper.Execute(cl::ctIntersection, dst, cl::pftNonZero, cl::pftNonZero);
              if (dst.size()) {
                double area = 0;
                for (const cl::Path& l : dst) {
                  area += cl::Area(l);
                }
                nonon |= area > 3;
              }
            }
            if (nonon) {
              continue;
            }
            auto next_haiti = node.info.haiti;
            next_haiti[j] = im::Point(x, y);
            auto next_indexes = node.info.indexes;
            next_indexes[j] = k;
            Info next_info(next_set, next_haiti, next_indexes);
            double area = 0;
            if (next_waku.size()) {
              cl::Area(next_waku.front());
            }
            State next(next_waku, next_uni, next_info, node.bornus + bornus, node.edges + path.size(), area);
            if (done.count(next_haiti) > 0) {
              continue;
            }
            done.insert(next_haiti);
            stacks[i + 1].insert(next);
            if (stacks[i + 1].size() > 100) {
              stacks[i + 1].erase(stacks[i + 1].begin());
            }
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
        }
        //std::cerr << "\t\t" << awawa << " transed to next" << std::endl;
      }
      // try put hint
      for (const im::Hint& h : hint) {
        cl::Path path;
        for (const auto& p : h.vertexes) {
          path << cl::IntPoint(p.x, p.y);
        }
        bool dame = false;
        auto ret_waku = cut_waku(node.waku, path);
        dame |= std::get<0>(ret_waku);
        auto ret_uni = patch_uni(node.uni, path);
        dame |= std::get<0>(ret_uni);
        if (dame) {
          continue;
        }
        auto bornus = std::get<1>(ret_waku) + std::get<1>(ret_uni);
        auto next_waku = std::get<2>(ret_waku);
        auto next_uni = std::get<2>(ret_uni);
        auto next_haiti = node.info.haiti;
        auto next_indexes = node.info.indexes;
        Info next_info = node.info;
        State next(next_waku, next_uni, next_info, node.bornus + bornus, node.edges + path.size());
        stacks[i + 1].insert(next);
        if (stacks[i + 1].size() > 100) {
          stacks[i + 1].erase(stacks[i + 1].begin());
        }
      }
    }
    if (end) {
      break;
    }
    //DrawPolygons(best_waku, 0x160000FF, 0x600000FF);
    //DrawPolygons(best_uni, 0x20FFFF00, 0x30FF0000);

    /*const auto& _path = best_waku.front();
    const int n = _path.size();
    auto orientation = cl::Orientation(_path);
    std::cerr << "====================" << std::endl;
    for (int i = 0; i < n; ++i) {
      auto prev = _path[(i - 1 + n) % n];
      auto vert = _path[i];
      auto next = _path[(i + 1) % n];
      auto deg = degree(prev, vert, next, orientation);
      std::cerr << (deg * 180 / PI) << std::endl;
      if (deg < min_deg) {
        std::cerr << "herererere" << std::endl;
      }
    } */
  }
  for (int i = 0; i < n; ++i) {
    std::cerr << "(" << i << " - " << best_info.indexes[i] << ") [" << (best_info.set[i] ? "x" : " ") << "] : " << best_info.haiti[i].x << " " << best_info.haiti[i].y << std::endl;
  }

  std::vector<im::Answer> ans;
  for (int i = 0; i < n; ++i) {
    ans.push_back(im::Answer(problem[i].id, best_info.indexes[i], best_info.haiti[i]));
  }
  return ans;
}
