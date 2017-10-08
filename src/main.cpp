#include "imagawa.hpp"
#include "tokugawa.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <cstdio>
#include <vector>
#include <algorithm>

int main() {

    const std::string imageName = "image";
    const std::string extension = ".bmp";

    std::vector< cv::Mat > imgs;


    /*
    var index
        - 0     : frame image
        - non 0 : piece images
    */
    int index = 0;
    while(true) {
        cv::Mat img = cv::imread(imageName + std::to_string(index++) + extension, cv::IMREAD_GRAYSCALE);
        if(img.empty()) break;
        else imgs.push_back(img);
    }

    // test
    //for(int i = 0; i < imgs.size(); i++) {
    //    auto name = std::to_string(i);
    //    cv::imshow(name, imgs[i]);
    //    cv::waitKey(0);
    //}
    //cv::destroyAllWindows();
    //cv::waitKey(0);

    /*---- detective of lines and points ----*/
    im::Piece frame;
    std::vector< im::Piece > problem;

    std::vector<std::vector<im::Point>> coordinats_of_pieces_in_mat(imgs.size());
    int id = 1;
    for(int i = 0; i < imgs.size(); i++) {
        if (i == 0) {
            auto cutImg = imgs[i](cv::Rect(20, 20, imgs[i].cols - 40, imgs[i].rows - 40)).clone();
            imgs[i] = std::move(cutImg);
        }

        cv::threshold(imgs[i], imgs[i], 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        auto pieceImgs = im::devideImg(imgs[i], coordinats_of_pieces_in_mat[i]);

        for (const cv::Mat& piece_img : pieceImgs) {
          std::cout << id << ":";
          int color_i = 0;
          cv::Mat dst;
          cv::Canny(piece_img, dst, 50, 200, 3);
          const auto& segments = im::detectSegments(dst);
          const auto& vertexes = im::detectVertexes(segments);
          std::cout << segments.size() << ":" << vertexes.size() << std::endl;
          auto rolltexes = im::roll(id, vertexes); // <-これが1ピース当たりの回転を含めた座標を返します
          std::cout << rolltexes.id << "-----" << std::endl;
          for (const auto &rolling : rolltexes.vertexes) {
            std::cout << "=====" << std::endl;
            for (const auto &segment : rolling) {
              std::cout << ">> " << segment.x << "," << segment.y << std::endl;
            }
          }

          if (i == 0) {
            int mpos = -1;
            int maxi = -1;
            for (const auto& ver : rolltexes.vertexes) {
              int minX = 1000000, maxX = -1;
              for (const auto& p : ver) {
                maxX = std::max(maxX, p.x);
                minX = std::min(minX, p.x);
              }
              if (maxi < maxX - minX) {
                maxi = maxX - minX;
                mpos = i;
              }
            }
            rolltexes.vertexes = { rolltexes.vertexes[mpos] };
            frame = rolltexes;
          }
          else {
              std::vector< std::vector< im::Point >  > newPoints;
              for(const auto& ver: rolltexes.vertexes) {
                  int minX = 1000000, maxX = -1;
                  for(const auto& p: ver) {
                      maxX = std::max(maxX, p.x);
                      minX = std::min(minX, p.x);
                  }

                  std::vector< im::Point > newPoint;
                  int midX = (maxX - minX) / 2;
                  for(const auto& p: ver) {
                      int newX;
                      int dif = abs(p.x - midX);
                      if(p.x > midX)    newX = p.x - dif * 2;
                      else              newX = p.x + dif * 2;
                      newPoint.push_back(im::Point(newX, p.y));
                  }
                  newPoints.push_back(newPoint);
              }
              rolltexes.vertexes.insert(rolltexes.vertexes.end(), newPoints.begin(), newPoints.end());
              problem.push_back(rolltexes);
          }

          id++;
        }
        /* ----- debug ----- */
        /*
        cv::cvtColor(pieceImg, pieceImg, CV_GRAY2BGR);
        for (auto &segment : segments) {

            //端点表示
            //cv::circle(pieceImg, cv::Point(segment[0], segment[1]), 5, cv::Scalar(255, 255, 255), -1, 8, 0);
            //cv::circle(pieceImg, cv::Point(segment[2], segment[3]), 5, cv::Scalar(255, 255, 255), -1, 8, 0);

            cv::line(pieceImg, cv::Point(segment[0], segment[1]),
            cv::Point(segment[2], segment[3]), cv::Scalar(color_i, color_i, 255), 2, 8);
        }

        for (auto &vertex : vertexes) {
          if (vertex.x == -1) continue;
          cv::circle(pieceImg, cv::Point(vertex.x, vertex.y), 5, cv::Scalar(color_i, 255, color_i), -1, 8, 0);
          color_i += 20;
        }

        cv::resize(pieceImg, pieceImg, cv::Size(), 0.5, 0.5);
        cv::namedWindow(std::to_string(id), CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
        cv::imshow(std::to_string(id), pieceImg);  */
    }

    // waiting for input
    // cv::waitKey(0);


    /*----- call a search func -----*/
    // type vector< im::Answer > answers
    auto answers = tk::search(frame, problem, {}, 0);

    /*----- output answer by GUI -----*/



}
