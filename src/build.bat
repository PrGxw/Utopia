@echo off

mkdir ..\..\build
pushd ..\..\build
cl -Zi ..\src\win64_utopia.cpp user32.lib gdi32.lib 
popd

