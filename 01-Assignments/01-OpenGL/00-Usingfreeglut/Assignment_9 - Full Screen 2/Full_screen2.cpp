//headers
#include<windows.h>

//global fuction declartions
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


bool bFullscreen = false;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
HWND ghwnd = NULL;


//WinMain()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//variable declarations

	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("AMD_Application");

	//code
	//initialization of WNDCLASSEX

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	//register above class
	RegisterClassEx(&wndclass);

	//create window
	hwnd = CreateWindow(szAppName, TEXT("My Window - Aishwarya Dekhane"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	ghwnd = hwnd;

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	//message loop
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void ToggleFullscreen(void);

	//code
	switch (iMsg)
	{

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 0x46:                //ASCII value of 'F' or 'f'
			ToggleFullscreen();
			break;

		default:
			break;
		}
		break;


	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}



void ToggleFullscreen(void)
{
		// turn off window region without redraw
		SetWindowRgn(ghwnd, 0, false);

		DEVMODE dobj;

		// request current screen settings
		EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &dobj);

		//specify which aspects of the screen settings to change 
		dobj.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

		//  set desired screen size/res	
		dobj.dmPelsWidth = 800;
		dobj.dmPelsHeight = 600;
		
		if (bFullscreen == false)
		{
			long result = ChangeDisplaySettings(&dobj, CDS_FULLSCREEN);

			//MessageBox(ghwnd, TEXT("This is if"), TEXT("My Message"), MB_OK);
			bFullscreen = true;
		}

		else
		{
			//MessageBox(ghwnd, TEXT("This is else"), TEXT("My Message"), MB_OK);

			long result = ChangeDisplaySettings(NULL, 0);

			dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
			
			SetWindowLong(ghwnd, GWL_STYLE, dwStyle);
		
			// move the window to (0,0)
			SetWindowPos(ghwnd, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			InvalidateRect(ghwnd, 0, true);

			bFullscreen = false;
		}
}



	