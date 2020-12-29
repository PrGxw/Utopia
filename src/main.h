#include <windows.h>
#include "data_types.h"
#include "matrix.h"

#define internal static
#define local_persist static
#define global_variable static

struct Win64WindowDimension
{
    int Width;
    int Height;
};

struct Win64ScreenBuffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct Point
{
    float x;
    float y;
    float z;
    float padding;
};

global_variable bool32 GlobalRunning;
global_variable Win64ScreenBuffer ScreenBuffer;
global_variable float theta;
