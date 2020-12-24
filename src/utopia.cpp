#include "utopia.h"
using namespace std;

internal void GameOutputSound(game_sound_buffer *SoundBuffer, int Frequency)
{
    int Magnitude = 3000;
    // int Frequency = 256;
    int NumSamplePeriod = SoundBuffer->SamplesPerSecond / Frequency;
    local_persist real32 tSine;
    int16 *Cache = (int16 *)SoundBuffer->Cache;

    for (int i = 0; i < SoundBuffer->SamplesToWrite; ++i)
    {
        real32 SineValue = sinf(tSine);
        int16 SampleValue = (int16)(SineValue * Magnitude);
        *Cache++ = SampleValue;
        *Cache++ = SampleValue;

        tSine += 2.0f * Pi32 * 1.0f / (real32)NumSamplePeriod;
    }
}

internal void
RenderWeirdGradient(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
    // c will move ptr by the size of the type it's being pointed to.
    // for unit8, it will move by 1
    uint8 *Row = (uint8 *)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < Buffer->Width; ++X)
        {
            /* 
                Pixel in memory: BB GG RR XX
                Register:        xx RR GG BB
                LITTLE ENDIAN ARCHITECTURE
            */
            uint8 Blue = (X + BlueOffset);
            uint8 Green = (Y + GreenOffset);
            uint8 Red = 0;
            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Buffer->Pitch;
    }
}

void GameUpdateAndRender(game_memory *Memory,
                         game_input *Input,
                         game_offscreen_buffer *Buffer,
                         game_sound_buffer *SoundBuffer)
{
    game_state *GameState = (game_state *) Memory->Memory;
    if (!Memory->isInitialized){
        GameState->Frequency = 256;
        Memory->isInitialized = true;
    }

    game_controller_input *Input0 = &Input->Controllers[0];
    if (Input0->IsAnalog)
    {
        GameState->Frequency = 256 + (int)(128.0f * (Input0->EndX));
        GameState->BlueOffset += (int)4.0f * (Input0->EndY);
    }
    else
    {
    }

    // Input.AButtenEndedDown
    // Input.AButtenHalfTransitionCount
    if (Input0->Down.EndedDown)
    {
        GameState->GreenOffset += 1;
    }

    // TODO: Allow sample offsets here for more robust platform options
    GameOutputSound(SoundBuffer, GameState->Frequency);
    RenderWeirdGradient(Buffer, GameState->BlueOffset, GameState->GreenOffset);
}