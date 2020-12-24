#ifndef WIN64_UTOPIA
#define WIN64_UTOPIA


#include "utopia.h"

#include <windows.h>
#include <stdio.h>
#include <xinput.h>
#include <dsound.h>

/*
 * Note:
 * INTERNAL_BUILD: 
 *  0 - Build for public release
 *  1 - Build for developer
 * SLOW_BUILD: 
 *  0 - No slow code allowed
 *  1 - slow code allowed
 */

#if SLOW_BUILD
#define Assert(Expression) if (!(Expression)) {*(int *)0 = 0;} 
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) (Megabytes(Value)*1024)
#define Terabytes(Value) (Gigabytes(Value)*1024)

#define ArrayCount(Array) (sizeof(Array)  / sizeof(Array[0]))

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
    uint32 RunningSampleIndex;
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
