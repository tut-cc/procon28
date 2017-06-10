#include "imagawa.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include<vector>
#include <algorithm>
#include <iostream>


im::Point::Point() : Point(0, 0) {}

im::Point::Point(int x_, int y_) : x(x_), y(y_) {}

im::Piece::Piece() : Piece(0, {}, {}, {}) {}

im::Piece::Piece(int id_, const std::vector<im::Point> &vertexes_, const std::vector<int> &edges2_,
  const std::vector<double> &degs_) : id(id_), vertexes(vertexes_), edges2(edges2_), degs(degs_) {}

im::Answer::Answer() : Answer(0, {}) {}

im::Answer::Answer(int id_, const std::vector<im::Point> &vertexes_) : id(id_), vertexes(vertexes_) {}

void im::hello() {
  std::cout << "hello 今川" << std::endl;
}

std::vector<cv::Mat> im::devideImg(const cv::Mat &binaryImg) {
  cv::Mat labels, stats, centroids;
  auto labelNum = cv::connectedComponentsWithStats(binaryImg, labels, stats, centroids);

  std::vector<cv::Mat> pieceImgs;
  for (auto i = 1; i < labelNum; i++) {
    auto *stat = stats.ptr<int>(i);

    auto l = std::max(stat[cv::ConnectedComponentsTypes::CC_STAT_LEFT] - 5, 0);
    auto t = std::max(stat[cv::ConnectedComponentsTypes::CC_STAT_TOP] - 5, 0);
    auto w = std::min(stat[cv::ConnectedComponentsTypes::CC_STAT_WIDTH] + 10, binaryImg.cols - l);
    auto h = std::min(stat[cv::ConnectedComponentsTypes::CC_STAT_HEIGHT] + 10, binaryImg.rows - t);
    auto s = stat[cv::ConnectedComponentsTypes::CC_STAT_AREA];

    if (s < 100) {
      continue;
    }

    auto pieceImg = binaryImg(cv::Rect(l, t, w, h)).clone();
    for (auto y = 0; y < h; y++) {
      auto *labelsRow = labels.ptr<int>(t + y);
      for (auto x = 0; x < w; x++) {
        if (labelsRow[l + x] != i) {
          pieceImg.data[w * y + x] = 0;
        }
      }
    }

    pieceImgs.push_back(pieceImg);
  }

  return pieceImgs;
}

std::vector<cv::Vec4i> im::detectSegments(const cv::Mat &edgeImg) {
  std::vector<cv::Vec4i> segments;
  cv::HoughLinesP(edgeImg, segments, 1, CV_PI / 180, 45, 30, 30);

  return segments;
}

im::Point getpoint(cv::Vec4i v1_o, cv::Vec4i v2_o){
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
  for(int i=0;i<4;i++){
    v1[i] = v1_o[i];
    v2[i] = v2_o[i];
  }
  if(v1[2]-v1[0] == 0) v1[2] += 0.001; //傾き無限大対策(応急処置)
  double a = ( v1[3] -  v1[1])/( v1[2] -  v1[0]);
  double b =  v1[1] - a* v1[0];
  if(v2[2]-v2[0] == 0) v2[2] += 0.001; //傾き無限大対策(応急処置)
  double c = ( v2[3] -  v2[1])/( v2[2] -  v2[0]);
  double d =  v2[1] - c* v2[0];
  //if(a == c) c+=1; //応急処置

	//傾きが一緒ならば省略
	if((a-c)*(a-c) < SLO) return im::Point(-1,-1);
  double x = (d - b)/(a - c);
  double y = a* x + b;

  return im::Point(x,y);
}

double getdis(cv::Vec4i v1, cv::Vec4i v2, im::Point vp){
  //各線分のいずれかの端っこからの距離を測定

  /*int midy1 = abs(v1[1]-v1[3])/2;
  int midx1 = abs(v1[0]-v1[2])/2;*/
  double midy1 = (double)v1[1];
  double midx1 = (double)v1[0];
  //double midy2 = (double)v1[3];
  //double midx2 = (double)v1[2];
  /*pair<double, double> middle1 = make_pair(midx1, midy1);
    pair<double, double> middle2 = make_pair(midx2, midy2); */

  double dy1 = midy1 - vp.y;
  //double dy2 = midy2 - vp.y;
  double dx1 = midx1 - vp.x;
  //double dx2 = midx2 - vp.x;

  double dis1 = sqrt(dx1*dx1 + dy1*dy1);
  //double dis2 = sqrt(dx2*dx2 + dy2*dy2);

  return dis1;
}

std::vector<im::Point> im::detectVertexes(const std::vector<cv::Vec4i> &segments) {
	/*
	vps_whouse
	vps
	getpoint
	getdis
	*/
	int i = 0;
	int vsize = segments.size(); //辺数
	std::vector<double> dises;
	std::vector<im::Point> vps_whouse; //交点の仮置き場
	std::vector<im::Point> vps;

	for(cv::Vec4i vecs:segments){ //辺1つを取得 [基準辺]
		im::Point vp;
		for(int j=0; j<vsize ;j++){ //別の辺を取得(ダブり) [比較辺]
			if(i == j) continue;
			vp = getpoint(vecs, segments[j]);
			double dis = getdis(vecs, segments[j], vp);
			vps_whouse.push_back(vp);
			dises.push_back(dis);
		}
		if(vps_whouse.size() == 0) break;
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
