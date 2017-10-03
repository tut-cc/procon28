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
#define derror 0.00001


im::Point::Point() : Point(0, 0) {}

im::Point::Point(int x, int y) : x(x), y(y) {}

im::Pointd::Pointd() : Pointd(0, 0) {}

im::Pointd::Pointd(double x, double y) : x(x), y(y) {}

im::Piece::Piece() : Piece(0, {}, {}, {}) {}

im::Piece::Piece(int id, const std::vector<im::Point> &vertexes, const std::vector<int> &edges2,
  const std::vector<double> &degs) : id(id), vertexes(vertexes), edges2(edges2), degs(degs) {}

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

  //std::cout << "kaishi" << std::endl;
  std::vector<cv::Vec4i> segments;
  for (auto i = 0; i < segs.size(); i++) {
    //std::cout << segs[i][0] << ',' << segs[i][1] << ',' << segs[i][2] << ',' << segs[i][3] << std::endl;

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
      if (std::abs(dot(segs[i], segs[j])) < 0.9) {
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
        abs2(p1q1) - dot1 / abs2(p1p2);

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
  //std::cout << "owari" << std::endl;

  //std::cout << "hajime" << std::endl;
  //for (auto &seg : segments) {
  //  std::cout << seg[0] << ',' << seg[1] << ',' << seg[2] << ',' << seg[3] << std::endl;
  //}
  //std::cout << "end" << std::endl;
  return segments;
}

im::Pointd getpoint(cv::Vec4i v1_o, cv::Vec4i v2_o) {
  //2つの辺の交点を求める
  /*
  0:start x
  1:start y
  2:end   x
  3:end   y
   */

  const int SLO = 10; //
  std::vector<double> v1(4);
  std::vector<double> v2(4);
  for (int i = 0; i < 4; i++) {
    v1[i] = v1_o[i];
    v2[i] = v2_o[i];
  }
  if (v1[2] - v1[0] == 0) v1[2] += 0.001; //傾き無限大対策(応急処置)
  double a = (v1[3] - v1[1]) / (v1[2] - v1[0]);
  double b = v1[1] - a* v1[0];
  if (v2[2] - v2[0] == 0) v2[2] += 0.001; //傾き無限大対策(応急処置)
  double c = (v2[3] - v2[1]) / (v2[2] - v2[0]);
  double d = v2[1] - c* v2[0];
  //if(a == c) c+=1; //応急処置

    //傾きが一緒ならば省略
  if ((a - c)*(a - c) < SLO) return im::Pointd(-1, -1);
  double x = (d - b) / (a - c);
  double y = a* x + b;

  return im::Pointd(x, y);
}

double getdis(cv::Vec4i v1, cv::Vec4i v2, im::Pointd vp) {
  //各線分のいずれかの端っこからの距離を測定

  /*int midy1 = abs(v1[1]-v1[3])/2;
  int midx1 = abs(v1[0]-v1[2])/2;*/
  double midy1 = (double)v1[1];
  double midx1 = (double)v1[0];
  double midy2 = (double)v1[3];
  double midx2 = (double)v1[2];
  /*pair<double, double> middle1 = make_pair(midx1, midy1);
    pair<double, double> middle2 = make_pair(midx2, midy2); */

  double dy1 = midy1 - vp.y;
  double dy2 = midy2 - vp.y;
  double dx1 = midx1 - vp.x;
  double dx2 = midx2 - vp.x;

  double dis1 = sqrt(dx1*dx1 + dy1*dy1);
  double dis2 = sqrt(dx2*dx2 + dy2*dy2);

  return std::min(dis1, dis2);
}

std::vector<im::Pointd> im::detectVertexes(const std::vector<cv::Vec4i> &segments) {
  /*
  vps_whouse
  vps
  getpoint
  getdis
  */
  int i = 0;
  int vsize = segments.size(); //辺数
  std::vector<double> dises;
  std::vector<im::Pointd> vps_whouse; //交点の仮置き場
  std::vector<im::Pointd> vps;

  for (cv::Vec4i vecs : segments) { //辺1つを取得 [基準辺]
    im::Pointd vp;
    for (int j = 0; j < vsize; j++) { //別の辺を取得(ダブり) [比較辺]
      if (i == j) continue;
      vp = getpoint(vecs, segments[j]);
      double dis = getdis(vecs, segments[j], vp);
      vps_whouse.push_back(vp);
      dises.push_back(dis);
    }
    if (vps_whouse.size() == 0) break;
    //最小値を求める
    std::vector<double>::iterator min_dis = std::min_element(dises.begin(), dises.end());
    int index = distance(dises.begin(), min_dis);
    vps.push_back(vps_whouse[index]);
    vps_whouse.clear();
    vps_whouse.shrink_to_fit();
    dises.clear();
    dises.shrink_to_fit();
    i++; //次の基準辺へ
    //if(i==4) break; //!!!デバッグ用　消すこと!!!
  }

  return vps;
}
/*
######################################################################
information about shape
N:na xa1 ya1 xa2 ya2 ... xana yana:nb xb1 yb1 xb2 yb2 ...xbna ybna:...
######################################################################
*/

//For the time being, coordinates is written by [mm]


std::vector<std::vector<im::Point>> im::roll(std::vector<im::Pointd> shape) {
  //Transfer [pix -> mm]
  double ratio = 55 / (512 * 2.5); //[(mm/pix/mm)]
  //double ratio = 1; //[(mm/pix/mm)]
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

  for (int corn = 0; corn < len_corn; corn++) {
    int dx = shape[corn].x
      - shape[(corn < len_corn - 1) ? corn + 1 : 0].x;
    int dy = shape[corn].y
      - shape[(corn < len_corn - 1) ? corn + 1 : 0].y;
    len_side[corn] = sqrt(dx*dx + dy*dy);
  }

  /*
  >>terms to stop<<
  1. Theta > 90
  2. dx > len_side[0]

  >>terms to break<<
  1. A point isn't at any grids.
  */

  double dy0 = shape[1].y - shape[0].y;
  double theta0 = 0;
  if (dy0 != 0) {
    theta0 = acos(dy0 / len_side[0]);
    //cout << len_side[0] << endl;
  }
  else {
    theta0 = 0;
  }
  double theta = 0, dtheta = 0;
  for (double dy = (int)len_side[0]; dy <= len_side[0] && theta <= PI / 4; dy--) {
    //(dx!=0) ? theta = asin(dx/len_side[0]) : theta = 0;
    theta = acos(dy / len_side[0]);
    dtheta = theta0 - theta;
    //cout << theta0*DP << ":" << theta*DP << ":" << dtheta*DP << endl;
    for (int corn = 0; corn < len_corn; corn++) {
      //Rotation matrix
      double x =
        (shape[corn].x - shape[0].x)*cos(dtheta) -
        (shape[corn].y - shape[0].y)*sin(dtheta);
      double y =
        (shape[corn].x - shape[0].x)*sin(dtheta) +
        (shape[corn].y - shape[0].y)*cos(dtheta);
      /*cout << x << "," << y << endl;
      cout << ceil(x) << "," << ceil(y) << endl;
      cout << floor(x) << "," << floor(y) << endl;*/
      bool flagx = false;
      bool flagy = false;
      if (abs(floor(x) - x) < derror) { x = floor(x); flagx = true; }
      if (abs(floor(y) - y) < derror) { y = floor(y); flagy = true; }
      if (abs(ceil(x) - x) < derror) { x = ceil(x); flagx = true; }
      if (abs(ceil(y) - y) < derror) { y = ceil(y); flagy = true; }
      if (flagx && flagy) tmp_res[corn] = im::Point(x, y);
      else break;

      if (corn == len_corn - 1) {
        result.push_back(tmp_res);
      }
    }
  }

  return result;
}
