cd /d %~dp0
mkdir include
cd include
del polyclipping
mklink /D polyclipping ..\lib\clipper\cpp
pause
