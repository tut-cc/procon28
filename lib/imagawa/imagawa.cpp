#include "imagawa.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>

#define PI 3.14159265
#define DP 180/PI
#define derror 0.5


im::Point::Point() : Point(0, 0) {}

im::Point::Point(int x, int y) : x(x), y(y) {}

im::Pointd::Pointd() : Pointd(0, 0) {}

im::Pointd::Pointd(double x, double y) : x(x), y(y) {}

im::Piece::Piece() : Piece(0, {}) {}

/*
im::Piece::Piece(int id, const std::vector<im::Point> &vertexes, const std::vector<int> &edges2,
  const std::vector<double> &degs) : id(id), vertexes(vertexes), edges2(edges2), degs(degs) {}
*/
im::Piece::Piece(int id, const std::vector<std::vector<im::Point>> &vertexes) 
	: id(id), vertexes(vertexes) {}

im::Answer::Answer() : Answer(0, {}) {}

im::Answer::Answer(int id, const std::vector<im::Point> &vertexes) : id(id), vertexes(vertexes) {}

void im::hello() {
  std::cout << "hello 今川" << std::endl;
}

std::vector<cv::Mat> im::devideImg(const cv::Mat &binaryImg) {

  // ラべリング
  cv::Mat labels, stats, centroids;
  auto labelNum = cv::connectedComponentsWithStats(binaryImg, labels, stats, centroids);

  std::vector<cv::Mat> pieceImgs;
  for (auto i = 1; i < labelNum; i++) {
    auto *stat = stats.ptr<int>(i);

    // サイズは少し大きめにとる
    auto l = std::max(stat[cv::ConnectedComponentsTypes::CC_STAT_LEFT] - 5, 0);
    auto t = std::max(stat[cv::ConnectedComponentsTypes::CC_STAT_TOP] - 5, 0);
    auto w = std::min(stat[cv::ConnectedComponentsTypes::CC_STAT_WIDTH] + 10, binaryImg.cols - l);
    auto h = std::min(stat[cv::ConnectedComponentsTypes::CC_STAT_HEIGHT] + 10, binaryImg.rows - t);
    auto s = stat[cv::ConnectedComponentsTypes::CC_STAT_AREA];

    // 小さすぎるクラスタはノイズと判断して無視
    if (s < 100) {
      continue;
    }

    // ピース切り出し
    auto pieceImg = binaryImg(cv::Rect(l, t, w, h)).clone();
    for (auto y = 0; y < h; y++) {
      auto *labelsRow = labels.ptr<int>(t + y);
      for (auto x = 0; x < w; x++) {

        // 入り込んだ別ピースは黒で塗りつぶす
        if (labelsRow[l + x] != i) {
          pieceImg.data[w * y + x] = 0;
        }
      }
    }

    pieceImgs.push_back(pieceImg);
  }

  return pieceImgs;
}

static int dist2(int x1, int y1, int x2, int y2) {
  return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

static bool compareSegments(const cv::Vec4i &segment1, const cv::Vec4i &segment2) {
  return segment1[0] < segment2[0];
}

static double dot(double v1[], double v2[]) {
  return v1[0] * v2[0] + v1[1] * v2[1];
}

static double abs2(double v[]) {
  return dot(v, v);
}

static double dot(const cv::Vec4i &v1, const cv::Vec4i &v2) {
  double u1[] = { v1[2] - v1[0], v1[3] - v1[1] };
  double u2[] = { v2[2] - v2[0], v2[3] - v2[1] };
  return dot(u1, u2) / (std::sqrt(abs2(u1)) * std::sqrt(abs2(u2)));
}

// 重なっている線分のみマージして返す(途切れた線分のマージは行わない)
std::vector<cv::Vec4i> im::detectSegments(const cv::Mat &edgeImg) {

  // 確率的ハフ変換による線分検出
  std::vector<cv::Vec4i> segs;
  cv::HoughLinesP(edgeImg, segs, 1, CV_PI / 180, 45, 30, 30);

  // 左端点のx座標が昇順になるようソート
  std::sort(segs.begin(), segs.end(), compareSegments);

  std::vector<cv::Vec4i> segments;
  for (auto i = 0; i < segs.size(); i++) {

    // 短い線分を無視
    if (dist2(segs[i][0], segs[i][1], segs[i][2], segs[i][3]) < 400) {
      continue;
    }

    for (auto j = i + 1; j < segs.size(); j++) {

      // 短い線分を無視
      if (dist2(segs[j][0], segs[j][1], segs[j][2], segs[j][3]) < 400) {
        continue;
      }

      // 傾きに差があればマージしない
      if (std::abs(dot(segs[i], segs[j])) < 0.9995) {
        continue;
      }

      // 線分2の左端点と線分1の距離(https://tgws.plus/ul/ul31.html)
      double p1q1[] = { segs[j][0] - segs[i][0], segs[j][1] - segs[i][1] };
      double p1p2[] = { segs[i][2] - segs[i][0], segs[i][3] - segs[i][1] };
      double p2q1[] = { segs[j][0] - segs[i][2], segs[j][1] - segs[i][3] };
      auto dot1 = dot(p1q1, p1p2);
      auto dd =
        dot1 < 0.0 ? abs2(p1q1) :
        dot1 > abs2(p1p2) ? abs2(p2q1) :
        abs2(p1q1) - dot1 * dot1 / abs2(p1p2);

      // 距離が大きければマージしない
      if (dd > 400.0) {
        continue;
      }

      // 右端点座標を更新
      if (segs[j][2] > segs[i][2]) {
        segs[i][2] = segs[j][2];
        segs[i][3] = segs[j][3];
      }

      // 取り込まれた線分の長さを0にする(フラグ代わり)
      segs[j][2] = segs[j][0];
      segs[j][3] = segs[j][1];
    }

    segments.push_back(segs[i]);
  }

  return segments;
}

// 2線分を延長したときの交点
static im::Inter intersection(const cv::Vec4i &v1, const cv::Vec4i &v2) {

  // 傾きが等しいときは大きい値を返す
  if (std::abs(dot(v1, v2)) >= 0.9995) {
    return im::Inter();
  }

  auto a1 = (v1[3] - v1[1]) / (v1[2] - v1[0] != 0 ? v1[2] - v1[0] : 1.0e-9);
  auto b1 = v1[1] - a1 * v1[0];
  auto a2 = (v2[3] - v2[1]) / (v2[2] - v2[0] != 0 ? v2[2] - v2[0] : 1.0e-9);
  auto b2 = v2[1] - a2 * v2[0];

  im::Inter inter;
  inter.p.x = -(b1 - b2) / (a1 - a2);
  inter.p.y = a1 * inter.p.x + b1;

  return inter;
}

im::Inter::Inter() : p(1.0e9, 1.0e9), d2(1.0e9), lr(0), f(false) {}

static double dist2(double x1, double y1, double x2, double y2) {
  return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

static double dist2(const cv::Vec4i &v, const im::Pointd &p, int lr) {
  return dist2((double)v[lr * 2 - 2], (double)v[lr * 2 - 1], p.x, p.y);
}

static int nearLR(const cv::Vec4i &v, const im::Pointd &p) {
  return dist2(v, p, 1) < dist2(v, p, 2) ? 1 : 2;
}

std::vector<im::Pointd> im::detectVertexes(const std::vector<cv::Vec4i> &segments) {

  // 頂点候補リスト
  std::vector<std::vector<Inter>> inters(segments.size());
  for (auto &inter : inters) {
    inter.resize(segments.size());
  }
  for (auto i = 0; i < segments.size(); i++) {
    for (auto j = i + 1; j < segments.size(); j++) {
      inters[i][j] = inters[j][i] = intersection(segments[i], segments[j]);
    }
  }

  // 頂点候補と線分端点の距離
  for (auto i = 0; i < segments.size(); i++) {
    for (auto j = i + 1; j < segments.size(); j++) {
      inters[i][j].lr = nearLR(segments[i], inters[i][j].p);
      inters[j][i].lr = nearLR(segments[j], inters[j][i].p);
      inters[i][j].d2 = inters[j][i].d2 =
        dist2(segments[i], inters[i][j].p, inters[i][j].lr) +
        dist2(segments[j], inters[j][i].p, inters[j][i].lr);
    }
  }

  // 開始頂点の決定
  auto startI = -1, startJ = -1;
  auto minD2 = 1.0e9;
  for (auto i = 0; i < segments.size(); i++) {
    for (auto j = i + 1; j < segments.size(); j++) {
      if (inters[i][j].d2 < minD2) {
        startI = i;
        startJ = j;
        minD2 = inters[i][j].d2;
      }
    }
  }
  inters[startI][startJ].f = inters[startJ][startI].f = true;

  std::vector<im::Pointd> vertexes;
  vertexes.push_back(inters[startI][startJ].p);

  auto i = startI;
  auto lr = inters[startI][startJ].lr;
  while (i != startJ) {
    auto minJ = -1;
    minD2 = 1.0e9;
    for (auto j = 0; j < inters.size(); j++) {
      if (inters[i][j].f || inters[i][j].lr == lr) {
        continue;
      }

      if (inters[i][j].d2 < minD2) {
        minJ = j;
        minD2 = inters[i][j].d2;
      }
    }

    if (minJ == -1) {
      std::cout << "NG" << std::endl;
      return vertexes;
    }

    vertexes.push_back(inters[i][minJ].p);
    inters[i][minJ].f = inters[minJ][i].f = true;
    lr = inters[minJ][i].lr;
    i = minJ;
  }

  std::cout << "OK" << std::endl;

  // 反時計回りだった場合時計回りに修正
  auto li = 0;
  auto lp = im::Pointd(1.0e9, 1.0e9);
  for (auto i = 0; i < vertexes.size(); i++) {
    if (vertexes[i].x < lp.x) {
      li = i;
      lp = vertexes[i];
    }
  }

  auto bef = vertexes[li > 0 ? li - 1 : vertexes.size() - 1];
  auto aft = vertexes[li < vertexes.size() - 1 ? li + 1 : 0];
  im::Pointd va(lp.x - bef.x, lp.y - bef.y);
  im::Pointd vb(aft.x - lp.x, aft.y - lp.y);
  if (va.x * vb.y - va.y * vb.x < 0) {
    std::reverse(vertexes.begin(), vertexes.end());
  }

  return vertexes;
}

/*
######################################################################
information about shape
N:na xa1 ya1 xa2 ya2 ... xana yana:nb xb1 yb1 xb2 yb2 ...xbna ybna:...
######################################################################
*/

//For the time being, coordinates is written by [mm]

//rollの戻り値確認
im::Piece im::roll(int id, std::vector<im::Pointd> shape) {
	im::Piece piece;
  //Transfer [pix -> mm]
  double ratio = 55 / (512 * 2.5); //[(mm/pix/mm)]
  //double ratio = 55 / (512 * 1.0); //[(mm/pix/mm)]
  //double ratio = 1.0; //[(mm/pix/mm)]
  std::cout << "-----" << std::endl;
  for (im::Pointd &xy : shape) {
    xy.x *= ratio;
    xy.y *= ratio;
    //std::cout << xy.x << "," << xy.y << std::endl;
  }

  int len_corn = shape.size();
  std::vector<double> len_side(len_corn, 0);
  std::vector<im::Point> tmp_res(len_corn, im::Point(0, 0));
  std::vector<std::vector<im::Point>> result;

	//WARNING!
  for (int corn = 0; corn < len_corn; corn++) {
    double dx = shape[corn].x
      - shape[(corn < len_corn - 1) ? corn + 1 : 0].x;
    double dy = shape[corn].y
      - shape[(corn < len_corn - 1) ? corn + 1 : 0].y;
		std::cout << "dx:" << dx << std::endl;
    len_side[corn] = sqrt(dx*dx + dy*dy);
  }
	/*
	for(int corn = 0; corn < segments.size(); corn++){
		cv::Vec4i side = segments[corn];
		double dx = side[2]-side[0];
		double dy = side[3]-side[1];
		len_side[corn] = sqrt(dx*dx+dy*dy)*ratio;
	}
	*/
	

  /*
  >>terms to stop<<
  1. Theta > 90
  2. dx > len_side[0]

  >>terms to break<<
  1. A point isn't at any grids.
  */

	//std::cout << "dy0:" << shape[0].y << "," << shape[1].y << std::endl;
  double dy0 = (shape[1].y - shape[0].y);
	//std::cout << "dy0:" << dy0 << std::endl;
  double theta0 = 0;
	//std::cout << "len_side[0]:" << len_side[0] << std::endl;
  if (dy0 != 0) {
		if(std::abs(dy0) <= len_side[0])
    	theta0 = acos(dy0 / len_side[0]);
		else if(dy0 > 0)
			theta0 = acos(1);
		else if(dy0 < 0)
			theta0 = acos(-1);
		//std::cout << "theta0:" << theta0 << std::endl;
    //cout << len_side[0] << endl;
  }
  else {
    theta0 = 0;
  }
  double theta = 0, dtheta = 0;
  for (double dy = (int)len_side[0]; dy <= len_side[0] && theta <= PI / 4; dy--) {
    //(dx!=0) ? theta = asin(dx/len_side[0]) : theta = 0;
    theta = acos(dy / len_side[0]);
		//std::cout << "theta1:" << theta << std::endl;
    dtheta = theta0 - theta;
		//std::cout << "theta2:" << theta << std::endl;
    //cout << theta0*DP << ":" << theta*DP << ":" << dtheta*DP << endl;
		int minX = 1.0e9;
		int minY = 1.0e9;
    for (int corn = 0; corn < len_corn; corn++) {
      //Rotation matrix
      double x =
        (shape[corn].x - shape[0].x)*cos(dtheta) -
        (shape[corn].y - shape[0].y)*sin(dtheta);
      double y =
        (shape[corn].x - shape[0].x)*sin(dtheta) +
        (shape[corn].y - shape[0].y)*cos(dtheta);
			//std::cout << "x1:" << x << std::endl;
      bool flagx = false;
      bool flagy = false;
			//std::cout << x << "," << y << std::endl;
      if (std::abs(floor(x) - x) < derror) { x = floor(x); flagx = true; }
      if (std::abs(floor(y) - y) < derror) { y = floor(y); flagy = true; }
      if (std::abs(ceil(x) - x) < derror) { x = ceil(x); flagx = true; }
      if (std::abs(ceil(y) - y) < derror) { y = ceil(y); flagy = true; }
			//std::cout << "x2:" << x << std::endl;
      if (flagx && flagy){
				if(x < minX) minX = x;
				if(y < minY) minY = y;
				tmp_res[corn] = im::Point(x, y);
				//std::cout << "res:" << x << "," << y << std::endl;
			}
      else break;

      if (corn == len_corn - 1) {
				for(auto &xy:tmp_res){
					if(minX<0) xy.x -= minX;
					if(minY<0) xy.y -= minY;
				}
        result.push_back(tmp_res);
				//回転追加
				for(int i=0;i<3;i++){
					int minX = 1.0e9;
					int minY = 1.0e9;
					for(auto &xy:tmp_res){
						xy.x = xy.x*cos(PI/2)-xy.y*sin(PI/2);
						xy.y = xy.x*sin(PI/2)+xy.y*cos(PI/2);
						if(xy.x < minX) minX = xy.x;
						if(xy.y < minY) minY = xy.y;
					}
					for(auto &xy:tmp_res){
						if(minX<0) xy.x -= minX;
						if(minY<0) xy.y -= minY;
					}
					result.push_back(tmp_res);
				}
      }
    }
  }
	piece.id = id;
	piece.vertexes = result;

  return piece;
}
