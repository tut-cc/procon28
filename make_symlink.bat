cd /d %~dp0
mkdir include
cd include
rmdir polyclipping
del polyclipping
mklink /D polyclipping ..\lib\clipper\cpp
pause
