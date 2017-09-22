#include "tokugawa.hpp"
#include "polyclipping/clipper.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <vector>

using namespace ClipperLib;

void DrawPolygons(const Paths& paths, unsigned int fill_color, unsigned int line_color)
{
  cv::Mat img;
  std::vector<cv::Point> points;
  for (const Path& p : paths) {
    
  }
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
