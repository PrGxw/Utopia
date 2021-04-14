#include <windows.h>
#include "data_types.h"
#include "engine.h"

#define internal static
#define local_persist static
#define global_variable static

#define PI 3.1415926f

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

global_variable bool32 GlobalRunning;
global_variable Win64ScreenBuffer ScreenBuffer;
global_variable float theta;
