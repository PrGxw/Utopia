@echo off

mkdir ..\bin
pushd ..\bin
cl -DSLOW_BUILD=1 -DINTERNAL_BUILD=1 -Zi ..\src\main.cpp ..\src\matrix.cpp user32.lib gdi32.lib
::cl -Zi ..\src\matrix.cpp
::cl -Zi ..\src\mywork.cpp
popd

