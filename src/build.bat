@echo off

mkdir ..\bin
pushd ..\bin
cl -DSLOW_BUILD=1 -DINTERNAL_BUILD=1 -Zi ..\src\win64_utopia.cpp ..\src\utopia.cpp user32.lib gdi32.lib 
popd

