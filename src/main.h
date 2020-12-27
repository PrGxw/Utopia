#include <windows.h>
#include "data_types.h"
#include "matrix.h"

#define internal static
#define local_persist static
#define global_variable static

struct win64_window_dimension
{
    int Width;
    int Height;
};

struct win64_screen_buffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

global_variable bool32 GlobalRunning;
global_variable win64_screen_buffer ScreenBuffer;
global_variable float theta;
