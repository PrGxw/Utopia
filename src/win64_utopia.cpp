
#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

struct win32_window_dimension
{
    int Width;
    int Height;
};

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_GET_STATE(x_input_get_state);
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return (ERROR_DEVICE_NOT_CONNECTED);
}
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return (ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

// HRESULT WINAPI DirectSoundCreate(_In_opt_ LPCGUID pcGuidDevice, _Outptr_ LPDIRECTSOUND *ppDS, _Pre_null_ LPUNKNOWN pUnkOuter);
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void
Win32LoadXinput(void)
{
    // TODO: Test this on indows 8

    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if (!XInputLibrary)
    {
        // TODO: Diagnostic
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    if (XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
    else
    {
        // TODO: Diagnostic
    }
}

internal void
Win32InitDSound(HWND Window, int32 SamplePerSecond, int32 BufferSize)
{
    // NOTE: Load the Library
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if (DSoundLibrary)
    {
        // NOTE: Get a DirectSound object!
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

        // TODO: double check that this works on XP
        LPDIRECTSOUND DirectSound;
        if (DirectSoundCreate && (DirectSoundCreate(0, &DirectSound, 0) == DS_OK))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplePerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;

            if (DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY) == DS_OK)
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                // NOTE: "Create" a primary buffer
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if ((DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)) == DS_OK)
                {

                    if (PrimaryBuffer->SetFormat(&WaveFormat) == DS_OK)
                    {
                        // NOTE: successfully set the format
                    }
                    else
                    {
                        // TODO: Diagnostics
                    }
                }
                else
                {
                    // TODO: Diagnostics
                }
            }
            else
            {
                // TODO: Diagnostics
            }
            // NOTE: "Create" a secondary buffer that we write to
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            LPDIRECTSOUNDBUFFER SecondaryBuffer;
            if ((DirectSound->CreateSoundBuffer(&BufferDescription, &SecondaryBuffer, 0)) == DS_OK)
            {
            }

            // NOTE: Start Playing
        }
        else
        {
            // TODO: Diagnostics
        }
    }
    else
    {
        // TODO: Diagnostic
    }
}
// TODO: This is a global for now.
// static keyword initializes variable to zero
global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackBuffer;

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    int Width = ClientRect.right - ClientRect.left;
    int Height = ClientRect.bottom - ClientRect.top;

    win32_window_dimension Dimension;
    Dimension.Width = Width;
    Dimension.Height = Height;
    return (Dimension);
}

internal void
RenderWeirdGradient(win32_offscreen_buffer *Buffer, int XOffset, int YOffset)
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
            uint8 Blue = (X + XOffset);
            uint8 Green = (Y + YOffset);
            uint8 Red = 0;
            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Buffer->Pitch;
    }
}

// device independent bitmask
// build buffer using windows to give us buffer to draw into
internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    // TODO: bullet proof this
    // Maybe don't free first, free after, then free first if that fails

    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;

    BITMAPINFOHEADER BitmapInfoHeader;
    BitmapInfoHeader.biSize = sizeof(BitmapInfoHeader);
    BitmapInfoHeader.biWidth = Width;
    BitmapInfoHeader.biHeight = -Height;
    BitmapInfoHeader.biPlanes = 1;
    BitmapInfoHeader.biBitCount = 32;
    BitmapInfoHeader.biCompression = BI_RGB;

    Buffer->Info.bmiHeader = BitmapInfoHeader;

    int BitmapMemorySize = (Width * Height) * Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Width * Buffer->BytesPerPixel;
}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer,
                           HDC DeviceContext,
                           int WindowWidth, int WindowHeight)
{
    StretchDIBits(DeviceContext,
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, Buffer->Width, Buffer->Height,
                  Buffer->Memory,
                  &Buffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
Win32MainWindowCallback(_In_ HWND Window,
                        _In_ UINT Message,
                        _In_ WPARAM WParam,
                        _In_ LPARAM LParam)
{
    LRESULT Result = 0;
    switch (Message)
    {
    case WM_SIZE:
    {
    }
    break;
    case WM_CLOSE:
    {
        // TOOD: handle this iwth a message to the user
        Running = false;
    }
    break;
    case WM_DESTROY:
    {
        // TODO: handle this as an error.
        Running = false;
    }
    break;
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
        uint32 VKCode = WParam;
        bool WasDown = ((LParam & (1 << 30)) != 0);
        bool IsDown = ((LParam & (1 << 31)) == 0);
        if (WasDown != IsDown)
        {
            if (VKCode == atoi("W"))
            {
            }
            else if (VKCode == atoi("A"))
            {
            }
            else if (VKCode == atoi("S"))
            {
            }
            else if (VKCode == atoi("D"))
            {
            }
            else if (VKCode == atoi("Q"))
            {
            }
            else if (VKCode == atoi("E"))
            {
            }
            else if (VKCode == VK_UP)
            {
            }
            else if (VKCode == VK_DOWN)
            {
            }
            else if (VKCode == VK_LEFT)
            {
            }
            else if (VKCode == VK_RIGHT)
            {
            }
            else if (VKCode == VK_ESCAPE)
            {
                OutputDebugStringA("Escape: ");
                if (IsDown)
                {
                    OutputDebugStringA("IsDown\n");
                }
                if (WasDown)
                {
                    OutputDebugString("WasDown\n");
                }
            }
            else if (VKCode == VK_SPACE)
            {
            }
        }

        // if we did bool AltKeyWasDown = (LParam & (1 << 29))
        // the compiler actually shove code to compare (LParam & (1 << 29)) == 0, which we really don't care at this moment
        // so why not just #define int32 bool32, such that it take the int comes out of (LParam & (1 << 29)) and use it directly
        bool32 AltKeyWasDown = (LParam & (1 << 29));
        if ((VKCode == VK_F4) && AltKeyWasDown)
        {
            Running = false;
        }
    }
    break;
    case WM_ACTIVATEAPP:
    {
        OutputDebugString("WM_ACTIVATEAPP\n");
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT Paint;
        HDC DeviceContext = BeginPaint(Window, &Paint);

        int X = Paint.rcPaint.left;
        int Y = Paint.rcPaint.top;
        int Width = Paint.rcPaint.right - Paint.rcPaint.left;
        int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

        win32_window_dimension Dimension = Win32GetWindowDimension(Window);
        Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);

        EndPaint(Window, &Paint);
    }
    break;
    default:
    {
        Result = DefWindowProc(Window, Message, WParam, LParam);
    }
    break;
    }
    return (Result);
}

int WinMain(HINSTANCE Instance,
            HINSTANCE PrevInstance,
            LPSTR CommandLine,
            int ShowCommand)
{
    Win32LoadXinput();

    // when the struct is intialized as {}, all the values are set to 0
    WNDCLASSEX WindowClass = {};

    Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

    WindowClass.cbSize = sizeof(WNDCLASSEXW);
    WindowClass.style = CS_OWNDC | CS_HREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    //  HICON     hIcon;
    WindowClass.lpszClassName = "Utopia";

    if (RegisterClassEx(&WindowClass))
    {
        HWND Window = CreateWindowEx(0, WindowClass.lpszClassName, "Utopia",
                                     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     0, 0, Instance, 0);
        if (Window)
        {
            // since we specified CS_OWNDC, we can just get one dc and not share with anyone else
            HDC DeviceContext = GetDC(Window);

            int XOffset = 0;
            int YOffset = 0;

            Win32InitDSound(Window, 48000, 48000 * sizeof(int16) * 2);

            Running = true;
            while (Running)
            {
                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if (Message.message == WM_QUIT)
                    {
                        Running = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                // TODO: should do this more frequently
                for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex)
                {
                    XINPUT_STATE ControllerState;
                    if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                    {
                        // TODO: SEE IF DWORD          dwPacketNumber; is incrementing too rapidly
                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                        bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                        bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                        bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
                        bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
                        bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
                        bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

                        int16 StickX = Pad->sThumbLX;
                        int16 StickY = Pad->sThumbLY;

                        if (AButton)
                        {
                            YOffset += 1;
                        }
                        if (BButton)
                        {
                            XOffset += 1;
                        }
                    }
                }
                XINPUT_VIBRATION Vibration;
                Vibration.wLeftMotorSpeed = 60000;
                Vibration.wRightMotorSpeed = 60000;
                XInputSetState(0, &Vibration);

                RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);

                ReleaseDC(Window, DeviceContext);
            }
        }
        else
        {
            // TODO: logging
        }
    }
    else
    {
        // TODO: logging
    }

    return (0);
}
