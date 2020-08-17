#ifndef UTOPIA
#define UTOPIA

#include <stdint.h>
// Implement sine ourselves
#include <math.h>

#define internal static 
#define local_persist static 
#define global_variable static

#define Pi32 3.14159265359f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

/*
 * Services that the platform layer provides to the game
 */

/*
 * Services that the game provides to the platform layer
 * (this may expand in the future, like sound on separate thread)
 */

// Takes FOUR THINGS - timing, controller/keyboard input,
// bitmap buffer to user, sound buffer to use
struct game_offscreen_buffer
{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct game_sound_buffer
{
    void* Cache;
    int SamplesToWrite;
    int SamplesPerSecond;
};

void GameUpdateAndRender(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset,
                         game_sound_buffer *SoundBuffer, int Frequency);

#endif /* UTOPIA */
