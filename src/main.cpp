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
    for(int i = 0; i < imgs.size(); i++) {
        cv::imshow(std::to_string(i), imgs[i]);
    }

    /*---- detective of lines and points ----*/
    im::Piece frame;
    std::vector< im::Piece > problem;

    std::vector<std::vector<im::Point>> coordinats_of_pieces_in_mat(imgs.size());
    for(int i = 0; i < imgs.size(); i++) {
        cv::threshold(imgs[i], imgs[i], 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        auto pieceImgs = im::devideImg(imgs[i], coordinats_of_pieces_in_mat[i]);

        int id = 1;
        std::cout << id << ":";
        int color_i = 0;
        cv::Canny(imgs[i], imgs[i], 50, 200, 3);
        auto segments = im::detectSegments(imgs[i]);
        auto vertexes = im::detectVertexes(segments);
        std::cout << segments.size() << ":" << vertexes.size() << std::endl;
        auto rolltexes = im::roll(id, vertexes); // <-これが1ピース当たりの回転を含めた座標を返します
        std::cout << rolltexes.id << "-----" << std::endl;
        for(auto &rolling : rolltexes.vertexes){
            std::cout << "=====" << std::endl;
            for(auto &segment : rolling){
                std::cout << ">> " << segment.x << "," << segment.y << std::endl;
            }
        }

        if(i == 0) {
            int mpos = -1;
            int maxi = -1;
            for(const auto& ver : rolltexes.vertexes) {
                int minX = 1000000, maxX = -1;
                for(const auto& p : ver) {
                    maxX = std::max(maxX, p.x);
                    minX = std::min(minX, p.x);
                }
                if(maxi < maxX - minX) {
                    mpos = i;
                }
            }
            rolltexes.vertexes = {rolltexes.vertexes[mpos]};
        }
        else problem.push_back(rolltexes);

        id++;

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
