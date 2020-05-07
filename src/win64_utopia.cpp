
#include  <windows.h>
#include <stdint.h>

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

// TODO: This is a global for now.
// static keyword initializes variable to zero
global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable   int BytesPerPixel = 4;

internal void
RenderWeirdGradient(int XOffset, int YOffset){
  int Width = BitmapWidth;
  int Height = BitmapHeight;
  
  int Pitch = Width*BytesPerPixel;
  // c will move ptr by the size of the type it's being pointed to.
  // for unit8, it will move by 1
  uint8 *Row = (uint8 *) BitmapMemory;
  for (int Y = 0; Y < BitmapHeight; ++Y){
    uint32 *Pixel = (uint32 *)Row;
    for (int X = 0; X < BitmapWidth; ++X){
      /* 
	 Pixel in memory: BB GG RR XX
	 Register:        xx RR GG BB
	 LITTLE ENDIAN ARCHITECTURE
       */
      uint8 Blue = (X + XOffset);
      uint8 Green = (Y + YOffset);
      uint8 Red = 0;
      *Pixel ++ = ((Green << 8) | Blue);
      
    }
    Row += Pitch;
  } 
}

// device independent bitmask
// build buffer using windows to give us buffer to draw into
internal void
Win64ResizeDIBSection(int Width, int Height){
  // TODO: bullet proof this
  // Maybe don't free first, free after, then free first if that fails

  if (BitmapMemory){
    VirtualFree(BitmapMemory, 0, MEM_RELEASE);
  }

  BitmapWidth = Width;
  BitmapHeight = Height;
  
  BITMAPINFOHEADER BitmapInfoHeader;
  BitmapInfoHeader.biSize = sizeof(BitmapInfoHeader);
  BitmapInfoHeader.biWidth = BitmapWidth;
  BitmapInfoHeader.biHeight = -BitmapHeight;
  BitmapInfoHeader.biPlanes = 1;
  BitmapInfoHeader.biBitCount = 32;
  BitmapInfoHeader.biCompression = BI_RGB;
  
  BitmapInfo.bmiHeader = BitmapInfoHeader;


  int BitmapMemorySize =(BitmapWidth * BitmapHeight) * BytesPerPixel;
  BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

}

internal void
Win64UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height){
  int WindowWidth = ClientRect->right - ClientRect->left;
  int WindowHeight = ClientRect->bottom - ClientRect->top;
  /*
    
   */
  StretchDIBits(DeviceContext,
		0, 0, BitmapWidth, BitmapHeight,
		0, 0, WindowWidth, WindowHeight,
		BitmapMemory,
		&BitmapInfo,
		DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
Win64MainWindowCallback(_In_ HWND   Window,
			_In_ UINT   Message,
			_In_ WPARAM WParam,
			_In_ LPARAM LParam){
  LRESULT Result = 0;
  switch(Message){
  case WM_SIZE:
    {
      RECT ClientRect;
      GetClientRect(Window, &ClientRect);
      int Width = ClientRect.right - ClientRect.left;
      int Height = ClientRect.bottom - ClientRect.top;
      Win64ResizeDIBSection(Width, Height);
    } break;
  case WM_CLOSE:
    {
      // TOOD: handle this iwth a message to the user
      Running = false;
    }break;
  case WM_DESTROY:
    {
      // TODO: handle this as an error.
      Running = false;
    }break;
  case WM_ACTIVATEAPP:
    {
      OutputDebugString("WM_ACTIVATEAPP\n");
    }break;
  case WM_PAINT:
    {
      PAINTSTRUCT Paint;
      HDC DeviceContext = BeginPaint(Window, &Paint);

      int X = Paint.rcPaint.left;
      int Y = Paint.rcPaint.top;
      int Width= Paint.rcPaint.right - Paint.rcPaint.left;
      int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

      RECT ClientRect;
      GetClientRect(Window, &ClientRect);
      Win64UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
      
      EndPaint(Window, &Paint);
    }break;
  default:
    {
      Result = DefWindowProc(Window, Message, WParam, LParam);
    }break;
  }
  return(Result);
}

int
WinMain(HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR     CommandLine,
	int       ShowCommand) {
  // when the struct is intialized as {}, all the values are set to 0
  WNDCLASSEX  WindowClass = {}; 
  WindowClass.cbSize = sizeof(WNDCLASSEXW);
  WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
  WindowClass.lpfnWndProc = Win64MainWindowCallback;
  WindowClass.hInstance = Instance;
  //  HICON     hIcon;
  WindowClass.lpszClassName = "Utopia";


  
  if(RegisterClassEx(&WindowClass)){
    HWND Window = CreateWindowEx(0, WindowClass.lpszClassName, "Utopia",
				       WS_OVERLAPPEDWINDOW|WS_VISIBLE,
				       CW_USEDEFAULT, CW_USEDEFAULT,
				       CW_USEDEFAULT, CW_USEDEFAULT,
				       0, 0, Instance, 0);
    if (Window) {
      Running = true;

      int XOffset = 0;
      int YOffset = 0;
      while (Running) {
	MSG Message;

	while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)){
	  if (Message.message == WM_QUIT) {
	    Running = false;
	  }
	  TranslateMessage(&Message);
	  DispatchMessage(&Message);
	}
	
	RenderWeirdGradient(XOffset, YOffset);

	HDC DeviceContext = GetDC(Window);
	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	int WindowWidth = ClientRect.right - ClientRect.left;
	int WindowHeight = ClientRect.bottom - ClientRect.top;
	Win64UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
	ReleaseDC(Window, DeviceContext);
	
	XOffset ++;
      }

    } else {
      // TODO: logging
    }
  } else {
    // TODO: logging
  }
	  
  return(0);
}
