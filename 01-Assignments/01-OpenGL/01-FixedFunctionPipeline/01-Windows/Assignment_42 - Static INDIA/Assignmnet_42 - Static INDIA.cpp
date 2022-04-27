#include <windows.h>
#include <gl/GL.h>
#include<gl/GLU.h>
#include<stdio.h>

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
FILE *gpFile = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbActiveWindow = false;

bool gbFullscreen = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//fuction declaration
	int initialize(void);
	void display(void);


	//variable declaration
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szClassName[] = TEXT("MyApplication");
	bool bDone = false;
	int iRet = 0;


	if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Log file can not be created"), TEXT("Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log file is successfully created\n\n");
	}

	//code
	//initialization of WNDCLASSX

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szClassName;
	wndclass.lpszMenuName = NULL;

	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szClassName,
		TEXT("OpenGL Native Windowing"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		100,
		100,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	iRet = initialize();

	if (iRet == -1)
	{
		fprintf(gpFile, "ChoosePixelFormat failed\n\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -2)
	{
		fprintf(gpFile, "SetPixelFormat failed\n\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -3)
	{
		fprintf(gpFile, "wglCreateContext failed\n\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -4)
	{
		fprintf(gpFile, "wglMakeCurrent failed\n\n");
		DestroyWindow(hwnd);
	}
	else
	{
		fprintf(gpFile, "Initialization Suceeded\n\n");
	}


	ShowWindow(hwnd, SW_SHOW);

	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//Message Loop = Game loop
	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == true)
			{
				//here call update
			}
			else
			{
				display();
			}
		}
	}

	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//void display(void);
	void resize(int, int);
	void ToggleFullscreen(void);
	void uninitialize(void);

	switch (iMsg)
	{
	case WM_ERASEBKGND:
		return(0);

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 0x46: //for 'f' or 'F'
			if (gbFullscreen == false)
			{
				ToggleFullscreen();
				gbFullscreen = true;
			}
			else
			{
				ToggleFullscreen();
				gbFullscreen = false;
			}
			break;

		default:
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		uninitialize();
		PostQuitMessage(0);
		break;

	default:
		break;

	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen(void)
{
	MONITORINFO mi;

	if (gbFullscreen == false)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
	}

	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
	}
}

int initialize(void)
{
	//function declaration
	void resize(int, int);


	//variable declaration
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;

	ghdc = GetDC(ghwnd);

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
	{
		return -1;
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		return -2;
	}

	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
	{
		return -3;
	}

	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		return -4;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	resize(WIN_WIDTH, WIN_HEIGHT);

	return 0;
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.0f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.56f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.56f, -0.5f, 0.0f);

	//I horizontal lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.65f, 0.5f, 0.0f);

	glVertex3f(-0.47f, 0.5f, 0.0f);

	//I horizontal lines
	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.65f, -0.5f, 0.0f);

	glVertex3f(-0.47f, -0.5f, 0.0f);

	glEnd();


//--------------------------------------------------------------------------------------------------------------
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.0f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//Letter N
	//N vertical lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.37f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.37f, -0.5f, 0.0f);



	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.19f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.19f, -0.5f, 0.0f);


	//N diagonal line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.37f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.19f, -0.5f, 0.0f);


	glEnd();

	//----------------------------------------------------------------------------------------------------------------

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.0f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//Letter D
	//D vertical lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.09f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.09f, -0.5f, 0.0f);


	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.09f, 0.4f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.09f, -0.4f, 0.0f);


	//D horizontal lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.09f, 0.5f, 0.0f);

	glVertex3f(0.04f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.09f, -0.5f, 0.0f);

	glVertex3f(0.04f, -0.5f, 0.0f);


	//D diagonal lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.04f, 0.5f, 0.0f);

	glVertex3f(0.09f, 0.4f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);
	
	glVertex3f(0.04f, -0.5f, 0.0f);

	glVertex3f(0.09f, -0.4f, 0.0f);


	glEnd();

	//------------------------------------------------------------------------------------------------------------------
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.0f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.28f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.28f, -0.5f, 0.0f);

	//I horizontal lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.19f, 0.5f, 0.0f);

	glVertex3f(0.37f, 0.5f, 0.0f);

	//I horizontal lines
	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.19f, -0.5f, 0.0f);

	glVertex3f(0.37f, -0.5f, 0.0f);

	glEnd();


	//--------------------------------------------------------------------------------------------------------------
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.0f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//Letter A
	//A diagonal line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.56f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.47f, -0.5f, 0.0f);


	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.56f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.65f, -0.5f, 0.0f);

	//A horizontal lines -white
	glColor3f(1.0f, 1.0f, 1.0f);

	glVertex3f(0.52f, 0.0f, 0.0f);

	glVertex3f(0.60f, 0.0f, 0.0f);

	//A horizontal lines -saffron
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.525f, 0.05f, 0.0f);

	glVertex3f(0.595f, 0.05f, 0.0f);

	//A horizontal lines -green
	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.51f, -0.05f, 0.0f);

	glVertex3f(0.61f, -0.05f, 0.0f);

	glEnd();


	//--------------------------------------------------------------------------------------------------------------
	SwapBuffers(ghdc);
}

void resize(int width, int height)
{
	if (height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, ((GLfloat)width / (GLfloat)height), 0.1f, 100.0f);
}

void uninitialize(void)
{

	if (gbFullscreen == true)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);

	}
	if (wglGetCurrentContext() == ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}

	if (ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (gpFile)
	{
		fprintf(gpFile, "Log file is cloased successfully \n\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}
