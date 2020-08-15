@echo off

mkdir ..\..\bin
pushd ..\..\bin
cl -Zi ..\src\win64_utopia.cpp ..\src\utopia.cpp user32.lib gdi32.lib 
popd

