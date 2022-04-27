#include <windows.h>
#include <gl/GL.h>
#include<gl/GLU.h>
#include<stdio.h>

//#define _USE_MATH_DEFINE 1

#include<math.h>

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
FILE *gpFile = NULL;

GLfloat angle_Tri = 0.0f;
GLfloat angle_Square = 0.0f;

GLfloat angle = 0.0f;

GLfloat Identity_matrix[16];
GLfloat Translation_matrix[16];
GLfloat Translation_matrix1[16];
GLfloat Rotation_matrix[16];
GLfloat Rotation_matrix1[16];

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbActiveWindow = false;

bool gbFullscreen = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//fuction declaration
	int initialize(void);
	void display(void);
	void spin(void);


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
				//spin();
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

void spin(void)
{
	angle_Tri = angle_Tri + 0.05f;
	if (angle_Tri >= 360.0f)
		angle_Tri = angle_Tri - 360.0f;

	angle_Square = angle_Square + 0.05f;
	if (angle_Square >= 360.0f)
		angle_Square = angle_Square - 360.0f;

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

	//Identity matrix initialization
	Identity_matrix[0] = 1.0f;
	Identity_matrix[1] = 0.0f;
	Identity_matrix[2] = 0.0f;
	Identity_matrix[3] = 0.0f;
	Identity_matrix[4] = 0.0f;
	Identity_matrix[5] = 1.0f;
	Identity_matrix[6] = 0.0f;
	Identity_matrix[7] = 0.0f;
	Identity_matrix[8] = 0.0f;
	Identity_matrix[9] = 0.0f;
	Identity_matrix[10] = 1.0f;
	Identity_matrix[11] = 0.0f;
	Identity_matrix[12] = 0.0f;
	Identity_matrix[13] = 0.0f;
	Identity_matrix[14] = 0.0f;
	Identity_matrix[15] = 1.0f;

	//Translation matrix initialization

	Translation_matrix[0] = 1.0f;
	Translation_matrix[1] = 0.0f;
	Translation_matrix[2] = 0.0f;
	Translation_matrix[3] = 0.0f;
	Translation_matrix[4] = 0.0f;
	Translation_matrix[5] = 1.0f;
	Translation_matrix[6] = 0.0f;
	Translation_matrix[7] = 0.0f;
	Translation_matrix[8] = 0.0f;
	Translation_matrix[9] = 0.0f;
	Translation_matrix[10] = 1.0f;
	Translation_matrix[11] = 0.0f;
	Translation_matrix[12] = -1.5f;
	Translation_matrix[13] = 0.0f;
	Translation_matrix[14] = -6.0f;
	Translation_matrix[15] = 1.0f;

	//Translation matrix initialization for rect

	Translation_matrix1[0] = 1.0f;
	Translation_matrix1[1] = 0.0f;
	Translation_matrix1[2] = 0.0f;
	Translation_matrix1[3] = 0.0f;
	Translation_matrix1[4] = 0.0f;
	Translation_matrix1[5] = 1.0f;
	Translation_matrix1[6] = 0.0f;
	Translation_matrix1[7] = 0.0f;
	Translation_matrix1[8] = 0.0f;
	Translation_matrix1[9] = 0.0f;
	Translation_matrix1[10] = 1.0f;
	Translation_matrix1[11] = 0.0f;
	Translation_matrix1[12] = 1.5f;
	Translation_matrix1[13] = 0.0f;
	Translation_matrix1[14] = -6.0f;
	Translation_matrix1[15] = 1.0f;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	resize(WIN_WIDTH, WIN_HEIGHT);

	return 0;
}

void display(void)
{
	//static GLfloat angle = 0.0f;

	glClear(GL_COLOR_BUFFER_BIT);


	//Triangle
	glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	glLoadMatrixf(Identity_matrix);



	//glTranslatef(-1.5f, 0.0f, -6.0f);
	glMultMatrixf(Translation_matrix);


	Rotation_matrix[0] = cos(angle);
	Rotation_matrix[1] = 0.0f;
	Rotation_matrix[2] = -sin(angle);
	Rotation_matrix[3] = 0.0f;
	Rotation_matrix[4] = 0.0f;
	Rotation_matrix[5] = 1.0f;
	Rotation_matrix[6] = 0.0f;
	Rotation_matrix[7] = 0.0f;
	Rotation_matrix[8] = sin(angle);
	Rotation_matrix[9] = 0.0f;
	Rotation_matrix[10] = cos(angle);
	Rotation_matrix[11] = 0.0f;
	Rotation_matrix[12] = 0.0f;
	Rotation_matrix[13] = 0.0f;
	Rotation_matrix[14] = 0.0f;
	Rotation_matrix[15] = 1.0f;

	//glRotatef(angle_Tri, 0.0f, 1.0f, 0.0f);
	glMultMatrixf(Rotation_matrix);

	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(0.0f, 1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(-1.0f, -1.0f);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2f(1.0f, -1.0f);
	glEnd();








	//Rectangle
	glMatrixMode(GL_MODELVIEW);

	//glLoadIdentity();
	glLoadMatrixf(Identity_matrix);


	//glTranslatef(1.5f, 0.0f, -6.0f);
	glMultMatrixf(Translation_matrix1);

	Rotation_matrix1[0] = 1.0f;
	Rotation_matrix1[1] = 0.0f;
	Rotation_matrix1[2] = 0.0f;
	Rotation_matrix1[3] = 0.0f;
	Rotation_matrix1[4] = 0.0f;
	Rotation_matrix1[5] = cos(angle);
	Rotation_matrix1[6] = sin(angle);
	Rotation_matrix1[7] = 0.0f;
	Rotation_matrix1[8] = 0.0f;
	Rotation_matrix1[9] = -sin(angle);
	Rotation_matrix1[10] = cos(angle);
	Rotation_matrix1[11] = 0.0f;
	Rotation_matrix1[12] = 0.0f;
	Rotation_matrix1[13] = 0.0f;
	Rotation_matrix1[14] = 0.0f;
	Rotation_matrix1[15] = 1.0f;


	//glRotatef(angle_Square, 1.0f, 0.0f, 0.0f);
	glMultMatrixf(Rotation_matrix1);

	glBegin(GL_QUADS);

	glColor3f(128.0f, 0.0f, 128.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);

	glColor3f(128.0f, 0.0f, 128.0f);
	glVertex3f(-1.0f, 1.0f, 0.0f);

	glColor3f(0.0f, 255.0f, 255.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);

	glColor3f(0.0f, 255.0f, 255.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);
	glEnd();

	//glFlush();
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
