# procon28

## 環境構築

### Windows

環境変数`OPENCV_INC`、`OPENCV_LIB`を設定してください。  
例：それぞれ`C:\opencv-3.2.0\build\include`、`C:\opencv-3.2.0\build\x64\vc14\lib`

DLLがあるところにパスを通してください。

### Mac

まずbrew を入れてください。  
次に下記コマンドを叩きます。

```sh
brew install cmake
brew install homebrew/science/opencv3
```

repo のtop ディレクトリで以下を実行すると  
build されます。

```sh
mkdir build
cd build
cmake ..
```
