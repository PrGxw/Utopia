#ifndef WIN64_UTOPIA
#define WIN64_UTOPIA


#include "utopia.h"

#include <windows.h>
#include <stdio.h>
#include <xinput.h>
#include <dsound.h>


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

struct win64_sound_output
{
    int SamplesPerSecond;
    int Frequency;
    int16 Magnitude;
    uint32 RunningSampleIndex;
    int NumSamplePeriod;
    int BytesPerSample;
    int SecondaryBufferSize;
    real32 tSine;
    int LatencySampleCount;
};

// TODO(casey): This is a global for now.
global_variable bool32 GlobalRunning;
global_variable win64_offscreen_buffer GlobalBackbuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

#endif /* WIN64_UTOPIA */
