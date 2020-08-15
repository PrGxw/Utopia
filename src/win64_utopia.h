#ifndef WIN64_UTOPIA
#define WIN64_UTOPIA


#include "utopia.h"

#include <windows.h>
#include <stdio.h>
#include <xinput.h>
#include <dsound.h>

// TODO(casey): Implement sine ourselves
#include <math.h>

struct win64_offscreen_buffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct win64_window_dimension
{
    int Width;
    int Height;
};

// TODO(casey): This is a global for now.
global_variable bool32 GlobalRunning;
global_variable win64_offscreen_buffer GlobalBackbuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

#endif /* WIN64_UTOPIA */
