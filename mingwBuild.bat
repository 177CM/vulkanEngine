@echo off
if not exist build mkdir build
cd build
cmake -S ../ -G "MinGW Makefiles"
mingw32-make.exe && mingw32-make.exe Shaders
cd .. 