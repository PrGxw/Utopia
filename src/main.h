#include <windows.h>
#include "data_types.h"
#include "engine.h"
#include "math.h"

#define internal static
#define local_persist static
#define global_variable static

#define PI 3.1415926f

struct win64_windows_dimension
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

struct mouse_input
{
    int X;
    int Y;
    bool32 LBDown;
    bool32 RBDown;
    bool32 MBDown;
};

struct game_input
{
    mouse_input Mouse;
};

struct game_state
{

};

global_variable bool32 GlobalRunning;
global_variable win64_screen_buffer ScreenBuffer;
global_variable win64_screen_buffer ZBuffer;
global_variable float theta;
global_variable game_input UserInput;
static bool fill = true;
