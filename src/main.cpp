#include "imagawa.hpp"
#include "tokugawa.hpp"
#include <iostream>
#include <string>
#include <cstdio>
#include <vector>

int main() {

    const string imageName = "image"
    const string extension = ".bmp";

    std::vector< cv::Mat > imgs;


    /*
    var index
        - 0     : frame image
        - non 0 : piece images
    */
    int index = 0;
    while(true) {
        cv:Mat img = cv::im_read(imageName + to_string(index++) + extension, cv::IMREAD_GRAYSCALE);
        if(img.empty()) break;
        else imgs.push_back(img);
    }

    // test
    for(int i = 0; i < imgs.size(); i++) {
        cv:imshow(i, imgs[i]);
    }

    /*---- detective of lines and points ----*/
    vector< vector< im::Point > > frame;
    vector< vector< im::Point > > problem;

    for(int i = 0; i < imgs.size(); i++) {
        cv::threshold(imgs[i], imgs[i], 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        auto pieceImgs = im::devideImg(imgs[i]);

        int id = 1;
        std::cout << id << ":";
        int color_i = 0;
        cv::Canny(pieceImg, pieceImg, 50, 200, 3);
        auto segments = im::detectSegments(pieceImg);
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

        if(i == 0)  frame.push_back(rolltexes.vertexes);
        else        problem.push_back(rolltexes.vertexes);

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
    auto answers = tk:search(frame, problem);

    /*----- output answer by GUI -----*/



}
