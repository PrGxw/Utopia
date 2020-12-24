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

struct game_button_state{
    int HalfTransitionCount;
    bool32 EndedDown;
};
struct game_controller_input{
    bool32 IsAnalog;
    real32 StartX;
    real32 StartY;

    real32 MinX;
    real32 MinY;

    real32 MaxX;
    real32 MaxY;

    real32 EndX;
    real32 EndY;
    union{
        game_button_state Button[6];
        struct{
            game_button_state Up;
            game_button_state Down;
            game_button_state Left;
            game_button_state Right;
            game_button_state LeftShoulder;
            game_button_state RightShoulder;
        };
    };
};

struct game_input{
    // TODO: insert clock value here.
    game_controller_input Controllers[4];
};

struct game_state {
    int BlueOffset;
    int GreenOffset;
    int Frequency;
};

struct game_memory {
    bool32 isInitialized;
    uint64 MemorySize;
    void * Memory;
};

void GameUpdateAndRender(game_memory *Memory,
                         game_input *Input,
                         game_offscreen_buffer *Buffer,
                         game_sound_buffer *SoundBuffer);

#endif /* UTOPIA */
