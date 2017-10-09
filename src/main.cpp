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

#include "polyclipping/clipper.hpp"

namespace cl = ClipperLib;

union Color {
  unsigned int raw;
  unsigned char bytes[4];
};

cv::Scalar uint2scalar(unsigned int _color)
{
  Color color = Color{ _color };
  std::cerr << (int)color.bytes[0] << " " << (int)color.bytes[1] << " " << (int)color.bytes[2] << " " << (int)color.bytes[3] << std::endl;
  return cv::Scalar(color.bytes[0], color.bytes[1], color.bytes[2], color.bytes[3]);
}

static void DrawPolygons(const cl::Paths& _paths, bool fill = false)
{
  if (_paths.size() == 0) {
    std::cerr << "nothing to draw" << std::endl;
  }
  cv::Mat img = cv::Mat::zeros(cv::Size(1500, 1200), CV_8UC4);
  img = uint2scalar(0x00FFFFFF);
  srand((unsigned)time(NULL));
  for (const auto& _path : _paths) {
    const cl::Paths one = {_path};
    std::vector<std::vector<cv::Point>> paths;
    std::vector<int> npts;

    for (const auto& path : one) {
      std::vector<cv::Point> points;
      for (const auto& point : path) {
        points.push_back(cv::Point(point.X, point.Y));
      }
      paths.push_back(points);
      npts.push_back(points.size());
    }
    std::vector<cv::Point *> raw_paths(paths.size());
    for (int i = 0; i < paths.size(); ++i) {
      raw_paths[i] = &paths[i][0];
    }

    if (fill)
      cv::fillPoly(img, (const cv::Point **) &raw_paths[0], &npts[0], paths.size(), uint2scalar(rand()));
    cv::polylines(img, (const cv::Point **) &raw_paths[0], &npts[0], paths.size(), true, uint2scalar(rand()));
  }

  cv::imshow("clipper sample", img);
  cv::imwrite("drawn.bmp", img);

  cv::waitKey(0);
  cv::destroyAllWindows();
}

std::vector< im::Piece > getShapeHints();
void QReader(int device_id);

int main_using_imagawa() {


  //QReader(0);
  //exit(1);

  const std::string imageName = "image";
  const std::string extension = ".bmp";

  std::vector< cv::Mat > imgs;


  /*
  var index
      - 0     : frame image
      - non 0 : piece images
  */
  int index = 0;
  while (true) {
    cv::Mat img = cv::imread("/Users/Yuuki/Documents/Programming/procon/procon28/build/src/" + imageName + std::to_string(index++) + extension, cv::IMREAD_GRAYSCALE);
    if (img.empty()) break;
    else imgs.push_back(img);
  }

  /*FILE *fp;
  if((fp = fopen("/Users/Yuuki/Documents/Programming/procon/procon28/build/hint-shape.dat", "r")) != NULL) {
      int n;
      printf("aaa");
      while(~fscanf(fp, "%d", &n)) {
          std::vector< cl::IntPoint > p;
          std::cout << n << std::endl;
          for(int i = 0; i < n; i++) {
              int x, y; fscanf(fp, "%d %d", &x, &y);
               p.push_back(cl::IntPoint(x, y));
          }
          cl::Paths paths;
          paths.push_back(p);
          DrawPolygons(paths,  0x160000FF, 0x600000FF);
      }
  } */




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
  for (int i = 0; i < imgs.size(); i++) {
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
      // std::cout << segments.size() << ":" << vertexes.size() << std::endl;
      auto rolltexes = im::roll(id, vertexes); // <-これが1ピース当たりの回転を含めた座標を返します
      /*std::cout << rolltexes.id << "-----" << std::endl;
      for (const auto &rolling : rolltexes.vertexes) {
        std::cout << "=====" << std::endl;
        for (const auto &segment : rolling) {
          std::cout << ">> " << segment.x << "," << segment.y << std::endl;
        }
    } */

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
        for (const auto& ver : rolltexes.vertexes) {
          int minX = 1000000, maxX = -1;
          for (const auto& p : ver) {
            maxX = std::max(maxX, p.x);
            minX = std::min(minX, p.x);
          }

          std::vector< im::Point > newPoint;
          int midX = (maxX - minX) / 2;
          for (const auto& p : ver) {
            int newX;
            int dif = abs(p.x - midX);
            if (p.x > midX)    newX = p.x - dif * 2;
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
  cl::Paths gui;
  for (int i = 0; i < answers.size(); i++) {
    cl::Path ps;
    //for(auto p: problem) {

    if (answers[i].index < 0) continue;
    for (auto v : problem[i].vertexes[answers[i].index]) {
      v.x += answers[i].point.x;
      v.y += answers[i].point.y;
      ps.push_back(cl::IntPoint(v.x * 6, v.y * 6));
    }
    //}
    gui.push_back(ps);

  }
  DrawPolygons(gui);

    return 0;

}


int main() {
  // load hints
  std::string path;
  std::cerr << "ヒントファイルの場所を入力してください : ";
  std::cin >> path;
  std::ifstream ifs(path);

  im::Piece frame;

  int n;
  ifs >> n;

  std::vector<im::Point> wa;
  int m;
  ifs >> m;
  for (int i = 0; i < m; ++i) {
    int x, y;
    ifs >> x >> y;
    wa.push_back(im::Point(x, y));
  }

  frame = im::Piece(-1, { wa });
  std::vector<im::Piece> problem;
  for (int i = 0; i < n; ++i) {
    int l;
    ifs >> l;
    std::vector<im::Point> vec;
    for (int j = 0; j < l; ++j) {
      int x, y;
      ifs >> x >> y;
      vec.push_back(im::Point(x, y));
    }
    std::vector< std::vector< im::Point >  > newPoints;
    // auto rolltexes = im::Piece(i, {vec});
    auto rolltexes = im::easy_roll(i, vec);
    for (const auto& ver : rolltexes.vertexes) {
      int minX = 1000000, maxX = -1;
      for (const auto& p : ver) {
        maxX = std::max(maxX, p.x);
        minX = std::min(minX, p.x);
      }

      std::vector< im::Point > newPoint;
      int midX = (maxX - minX) / 2;
      for (const auto& p : ver) {
        int newX;
        int dif = abs(p.x - midX);
        if (p.x > midX)    newX = p.x - dif * 2;
        else              newX = p.x + dif * 2;
        newPoint.push_back(im::Point(newX, p.y));
      }
      newPoints.push_back(newPoint);
    }
    rolltexes.vertexes.insert(rolltexes.vertexes.end(), newPoints.begin(), newPoints.end());
  /*  for (auto a : rolltexes.vertexes) {
      cl::Path p;
      for (auto b : a) {
        p << cl::IntPoint(31 + b.x * 31, 31 + b.y * 31);
      }
      DrawPolygons({p});
    } */

    problem.push_back(rolltexes);
  }

  auto answers = tk::search(frame, problem, {}, 0);

  cl::Paths gui;
  for( int i = 0; i < answers.size(); i++)  {
      cl::Path ps;
      //for(auto p: problem) {

          if(answers[i].index < 0) continue;
          for(auto v: problem[i].vertexes[answers[i].index]) {
              v.x += answers[i].point.x;
              v.y += answers[i].point.y;
              ps.push_back(cl::IntPoint(v.x * 6, v.y * 6));
          }
      //}
      gui.push_back(ps);

  }
  //cl::Path f;
  //for( auto p: frame.vertexes.front() ) {
  //  f.push_back(cl::IntPoint(p.x * 6, p.y * 6));
  //}
  //gui.push_back(f);
  DrawPolygons(gui, true);

  return 0;
}



static int offset_id = 1000;
std::vector< im::Piece > getShapeHints() {

  std::vector< im::Piece > ret;
  FILE *fp;
  if ((fp = fopen("hint-shape.dat", "r")) != NULL) {
    int N;
    while (~fscanf(fp, "%d", &N)) {
      std::vector< im::Point > points;
      for (int i = 0; i < N; i++) {
        int x, y; fscanf(fp, "%d %d", &x, &y);
        points.push_back(im::Point(x, y));
      }
      ret.push_back(im::Piece(offset_id++, { points }));
    }
  }

  return ret;
}

// done!
void QReader(int device_id) {

  // USBカメラを開く
  cv::VideoCapture cap(device_id);

  // USBカメラが開けていない時は終了
  if (!cap.isOpened()) {
    std::cerr << "Video device " << device_id << " is not found" << std::endl;
    exit(1);
  }

  // USBカメラの解像度の設定
  cap.set(CV_CAP_PROP_FRAME_WIDTH, 900);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, 900);

  cv::namedWindow("QReader", 1);

  for (;;) {
    cv::Mat frame;

    // キャプチャデバイスを開くとストリームみたいに使えるので
    // Matのインスタンスに突っ込める
    cap >> frame;

    cv::imshow("QReader", frame);

    int key = cv::waitKey(1);

    // qが押されると無限ループから離脱
    // wが押されると画像を保存
    if (key == 119) {
      //result.emplace_back(frame.clone());
      std::cout << "Saved!" << std::endl;

      cv::imwrite("hint.png", frame.clone());
      // temp01.datへ文字列書き出し
      system("zbarimg hint.png > temp01.dat 2>&1");

      char c;
      std::string info = "";
      FILE *fp;

      int N;
      std::vector< std::vector< std::pair< int, int > > > vers;

      // temp01.pngからQRの内容を読む
      if ((fp = fopen("temp01.dat", "r")) != NULL) {
        while (~fscanf(fp, "%c", &c)) {
          info += c;
        }
        fclose(fp);
        system("rm temp01.dat");

        info.erase(0, 8);
        if (info.find("WARNING") != -1) {
          printf("QRCode is not detected.\n");
          goto next;
        }
        if ((fp = fopen("temp02.dat", "w")) != NULL) {
          fprintf(fp, "%s", info.c_str());
        }
        fclose(fp);

        std::string val = "";
        if ((fp = fopen("temp02.dat", "r")) != NULL) {
          fscanf(fp, "%d", &N);
          vers = std::vector< std::vector< std::pair< int, int > > >(N);
          for (int i = 0; i < N; i++) {
            int ver_N;
            fscanf(fp, ":%d", &ver_N);
            if (i == 0) val += std::to_string(ver_N);
            for (int j = 0; j < ver_N; j++) {
              int x, y; fscanf(fp, "%d %d", &x, &y);
              vers[i].push_back(std::make_pair(x, y));
              if (i == 0) val += " " + std::to_string(x) + " " + std::to_string(y);
            }
          }
        }
        val += '\n';
        fclose(fp);
        system("rm temp02.dat");

        bool exist = false;
        if ((fp = fopen("hint-shape.dat", "r")) != NULL) {
          char s[1000];
          while (fgets(s, sizeof(s), fp) != NULL) {
            std::string str = std::string(s);
            if (str == val) {
              printf("This hints already exist.\n");
              exist = true;
              break;
            }
          }
        }
        fclose(fp);

        if ((fp = fopen("hint-shape.dat", "a")) != NULL && !exist) {
          for (int i = 0; i < N; i++) {
            fprintf(fp, "%d", (int)vers[i].size());
            for (int j = 0; j < vers[i].size(); j++) {
              fprintf(fp, " %d %d", vers[i][j].first, vers[i][j].second);
            }
            fprintf(fp, "\n");
          }
        }
        fclose(fp);
      }
    }
    if (key == 113) {
      break;
    }
  next:;
  }
  cv::destroyWindow("QReader");
}
