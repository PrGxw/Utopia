#include "main.h"
// http://www.codinglabs.net/article_world_view_projection_matrix.aspx
// https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/projection-matrix-introduction
static void
ResizeDIBSection(Win64ScreenBuffer *Buffer, int Width, int Height)
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

void ClearScreenBuffer(Win64ScreenBuffer *Buffer)
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
}

static void
FillScreenBuffer(Win64ScreenBuffer *Buffer)
{
    ClearScreenBuffer(Buffer);
    // projections
    // http://www.songho.ca/opengl/gl_projectionmatrix.html


    /*
     * temporary variables
     */
    static float theta;
    // theta += 0.001;
    theta += 0.01f * PI;
    // static float distance;
    // distance -= 0.001;

    float camera_matrix[4][4];
    IdentityMatrix4x4((float *)camera_matrix);
    // Translate(0, 0, 0, (float *)camera_matrix);

    float view_matrix[4][4];
    MatrixInverse((float*)camera_matrix, 4, (float*)view_matrix);

    /*
     * temporary objects
     */
    int square[8][4] = { {1, 1, 1, 1}, {1, -1, 1, 1}, {-1, 1, 1, 1}, {-1, -1, 1, 1}, {1, 1, -1, 1}, {1, -1, -1, 1}, {-1, 1, -1, 1}, {-1, -1, -1, 1} };
    Shape triangle_front;
    float triangle_front_geometry[3][4] = { {1,1,0,1}, {1,-1,0,1}, {-1,1,0,1}};
    triangle_front.geometry = (float*) &triangle_front_geometry;
    triangle_front.num_elements = 3;

    Shape triangle_back;
    float triangle_back_geometry[3][4] = {{1,1,-1,1}, {1,-1,-1,1}, {-1,-1,-1,1}};
    triangle_back.geometry = (float*) &triangle_back_geometry;
    triangle_back.num_elements = 3;

    Shape* shapes[2];
    shapes[0] = &triangle_front;
    shapes[1] = &triangle_back;

    /*
     * image settings
     */
    float image_width = 16;
    float image_height = 9;
    float eye_distance = 1;

    for (int shape_index =0; shape_index < MatrixRowSize(shapes); shape_index ++){
        Shape* shape = shapes[shape_index];
        Point vertices[3] = {};
        for (int i = 0; i < shape->num_elements; i++)
        {
            float vertex[4] = {};
            for (int j = 0; j < 4; j++){
                vertex[j] = shape->geometry[(i*4)+j];
            }
            float transform_matrix[4][4];
            IdentityMatrix4x4((float*) transform_matrix);

            // 1. convert local to world
            // RotateXAxis(theta, (float*)transform_matrix);
            RotateYAxis(theta, (float*)transform_matrix);
            // RotateZAxis(theta, (float*)transform_matrix);
            Translate(0, 0, -2, (float*)transform_matrix);

            // 2. convert to camera space
            MatrixMultiply(
                (float *) view_matrix, 4, 4,
                (float *) transform_matrix, 4, 4,
                (float *) transform_matrix
            );
            // 2.5 convert to image space
            float point[4] = {};
            MatrixMultiply((float *)transform_matrix, 4, 4,
                        (float *)vertex, 4, 1,
                        (float *)point);
            float x = *(point + 0);
            float y = *(point + 1);
            float z = *(point + 2);
            *(point + 0) = (x/-z) * eye_distance;
            *(point + 1) = (y/-z) * eye_distance;

            // 3. convert to NDC space
            float ndc_point[2] = {};
            *(ndc_point + 0) = (*(point + 0) + image_width / 2) / image_width;
            *(ndc_point + 1) = (*(point + 1) + image_height / 2) / image_height;
            if ((*(ndc_point + 0) > 1) || (*(ndc_point + 1) > 1) || (z >= 0) || (*(ndc_point + 0) < 0) || (*(ndc_point + 1) < 0)) {
                continue;
            }

            // 5. convert to rasterspace
            Point raster_point;
            raster_point.x = (int)(*(ndc_point + 0) * Buffer->Width);
            raster_point.y = (int)(*(ndc_point + 1) * Buffer->Height);
            vertices[i] = raster_point;
            // 6. draw
            DrawPixel(raster_point.x, raster_point.y);
        }
        for (int row = 0; row < Buffer->Height; row++){
            for (int col = 0; col < Buffer->Width; col++){
                Point cur_point;
                cur_point.x = col;
                cur_point.y = row;
                if (ContainedInTriangle(&vertices[0], &vertices[1], &vertices[2], &cur_point)){
                    DrawPixel(col, row);
                }
            }
        }

    }


}

static Win64WindowDimension
GetWindowDimension(HWND Window)
{
    Win64WindowDimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return (Result);
}

static void DisplayBufferInWindow(Win64ScreenBuffer *ScreenBuffer,
                                  HDC DeviceContext,
                                  Win64WindowDimension *WindowDimension)
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
                Win64WindowDimension Dimension = GetWindowDimension(Window);
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
