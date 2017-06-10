#include "imagawa.hpp"
#include "tokugawa.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

int main()
{
  im::hello();
  tk::hello();

  std::string str;
  std::cin >> str;
  cv::Mat img = cv::imread(str, cv::IMREAD_GRAYSCALE);
  if (img.empty()) return -1;

  cv::threshold(img, img, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
  auto pieceImgs = im::devideImg(img);

  int id = 1;
  for (auto &pieceImg : pieceImgs) {
    cv::Canny(pieceImg, pieceImg, 50, 200, 3);
    auto segments = im::detectSegments(pieceImg);
    auto vertexes = im::detectVertexes(segments);

    cv::cvtColor(pieceImg, pieceImg, CV_GRAY2BGR);
    for (auto &segment : segments) {
      if (abs(segment[0] - segment[2]) + abs(segment[1] - segment[3]) < 20) {
        continue;
      }
      cv::line(pieceImg, cv::Point(segment[0], segment[1]),
        cv::Point(segment[2], segment[3]), cv::Scalar(0, 0, 255), 2, 8);
    }

    for (auto &vertex : vertexes) {
      cv::circle(pieceImg, cv::Point(vertex.x, vertex.y), 5, cv::Scalar(0, 255, 0), -1, 8, 0);
    }

    cv::namedWindow(std::to_string(id), CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
    cv::imshow(std::to_string(id), pieceImg);
    id++;
  }

  cv::waitKey(0);
}
