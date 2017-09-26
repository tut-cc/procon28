#include "tokugawa.hpp"
#include "polyclipping/clipper.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <vector>
#include <iostream>

using namespace ClipperLib;

union Color {
  unsigned int raw;
  unsigned char bytes[4];
};

cv::Scalar uint2scalar(unsigned int _color)
{
  Color color = Color{_color};
  std::cerr << (int)color.bytes[0] << " " << (int)color.bytes[1] << " " << (int)color.bytes[2] << " " << (int)color.bytes[3];
  return cv::Scalar(color.bytes[0], color.bytes[1], color.bytes[2], color.bytes[3]);
}

void DrawPolygons(const Paths& _paths, unsigned int fill_color, unsigned int line_color)
{
  cv::Mat img = cv::Mat::zeros(cv::Size(300, 300), CV_8UC4);
  std::vector<std::vector<cv::Point>> paths;
  std::vector<int> npts;
  
  img = uint2scalar(0x00FFFFFF);
  
  for (const Path& path : _paths) {
    std::vector<cv::Point> points;
    for (const IntPoint point : path) {
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
	DrawPolygons(subj, 0x160000FF, 0x600000FF); //blue
	DrawPolygons(clip, 0x20FFFF00, 0x30FF0000); //orange

	//perform intersection ...
	Clipper c;
	c.AddPaths(subj, ptSubject, true);
	c.AddPaths(clip, ptClip, true);
	c.Execute(ctIntersection, solution, pftNonZero, pftNonZero);

	//draw solution with user-defined routine ... 
	DrawPolygons(solution, 0x3000FF00, 0xFF006600); //solution shaded green
}

int main()
{
  tk::tk_hello();

  testclipper();

  std::vector<std::vector<im::Piece>> problem;

  return 0;
}
