#include "tokugawa.hpp"
#include "polyclipping/clipper.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <vector>
#include <iostream>
#include <fstream>

using namespace ClipperLib;

namespace cl = ClipperLib;

union Color {
  unsigned int raw;
  unsigned char bytes[4];
};

cv::Scalar uint2scalar(unsigned int _color)
{
  Color color = Color{_color};
  std::cerr << (int)color.bytes[0] << " " << (int)color.bytes[1] << " " << (int)color.bytes[2] << " " << (int)color.bytes[3] << std::endl;
  return cv::Scalar(color.bytes[0], color.bytes[1], color.bytes[2], color.bytes[3]);
}
static void DrawPolygons(const cl::Paths& _paths)
{
  if (_paths.size() == 0) {
    std::cerr << "nothing to draw" << std::endl;
  }
  cv::Mat img = cv::Mat::zeros(cv::Size(1500, 1200), CV_8UC4);
  img = uint2scalar(0x00FFFFFF);
  srand((unsigned)time(NULL));
  for (const auto& _path : _paths) {
    const cl::Paths one = {_path};
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

//    cv::fillPoly(img, (const cv::Point **) &raw_paths[0], &npts[0], paths.size(), uint2scalar(rand()));
    cv::polylines(img, (const cv::Point **) &raw_paths[0], &npts[0], paths.size(), true, uint2scalar(rand()));
  }

  cv::imshow("clipper sample", img);

  cv::waitKey(0);
  cv::destroyAllWindows();
}

void testclipper()
{
	Paths subj(2), clip(1), solution;

	//define outer blue 'subject' polygon
	subj[0] <<
	  IntPoint(180,200) << IntPoint(260,200) <<
	  IntPoint(260,150) << IntPoint(180,150);

	//define subject's inner triangular 'hole' (with reverse orientation)
	subj[1] <<
	  IntPoint(215,160) << IntPoint(230,190) << IntPoint(200,190);

	//define orange 'clipping' polygon
	clip[0] <<
	  IntPoint(190,210) << IntPoint(240,210) <<
	  IntPoint(240,130) << IntPoint(190,130);

	//draw input polygons with user-defined routine ...
	//DrawPolygons(subj, 0x160000FF, 0x600000FF); //blue
	//DrawPolygons(clip, 0x20FFFF00, 0x30FF0000); //orange

	//perform intersection ...
	Clipper c;
	c.AddPaths(subj, ptSubject, true);
	c.AddPaths(clip, ptClip, true);
	c.Execute(ctIntersection, solution, pftNonZero, pftNonZero);

	//draw solution with user-defined routine ...
	//DrawPolygons(solution, 0x3000FF00, 0xFF006600); //solution shaded green
}

void gen(std::vector<im::Piece> problem)
{
  Clipper c;
  for (const auto& p : problem) {
    Path path;
    for (const auto& v : p.vertexes.front()) {
      path << IntPoint(v.x, v.y);
    }
    c.AddPath(path, ptClip, true);
  }
  Paths sol;
  c.Execute(ctUnion, sol, pftNonZero, pftNonZero);

  std::cerr << "Pathss size : " << sol.size() << std::endl;
  for (const auto& path : sol) {
    std::cerr << "\tpath size : " << path.size() << std::endl;
    for (const auto& point : path) {
      std::cerr << "\t\t" << point.X << ", " << point.Y << std::endl;
    }
  }

  DrawPolygons(sol);
}

int main()
{
  //tk::tk_hello();

  //testclipper();

  std::vector<im::Piece> problem;

  // test matsuri search
  std::string path;
  std::cin >> path;
  std::ifstream ifs(path);

  im::Piece waku;
  {
    int n;
    ifs >> n;
    std::vector<im::Point> points;
    for (int i = 0; i < n; ++i) {
      int x, y;
      ifs >> x;
      ifs >> y;
      points.push_back(im::Point(x, y));
    }
    waku = im::Piece(-1, { points });
  }
  for (int id = 0; !ifs.eof(); ++id) {
    int n;
    ifs >> n;
    std::vector<im::Point> points;
    for (int i = 0; i < n; ++i) {
      int x, y;
      ifs >> x;
      ifs >> y;
      points.push_back(im::Point(x, y));
    }
    im::Piece piece(id, { points });
    problem.push_back(piece);
  }

  std::vector<im::Point> offsets;
  for (auto& piece : problem) {
    int minx = 1 << 28;
    int miny = 1 << 28;
    for (const auto& point : piece.vertexes.front()) {
      minx = std::min(minx, point.x);
      miny = std::min(miny, point.y);
    }
    offsets.push_back(im::Point(minx, miny));
    for (auto& point : piece.vertexes.front()) {
      point.x -= minx;
      point.y -= miny;
    }
  }

  //gen(problem);

  auto ans = tk::search(waku, problem, std::vector<im::Hint>(), 0);

  std::cerr << "==============" << std::endl;
  for (int i = 0; i < offsets.size(); ++i) {
    std::cerr << "(" << i << ") : " << offsets[i].x << " " << offsets[i].y << std::endl;
  }

  cl::Paths gui;
  for (int i = 0; i < ans.size(); i++) {
    cl::Path ps;
    //for(auto p: problem) {

    if (ans[i].index < 0) continue;
    for (auto v : problem[i].vertexes[ans[i].index]) {
      v.x += ans[i].point.x;
      v.y += ans[i].point.y;
      ps.push_back(cl::IntPoint(v.x * 6, v.y * 6));
    }
    //}
    gui.push_back(ps);

  }
  DrawPolygons(gui);

  return 0;
}
