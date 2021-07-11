#include "main.h"
// http://www.codinglabs.net/article_world_view_projection_matrix.aspx
// https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/projection-matrix-introduction
static void
ResizeDIBSection(win64_screen_buffer *Buffer,
                 win64_screen_buffer *ZBuffer,
                 int Width, int Height)
{
    // TODO(casey): Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.

    if (Buffer->Memory)
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    if (ZBuffer->Memory)
        VirtualFree(ZBuffer->Memory, 0, MEM_RELEASE);

    Buffer->Width = Width;
    Buffer->Height = Height;
    ZBuffer->Width = Width;
    ZBuffer->Height = Height;

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
    ZBuffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

static void
DrawPixel(float x, float y, int color)
{
    int index = ((int)x + (int)y * ScreenBuffer.Pitch / 4);
    uint32 *pixel = (uint32 *)ScreenBuffer.Memory;
    uint32 Blue = color & 0x0000ff;
    uint32 Green = color & 0x00ff00;
    uint32 Red = color & 0xff0000;
    // pixel[index] = ((Red << 16) | (Green << 8) | Blue);
    pixel[index] = Red | Green | Blue;
}

void ClearScreenBuffer(win64_screen_buffer *Buffer)
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

void InitZBUffer(win64_screen_buffer *Buffer)
{
    float *Idx = (float *)Buffer->Memory;
    for (int i = 0; i < Buffer->Height * Buffer->Width; ++i)
    {
        *Idx = INFINITY;
        Idx++;
    }
}

bool IsInFront(win64_screen_buffer *ZBuffer, win64_screen_buffer *Buffer,
               Shape *triangle_points, float focal_distance,
               Point *target_p)
{
    Point screen_p1 = *(((Point *)(triangle_points->geometry)) + 0);
    Point screen_p2 = *(((Point *)(triangle_points->geometry)) + 1);
    Point screen_p3 = *(((Point *)(triangle_points->geometry)) + 2);

    float area = EdgeFunction(&screen_p1, &screen_p2, &screen_p3) / 2;
    float screen_w1 = EdgeFunction(&screen_p2, &screen_p3, target_p) / 2 / area;
    float screen_w2 = EdgeFunction(&screen_p1, &screen_p3, target_p) / 2 / area;
    float screen_w3 = EdgeFunction(&screen_p1, &screen_p2, target_p) / 2 / area;

    float interpolated_z = 1 / (screen_w1 / (screen_p1.z) + screen_w2 / (screen_p2.z) + screen_w3 / (screen_p3.z));

    float *memory = (float *)ZBuffer->Memory;
    if (*(AccessScreenBuffer((int)target_p->x, (int)target_p->y, memory, Buffer->Width)) > interpolated_z)
    {
        *(AccessScreenBuffer((int)target_p->x, (int)target_p->y, memory, Buffer->Width)) = interpolated_z;
        return true;
    }
    return false;
}

static void
FillScreenBuffer(win64_screen_buffer *Buffer, win64_screen_buffer *ZBuffer, game_input *UserInput)
{
    ClearScreenBuffer(Buffer);
    InitZBUffer(ZBuffer);
    // projections
    // http://www.songho.ca/opengl/gl_projectionmatrix.html

    /*
     * temporary variables
     */

    // static float distance;
    // distance -= 0.001;

    float camera_matrix[4][4];
    IdentityMatrix4x4((float *)camera_matrix);
    // Translate(0, 0, 0, (float *)camera_matrix);

    float view_matrix[4][4];
    MatrixInverse((float *)camera_matrix, 4, (float *)view_matrix);

    /*
     * temporary objects
     */
    Point triangle_front_points[3] = {{1, 1, 0, 1}, {1, -1, 0, 1}, {-1, 1, 0, 1}};
    Shape triangle_front = {(void *)&triangle_front_points, 3, 0xffd2da};
    ;

    Point triangle_back_points[3] = {{1, 1, -1, 1}, {1, -1, -1, 1}, {-1, -1, -1, 1}};
    Shape triangle_back = {(void *)&triangle_back_points, 3, 0xe1bd23};
    ;

    Shape *shapes[2];
    shapes[0] = &triangle_front;
    shapes[1] = &triangle_back;

    /*
     * image settings
     */
    float frame_width = 16;
    float frame_height = 9;
    float focal_distance = 1;

    for (int shape_index = 0; shape_index < MatrixRowSize(shapes); shape_index++)
    {
        Shape *shape = shapes[shape_index];
        Shape raster_points = {};
        Point tmp[3] = {};
        raster_points = {(void *)&tmp, 3, NULL};

        Shape image_points = {};
        Point tmp1[3] = {};
        image_points = {(void *)&tmp1, 3, NULL};

        for (int i = 0; i < 3; i++)
        {
            Point *vertex = ((Point *)(shape->geometry)) + i;
            float transform_matrix[4][4];
            IdentityMatrix4x4((float *)transform_matrix);

            // 1. convert local to world
            RotateXAxis(0, (float *)transform_matrix);
            RotateYAxis(theta, (float *)transform_matrix);
            RotateZAxis(0, (float *)transform_matrix);
            Translate(0, 0, -2, (float *)transform_matrix);

            // 2. convert to camera space
            MatrixMultiply(
                (float *)view_matrix, 4, 4,
                (float *)transform_matrix, 4, 4,
                (float *)transform_matrix);

            // 2.5 convert to image space
            MatrixMultiply((float *)transform_matrix, 4, 4,
                           (float *)vertex, 4, 1,
                           (float *)vertex);

            Point image_point = {};
            PointToImage(&image_point, vertex, focal_distance);
            *(((Point *)image_points.geometry) + i) = image_point;

            // 3. convert to NDC space
            Point ndc_point = {};
            ImageToNDC(&image_point, &ndc_point, frame_height, frame_width);
            if ((ndc_point.x > 1) || (ndc_point.y > 1) || (vertex->z >= 0) || (ndc_point.x < 0) || (ndc_point.y < 0))
                continue;

            // 5. convert to rasterspace
            Point raster_point;
            NDCToRaster(&ndc_point, &raster_point, Buffer->Height, Buffer->Width);
            raster_point.z = vertex->z;

            *(((Point *)raster_points.geometry) + i) = raster_point;
            // 6. draw
            DrawPixel(raster_point.x, raster_point.y, (int)0x000000);
        }

        if (!fill)
            continue;

        Point bounding_top_left = {NULL};
        Point bounding_bot_right = {NULL};
        FindTriangleBoudningBox(&raster_points, &bounding_top_left, &bounding_bot_right);

        for (int row = (int)bounding_top_left.y; row <= (int)bounding_bot_right.y; row++)
        {
            for (int col = (int)bounding_top_left.x; col <= (int)bounding_bot_right.x; col++)
            {
                Point cur_point = {(float)col, (float)row, 0};
                if (ContainedInTriangle(&raster_points, &cur_point) &&
                    IsInFront(ZBuffer, Buffer, &raster_points, focal_distance, &cur_point))
                {
                    DrawPixel(cur_point.x, cur_point.y, shape->color_hex);
                }
            }
        }
    }
}

static win64_windows_dimension
GetWindowDimension(HWND Window)
{
    win64_windows_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return (Result);
}

static void DisplayBufferInWindow(win64_screen_buffer *ScreenBuffer,
                                  HDC DeviceContext,
                                  win64_windows_dimension *WindowDimension)
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

void UpdateAndRender(win64_screen_buffer *ScreenBuffer, win64_screen_buffer *ZBuffer, game_input *UserInput)
{
    FillScreenBuffer(ScreenBuffer, ZBuffer, UserInput);
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

    case WM_MOUSEMOVE:
    {
        if (UserInput.Mouse.LBDown)
        {
            int mouseY = (LParam & 0xffff0000) >> 16;
            int mouseX = (LParam & 0xffff);
            int MouseXMoveDistance = mouseX - UserInput.Mouse.X;
            if (MouseXMoveDistance > 0)
            {
                theta += 0.01 * PI;
            }
            else if (MouseXMoveDistance < 0)
            {
                theta += -0.01 * PI;
            }
        }
        // char str[256];
        // sprintf_s(str, sizeof(str), "X: %d, Y: %d\n", mouseX, mouseY);
        // OutputDebugStringA(str);
    }
    break;

    case WM_LBUTTONDOWN:
    {
        UserInput.Mouse.LBDown = true;
        UserInput.Mouse.X = (LParam & 0xffff);
        UserInput.Mouse.Y = (LParam & 0xffff0000) >> 16;
        // OutputDebugStringA("mouse L clicks\n");
    }
    break;
    case WM_LBUTTONUP:
    {
        UserInput.Mouse.LBDown = false;
        // OutputDebugStringA("mouse L clicks\n");
    }
    break;
    case WM_RBUTTONDOWN:
    {
        OutputDebugStringA("mouse R clicks\n");
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
            ResizeDIBSection(&ScreenBuffer, &ZBuffer, ResolutionInitialHeight, ResolutionInitialWidth);

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

                UpdateAndRender(&ScreenBuffer, &ZBuffer, &UserInput);

                win64_windows_dimension Dimension = GetWindowDimension(Window);
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
