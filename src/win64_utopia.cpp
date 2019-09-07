
#include  <windows.h>

LRESULT CALLBACK MainWindowCallback(_In_ HWND   Window,
				    _In_ UINT   Message,
				    _In_ WPARAM WParam,
				    _In_ LPARAM LParam){
  LRESULT Result = 0;
  switch(Message){
  case WM_SIZE:
    {
      OutputDebugString("WM_SIZE\n");
    } break;
  case WM_DESTROY:
    {
      OutputDebugString("WM_DESTROY\n");
    }break;
  case WM_CLOSE:
    {
      OutputDebugString("WM_CLOSE\n");
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
      static DWORD Operation = WHITENESS;
      PatBlt(DeviceContext, X, Y, Width, Height, Operation);
      if (Operation == WHITENESS){
	Operation = BLACKNESS;
      } else {
	Operation = WHITENESS;
      }
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
	int       ShowCode){
  // when the struct is intialized as {}, all the values are set to 0
  WNDCLASSEX  WindowClass = {}; 
  WindowClass.cbSize = sizeof(WNDCLASSEXW);
  WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
  WindowClass.lpfnWndProc = MainWindowCallback;
  WindowClass.hInstance = Instance;
  //  HICON     hIcon;
  WindowClass.lpszClassName = "Utopia";

  if(RegisterClassEx(&WindowClass)){
    HWND WindowHandle = CreateWindowEx(0,
				       WindowClass.lpszClassName,
				       "Utopia",
				       WS_OVERLAPPEDWINDOW|WS_VISIBLE,
				       CW_USEDEFAULT,
				       CW_USEDEFAULT,
				       CW_USEDEFAULT,
				       CW_USEDEFAULT,
				       0,
				       0, 
				       Instance,
				       0);
    if (WindowHandle) {
      MSG Message;
      for (;;){
	BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
	if (MessageResult > 0){
	  TranslateMessage(&Message);
	  DispatchMessage(&Message);
	} else {
	  break;
	}
      }

    } else {
      
    }
  } else {
    
  }
  
  return(0);
}
