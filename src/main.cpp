#include "main.h"
// http://www.codinglabs.net/article_world_view_projection_matrix.aspx
static void
ResizeDIBSection(win64_screen_buffer *Buffer, int Width, int Height)
{
    // TODO(casey): Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.

    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;

    int BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Buffer->Width * Buffer->Height) * BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Width * BytesPerPixel;
}

static void
DrawPixel(float x, float y)
{
    int index = ((int)x + (int)y * ScreenBuffer.Pitch / 4);
    uint32 *pixel = (uint32 *)ScreenBuffer.Memory;
    uint8 Blue = 0;
    uint8 Green = 0;
    uint8 Red = 0;
    pixel[index] = ((Red << 16) | (Green << 8) | Blue);
}

static void
FillScreenBuffer(win64_screen_buffer *Buffer)
{
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
            uint8 Blue = 255;
            uint8 Green = 255;
            uint8 Red = 255;
            *Pixel++ = ((Red << 16) | (Green << 8) | Blue);
        }
        Row += Buffer->Pitch;
    }

    // projections
    // http://www.songho.ca/opengl/gl_projectionmatrix.html
    int square[4][3] = {{1, 1, -1}, {1, -1, -1}, {-1, 1, -1}, {-1, -1, -1}};
    float width = 2;
    float height = 2;
    float z_far = -2;
    float z_near = 0;
    theta += 0.01;
    float projection_matrix[4][4] = {{1 / width, 0, 0, 0},
                                     {0, 1 / height, 0, 0},
                                     {0, 0, -2 / (z_far - z_near), -(z_far + z_near) / (z_far - z_near)},
                                     {0, 0, 0, 1}};

    for (int i = 0; i < MatrixRowSize(square); i++)
    {
        float rotation_matrix[4][4];
        float rotation_matrix1[4][4] = {{cosf(theta), sinf(theta), 0, 0},
                                        {-sinf(theta), cosf(theta), 0, 0},
                                        {0, 0, 1, 0},
                                        {0, 0, 0, 1}};
        float rotation_matrix2[4][4] = {{1, 0, 0, 0},
                                        {0, cosf(theta), sinf(theta), 0},
                                        {0, -sinf(theta), cosf(theta), 0},
                                        {0, 0, 0, 1}};
        matrix_multiply((float *)rotation_matrix1, MatrixRowSize(rotation_matrix1), MatrixColSize(rotation_matrix1),
                        (float *)rotation_matrix2, MatrixRowSize(rotation_matrix2), MatrixColSize(rotation_matrix2),
                        (float *)rotation_matrix);
        // float rotation_matrix[4][4] = {{1,0,0,0},{0,1,0,0}, {0,0,1,0}, {0,0,0,1}};
        float point[4] = {};
        float vertex[4] = {};
        float transform_matrix[4][4] = {};
        for (int j = 0; j < 3; j++)
        {
            vertex[j] = (float)square[i][j];
        }
        vertex[3] = 1;
        // matrix_multiply((float *)rotation_matrix, MatrixRowSize(rotation_matrix), MatrixColSize(rotation_matrix),
        // (float *)projection_matrix, MatrixRowSize(projection_matrix), MatrixColSize(projection_matrix),
        // (float *)transform_matrix);
        matrix_multiply((float *)rotation_matrix, MatrixRowSize(rotation_matrix), MatrixColSize(rotation_matrix),
                        (float *)vertex, MatrixRowSize(vertex), MatrixColSize(vertex),
                        (float *)point);
        DrawPixel((point[0] + 10) * 50, (point[1] + 10) * 50);
    }
}

static win64_window_dimension
GetWindowDimension(HWND Window)
{
    win64_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return (Result);
}

static void DisplayBufferInWindow(win64_screen_buffer *ScreenBuffer,
                                  HDC DeviceContext,
                                  win64_window_dimension *WindowDimension)
{
    StretchDIBits(DeviceContext,
                  /*
                  X, Y, Width, Height,
                  X, Y, Width, Height,
                  */
                  0, 0, WindowDimension->Width, WindowDimension->Height,
                  0, 0, ScreenBuffer->Width, ScreenBuffer->Height,
                  ScreenBuffer->Memory,
                  &ScreenBuffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

internal LRESULT CALLBACK
Win64MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
    case WM_CLOSE:
    {
        // TODO(casey): Handle this with a message to the user?
        GlobalRunning = false;
    }
    break;

    case WM_ACTIVATEAPP:
    {
        OutputDebugStringA("WM_ACTIVATEAPP\n");
    }
    break;

    case WM_DESTROY:
    {
        // TODO(casey): Handle this as an error - recreate window?
        GlobalRunning = false;
    }
    break;

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_PAINT:
    default:
    {
        //            OutputDebugStringA("default\n");
        Result = DefWindowProcA(Window, Message, WParam, LParam);
    }
    break;
    }

    return (Result);
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{

    WNDCLASSA WindowClass = {};

    WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    WindowClass.lpfnWndProc = Win64MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClassA(&WindowClass))
    {
        HWND Window = CreateWindowExA(
            0, WindowClass.lpszClassName, "Handmade Hero",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            0, 0, Instance, 0);
        if (Window)
        {
            // NOTE: Since we specified CS_OWNDC, we can just
            // get one device context and use it forever because we
            // are not sharing it with anyone.
            HDC DeviceContext = GetDC(Window);

            GlobalRunning = true;

            int ResolutionInitialHeight = 1280;
            int ResolutionInitialWidth = 720;
            ResizeDIBSection(&ScreenBuffer, ResolutionInitialHeight, ResolutionInitialWidth);

            while (GlobalRunning)
            {
                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if (Message.message == WM_QUIT)
                    {
                        GlobalRunning = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }

                FillScreenBuffer(&ScreenBuffer);
                win64_window_dimension Dimension = GetWindowDimension(Window);
                DisplayBufferInWindow(&ScreenBuffer, DeviceContext, &Dimension);
            }
        }
        else // Windows create window failed
        {
        }
    }
    else // windows register windows class failed
    {
    }

    return (0);
}
