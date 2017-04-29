#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "imagawa.hpp"
#include "tokugawa.hpp"

int main()
{
  im::hello();
  tk::hello();

  std::string str;
  std::cin >> str;
  cv::Mat img = cv::imread(str, 1);
  if (img.empty()) return -1;

  cv::namedWindow("image", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
  cv::imshow("image", img);

  cv::waitKey(0);
}
