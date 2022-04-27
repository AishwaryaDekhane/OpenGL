#include "Header.h"
#include <windows.h>
#include <gl/GL.h>
#include<gl/GLU.h>
#include<stdio.h>
#include<math.h>
#include<Mmsystem.h> //for PlaySound()

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"Winmm.lib")  //For PlaySound()

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define PI 3.142f

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
FILE *gpFile = NULL;
GLfloat angle = 0.0f;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbActiveWindow = false;

bool gbFullscreen = false;

static float i1x = -1.0f;
static float i1y = 0.0f;        //for letter I1

static float nx = -0.28f;
static float ny = 2.4f;         //for letter N


//saffron color     
static float s1 = -3.1f;			//for letter D
static float s2 = -3.1f;
static float s3 = -3.1f;

//green color
static float g1 = -3.1f;
static float g2 = -3.1f;
static float g3 = -3.1f;


static float i2x = 0.28f;		//for letter I2
static float i2y = -3.1f;

static float ax = 1.7f;
static float ay = 0.0f;         //for letter A

static float p1x = -3.5f;       //translation of plane 
static float p2y = -3.5f;
static float p2x = -3.5f;
static float p3x = -3.5f;
static float p3y = -3.5f;

//saffron color     
static float mfs1 = 23.0f;			//for mid plane flag saffron 
static float mfs2 = 23.0f;
static float mfs3 = 23.0f;

//saffron color
static float As1 = -4.8f;			//for letter A
static float As2 = -4.8f;
static float As3 = -4.8f;

//white color
static float Aw1 = -4.8f;
static float Aw2 = -4.8f;
static float Aw3 = -4.8f;

//green color
static float Ag1 = -4.8f;
static float Ag2 = -4.8f;
static float Ag3 = -4.8f;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//fuction declaration
	int initialize(void);
	void display(void);
	void ToggleFullscreen(void);


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

	ToggleFullscreen();

	PlaySound(MAKEINTRESOURCE(My_Song), NULL, SND_RESOURCE | SND_ASYNC);

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

	case WM_CREATE:
		ToggleFullscreen();
		break;

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		/*case 0x46: //for 'f' or 'F'
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
			break;*/

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

//#################################################################################################################

void LetterOnMiddlePlaneI(void)
{
	glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.93f, 0.008f, 0.0f);

	glVertex3f(-0.93f, -0.008f, 0.0f);

	//I horizontal lines

	glVertex3f(-0.92f, 0.008f, 0.0f);

	glVertex3f(-0.94f, 0.008f, 0.0f);

	//I horizontal lines
	glVertex3f(-0.92f, -0.008f, 0.0f);

	glVertex3f(-0.94f, -0.008f, 0.0f);

	glEnd();
}

void LetterOnMiddlePlaneA(void)
{
	glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter A
	//A diagonal line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.90f, 0.0099f, 0.0f);

	glVertex3f(-0.91f, -0.009f, 0.0f);

	glVertex3f(-0.90f, 0.0099f, 0.0f);

	glVertex3f(-0.89f, -0.009f, 0.0f);

	//A horizontal line
	glVertex3f(-0.908f, 0.001f, 0.0f);

	glVertex3f(-0.895f, 0.001f, 0.0f);


	glEnd();

}

void LetterOnMiddlePlaneF(void)
{
	glLineWidth(3.0f);

	glBegin(GL_LINES);
	//Letter F
	//F vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.88f, 0.008f, 0.0f);

	glVertex3f(-0.88f, -0.008f, 0.0f);

	//horizontal lines
	glVertex3f(-0.88f, 0.008f, 0.0f);

	glVertex3f(-0.86f, 0.008f, 0.0f);

	glVertex3f(-0.88f, -0.001f, 0.0f);

	glVertex3f(-0.86f, -0.001f, 0.0f);

	glEnd();
}


//##################################################################################################################

void LetterOnTopPlaneI(void)
{
	/*glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.93f, 0.499f, 0.0f);

	glVertex3f(-0.93f, 0.481f, 0.0f);

	//I horizontal lines

	glVertex3f(-0.92f, 0.499f, 0.0f);

	glVertex3f(-0.94f, 0.499f, 0.0f);

	//I horizontal lines
	glVertex3f(-0.92f, 0.481f, 0.0f);

	glVertex3f(-0.94f, 0.481f, 0.0f);

	glEnd();*/

	glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.93f, 0.43f, 0.0f);

	glVertex3f(-0.93f, 0.412f, 0.0f);

	//I horizontal lines

	glVertex3f(-0.92f, 0.43f, 0.0f);

	glVertex3f(-0.94f, 0.43f, 0.0f);

	//I horizontal lines
	glVertex3f(-0.92f, 0.412f, 0.0f);

	glVertex3f(-0.94f, 0.412f, 0.0f);

	glEnd();
}

void LetterOnTopPlaneA(void)
{
	/*glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter A
	//A diagonal line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.90f, 0.499f, 0.0f);

	glVertex3f(-0.91f, 0.481f, 0.0f);

	glVertex3f(-0.90f, 0.499f, 0.0f);

	glVertex3f(-0.89f, 0.481f, 0.0f);

	//A horizontal line
	glVertex3f(-0.908f, 0.49f, 0.0f);

	glVertex3f(-0.895f, 0.49f, 0.0f);


	glEnd();*/

	glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter A
	//A diagonal line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.90f, 0.43f, 0.0f);

	glVertex3f(-0.91f, 0.412f, 0.0f);

	glVertex3f(-0.90f, 0.43f, 0.0f);

	glVertex3f(-0.89f, 0.412f, 0.0f);

	//A horizontal line
	glVertex3f(-0.908f, 0.42f, 0.0f);

	glVertex3f(-0.895f, 0.42f, 0.0f);


	glEnd();

}

void LetterOnTopPlaneF(void)
{
	/*glLineWidth(3.0f);

	glBegin(GL_LINES);
	//Letter F
	//F vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.88f, 0.499f, 0.0f);

	glVertex3f(-0.88f, 0.481f, 0.0f);

	//horizontal lines
	glVertex3f(-0.88f, 0.499f, 0.0f);

	glVertex3f(-0.86f, 0.499f, 0.0f);

	glVertex3f(-0.88f, 0.49f, 0.0f);

	glVertex3f(-0.86f, 0.49f, 0.0f);

	glEnd();*/

	glLineWidth(3.0f);

	glBegin(GL_LINES);
	//Letter F
	//F vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.88f, 0.43f, 0.0f);

	glVertex3f(-0.88f, 0.412f, 0.0f);

	//horizontal lines
	glVertex3f(-0.88f, 0.43f, 0.0f);

	glVertex3f(-0.86f, 0.43f, 0.0f);

	glVertex3f(-0.88f, 0.42f, 0.0f);

	glVertex3f(-0.86f, 0.42f, 0.0f);

	glEnd();
}

//##################################################################################################################

void LetterOnBottomPlaneI(void)
{
	/*glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.93f, -0.499f, 0.0f);

	glVertex3f(-0.93f, -0.481f, 0.0f);

	//I horizontal lines

	glVertex3f(-0.92f, -0.499f, 0.0f);

	glVertex3f(-0.94f, -0.499f, 0.0f);

	//I horizontal lines
	glVertex3f(-0.92f, -0.481f, 0.0f);

	glVertex3f(-0.94f, -0.481f, 0.0f);

	glEnd();*/

	glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.93f, -0.43f, 0.0f);

	glVertex3f(-0.93f, -0.412f, 0.0f);

	//I horizontal lines

	glVertex3f(-0.92f, -0.43f, 0.0f);

	glVertex3f(-0.94f, -0.43f, 0.0f);

	//I horizontal lines
	glVertex3f(-0.92f, -0.412f, 0.0f);

	glVertex3f(-0.94f, -0.412f, 0.0f);

	glEnd();
}

void LetterOnBottomPlaneA(void)
{
	/*glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter A
	//A diagonal line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.90f, -0.481f, 0.0f);

	glVertex3f(-0.91f, - 0.499f, 0.0f);

	glVertex3f(-0.90f, -0.481f, 0.0f);

	glVertex3f(-0.89f, - 0.499f, 0.0f);

	//A horizontal line
	glVertex3f(-0.908f, -0.49f, 0.0f);

	glVertex3f(-0.895f, -0.49f, 0.0f);


	glEnd();*/

	glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter A
	//A diagonal line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.90f, -0.412f, 0.0f);

	glVertex3f(-0.91f, -0.43f, 0.0f);

	glVertex3f(-0.90f, -0.412f, 0.0f);

	glVertex3f(-0.89f, -0.43f, 0.0f);

	//A horizontal line
	glVertex3f(-0.908f, -0.42f, 0.0f);

	glVertex3f(-0.895f, -0.42f, 0.0f);


	glEnd();

}

void LetterOnBottomPlaneF(void)
{
	/*glLineWidth(3.0f);

	glBegin(GL_LINES);
	//Letter F
	//F vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.88f, -0.499f, 0.0f);

	glVertex3f(-0.88f, -0.481f, 0.0f);

	//horizontal lines
	glVertex3f(-0.88f, -0.488f, 0.0f);

	glVertex3f(-0.86f, -0.488f, 0.0f);

	glVertex3f(-0.88f, -0.49f, 0.0f);

	glVertex3f(-0.86f, -0.49f, 0.0f);

	glEnd();*/

	glLineWidth(3.0f);

	glBegin(GL_LINES);
	//Letter F
	//F vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.88f, -0.43f, 0.0f);

	glVertex3f(-0.88f, -0.412f, 0.0f);

	//horizontal lines
	glVertex3f(-0.88f, -0.412f, 0.0f);

	glVertex3f(-0.86f, -0.412f, 0.0f);

	glVertex3f(-0.88f, -0.42f, 0.0f);

	glVertex3f(-0.86f, -0.42f, 0.0f);

	glEnd();
}

//################################################################################################################

void DrawTopPlane(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//GLfloat fxTrans = -4.0f, fytrans = 2.3f, angle = -90.0f;
	/*angle = angle + 0.001f;

	if (angle < 2.0f*PI*0.75)
	{
		glTranslatef(cos(angle) +0.5, sin(angle) + 0.6, -1.5f);
	}*/

	//glTranslatef(p2x, -p2y, -1.5f);
	//glRotatef(angle, 0.0f, 0.0f, 1.0f);
	//glTranslatef(p2x, -0.1f, -1.5f);

	/*glLineWidth(3.0f);

	glBegin(GL_POINTS);


	glColor3f(1.0f, 0.0f, 0.0f); //red

	for (GLfloat angle = 0.0f;angle < 2.0f*PI*0.25;angle = angle + 0.001f)
	{
		glVertex3f(cos(angle) - 1.9, sin(angle) +0.0, -1.5f);
	}

	glEnd();*/
//________________________________________________________________________________________________________
	/*glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, 0.48f, 0.0f);

	glVertex3f(-0.96f, 0.5f, 0.0f);

	glVertex3f(-0.85f, 0.5f, 0.0f);

	glVertex3f(-0.85f, 0.48f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//lower wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.91f, 0.48f, 0.0f);

	glVertex3f(-0.93f, 0.44f, 0.0f);

	glVertex3f(-0.96f, 0.44f, 0.0f);

	glVertex3f(-0.88f, 0.48f, 0.0f);


	glEnd();


	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//upper wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.91f, 0.5f, 0.0f);

	glVertex3f(-0.93f, 0.54f, 0.0f);

	glVertex3f(-0.96f, 0.54f, 0.0f);

	glVertex3f(-0.88f, 0.5f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail up
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, 0.5f, 0.0f);

	glVertex3f(-0.99f, 0.52f, 0.0f);

	glVertex3f(-0.97f, 0.52f, 0.0f);

	glVertex3f(-0.94f, 0.5f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	


	//filled circle
	float tx1, ty1, tx2, ty2;
	float angle1;
	double radius1 = 0.01;

	tx1 = -0.85, ty1 = 0.49;

	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(tx1, ty1);

	for (angle1 = 1.0f;angle1<361.0f;angle1 += 0.2)
	{
		tx2 = tx1 + sin(angle1)*radius1;
		ty2 = ty1 + cos(angle1)*radius1;
		glVertex2f(tx2, ty2);
	}

	glEnd();*/



	glTranslatef(p2x, -p2y, -1.4f);

	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, 0.44f, 0.0f);

	glVertex3f(-0.96f, 0.40f, 0.0f);

	glVertex3f(-0.80f, 0.385f, 0.0f);

	glVertex3f(-0.80f, 0.455f, 0.0f);

	glEnd();

	//------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.80f, 0.385f, 0.0f);

	glVertex3f(-0.80f, 0.455f, 0.0f);

	glVertex3f(-0.75f, 0.430f, 0.0f);

	glVertex3f(-0.75f, 0.410f, 0.0f);

	glEnd();

	//-----------------------------------------------------------------------------------------------------
	//filled circle
	float x1, y1, x2, y2;
	float angle;
	double radius = 0.01;

	x1 = -0.75, y1 = 0.420;

	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x1, y1);

	for (angle = 1.0f;angle<361.0f;angle += 0.2)
	{
		x2 = x1 + sin(angle)*radius;
		y2 = y1 + cos(angle)*radius;
		glVertex2f(x2, y2);
	}

	glEnd();

	//---------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//upper wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.92f, 0.44f, 0.0f);

	glVertex3f(-0.92f, 0.50f, 0.0f);

	glVertex3f(-0.89f, 0.50f, 0.0f);

	glVertex3f(-0.84f, 0.44f, 0.0f);

	glEnd();

	//---------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//lower wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.92f, 0.40f, 0.0f);

	glVertex3f(-0.92f, 0.34f, 0.0f);

	glVertex3f(-0.89f, 0.34f, 0.0f);

	glVertex3f(-0.84f, 0.40f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail up
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.95f, 0.42f, 0.0f);

	glVertex3f(-0.97f, 0.42f, 0.0f);

	glVertex3f(-0.99f, 0.46f, 0.0f);

	glVertex3f(-0.97f, 0.46f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail down
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.95f, 0.42f, 0.0f);

	glVertex3f(-0.97f, 0.42f, 0.0f);

	glVertex3f(-0.99f, 0.38f, 0.0f);

	glVertex3f(-0.97f, 0.38f, 0.0f);

	glEnd();
}

void DrawMiddlePlane(void)
{
	//Middle Plane

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/*glTranslatef(p1x, 0.0f, -1.5f);

	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, 0.01f, 0.0f);

	glVertex3f(-0.96f, -0.01f, 0.0f);

	glVertex3f(-0.85f, -0.01f, 0.0f);

	glVertex3f(-0.85f, 0.01f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//lower wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.91f, -0.01f, 0.0f);

	glVertex3f(-0.93f, -0.05f, 0.0f);

	glVertex3f(-0.96f, -0.05f, 0.0f);

	glVertex3f(-0.88f, -0.01f, 0.0f);


	glEnd();


	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//upper wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.91f, 0.01f, 0.0f);

	glVertex3f(-0.93f, 0.05f, 0.0f);

	glVertex3f(-0.96f, 0.05f, 0.0f);

	glVertex3f(-0.88f, 0.01f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, 0.01f, 0.0f);

	glVertex3f(-0.99f, 0.03f, 0.0f);

	glVertex3f(-0.97f, 0.03f, 0.0f);

	glVertex3f(-0.94f, 0.01f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________

	//filled circle
	float x1, y1, x2, y2;
	float angle;
	double radius = 0.01;

	x1 = -0.85, y1 = 0.0;

	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x1, y1);

	for (angle = 1.0f;angle<361.0f;angle += 0.2)
	{
		x2 = x1 + sin(angle)*radius;
		y2 = y1 + cos(angle)*radius;
		glVertex2f(x2, y2);
	}

	glEnd();*/

	glTranslatef(p1x, 0.0f, -1.4f);

	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, 0.02f, 0.0f);

	glVertex3f(-0.96f, -0.02f, 0.0f);

	glVertex3f(-0.80f, -0.035f, 0.0f);

	glVertex3f(-0.80f, 0.035f, 0.0f);

	glEnd();

	//------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.80f, -0.035f, 0.0f);

	glVertex3f(-0.80f, 0.035f, 0.0f);

	glVertex3f(-0.75f, 0.01f, 0.0f);

	glVertex3f(-0.75f, -0.01f, 0.0f);

	glEnd();

	//-----------------------------------------------------------------------------------------------------
	//filled circle
	float x1, y1, x2, y2;
	float angle;
	double radius = 0.01;

	x1 = -0.75, y1 = 0.0;

	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x1, y1);

	for (angle = 1.0f;angle<361.0f;angle += 0.2)
	{
		x2 = x1 + sin(angle)*radius;
		y2 = y1 + cos(angle)*radius;
		glVertex2f(x2, y2);
	}

	glEnd();

	//---------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//upper wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.92f, 0.02f, 0.0f);

	glVertex3f(-0.92f, 0.09f, 0.0f);

	glVertex3f(-0.89f, 0.09f, 0.0f);

	glVertex3f(-0.84f, 0.02f, 0.0f);

	glEnd();

	//---------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//lower wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.92f, -0.02f, 0.0f);

	glVertex3f(-0.92f, -0.09f, 0.0f);

	glVertex3f(-0.89f, -0.09f, 0.0f);

	glVertex3f(-0.84f, -0.02f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail up
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.95f, 0.00f, 0.0f);

	glVertex3f(-0.97f, 0.00f, 0.0f);

	glVertex3f(-0.99f, 0.04f, 0.0f);

	glVertex3f(-0.97f, 0.04f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail down
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.95f, 0.00f, 0.0f);

	glVertex3f(-0.97f, 0.00f, 0.0f);

	glVertex3f(-0.99f, -0.04f, 0.0f);

	glVertex3f(-0.97f, -0.04f, 0.0f);

	glEnd();
}

void DrawBottomPlane(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/*glTranslatef(p3x, p3y, -1.5f);

	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, -0.48f, 0.0f);

	glVertex3f(-0.96f, -0.5f, 0.0f);

	glVertex3f(-0.85f, -0.5f, 0.0f);

	glVertex3f(-0.85f, -0.48f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//lower wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.91f, -0.48f, 0.0f);

	glVertex3f(-0.93f, -0.44f, 0.0f);

	glVertex3f(-0.96f, -0.44f, 0.0f);

	glVertex3f(-0.88f, -0.48f, 0.0f);


	glEnd();


	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//upper wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.91f, -0.5f, 0.0f);

	glVertex3f(-0.93f, -0.54f, 0.0f);

	glVertex3f(-0.96f, -0.54f, 0.0f);

	glVertex3f(-0.88f, -0.5f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, -0.48f, 0.0f);

	glVertex3f(-0.99f, -0.46f, 0.0f);

	glVertex3f(-0.97f, -0.46f, 0.0f);

	glVertex3f(-0.94f, -0.48f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________

	//filled circle
	float bx1, by1, bx2, by2;
	float angle2;
	double radius2 = 0.01;

	bx1 = -0.85, by1 = -0.49;

	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(bx1, by1);

	for (angle2 = 1.0f;angle2<361.0f;angle2 += 0.2)
	{
		bx2 = bx1 + sin(angle2)*radius2;
		by2 = by1 + cos(angle2)*radius2;
		glVertex2f(bx2, by2);
	}

	glEnd();*/
	glTranslatef(p3x, p3y, -1.4f);

	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, -0.44f, 0.0f);

	glVertex3f(-0.96f, -0.40f, 0.0f);

	glVertex3f(-0.80f, -0.385f, 0.0f);

	glVertex3f(-0.80f, -0.455f, 0.0f);

	glEnd();

	//------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.80f, -0.385f, 0.0f);

	glVertex3f(-0.80f, -0.455f, 0.0f);

	glVertex3f(-0.75f, -0.430f, 0.0f);

	glVertex3f(-0.75f, -0.410f, 0.0f);

	glEnd();

	//-----------------------------------------------------------------------------------------------------
	//filled circle
	float x1, y1, x2, y2;
	float angle;
	double radius = 0.01;

	x1 = -0.75, y1 = -0.420;

	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x1, y1);

	for (angle = 1.0f;angle<361.0f;angle += 0.2)
	{
		x2 = x1 + sin(angle)*radius;
		y2 = y1 + cos(angle)*radius;
		glVertex2f(x2, y2);
	}

	glEnd();

	//---------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//upper wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.92f, -0.44f, 0.0f);

	glVertex3f(-0.92f, -0.50f, 0.0f);

	glVertex3f(-0.89f, -0.50f, 0.0f);

	glVertex3f(-0.84f, -0.44f, 0.0f);

	glEnd();

	//---------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//lower wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.92f, -0.40f, 0.0f);

	glVertex3f(-0.92f, -0.34f, 0.0f);

	glVertex3f(-0.89f, -0.34f, 0.0f);

	glVertex3f(-0.84f, -0.40f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail up
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.95f, -0.42f, 0.0f);

	glVertex3f(-0.97f, -0.42f, 0.0f);

	glVertex3f(-0.99f, -0.46f, 0.0f);

	glVertex3f(-0.97f, -0.46f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail down
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.95f, -0.42f, 0.0f);

	glVertex3f(-0.97f,- 0.42f, 0.0f);

	glVertex3f(-0.99f, -0.38f, 0.0f);

	glVertex3f(-0.97f, -0.38f, 0.0f);

	glEnd();
	
}

//##################################################################################################################

void DrawLetterI1(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(i1x, i1y, -1.5f);

	glLineWidth(60.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.56f, 0.3f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.56f, -0.3f, 0.0f);

	//I horizontal lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.65f, 0.3f, 0.0f);

	glVertex3f(-0.47f, 0.3f, 0.0f);

	//I horizontal lines
	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.65f, -0.3f, 0.0f);

	glVertex3f(-0.47f, -0.3f, 0.0f);

	glEnd();
}

void DrawLetterN(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(nx, ny, -1.5f);

	glLineWidth(60.0f);

	glBegin(GL_LINES);

	//Letter N
	//N vertical lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.37f, 0.3f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.37f, -0.3f, 0.0f);



	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.19f, 0.3f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.19f, -0.3f, 0.0f);


	//N diagonal line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.37f, 0.3f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.19f, -0.3f, 0.0f);


	glEnd();
}

void DrawLetterD(void)
{
	/*//saffron color
	static float s1 = 0.0f;
	static float s2 = 0.0f;
	static float s3 = 0.0f;

	//green color
	static float g1 = 0.0f;
	static float g2 = 0.0f;
	static float g3 = 0.0f;*/


	//glColor3f(1.0f, 0.6f, 0.2f);
	//glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -1.5f);

	glLineWidth(60.0f);

	glBegin(GL_LINES);

	//Letter D
	//D vertical lines
	glColor3f(s1, s2, s3);

	glVertex3f(-0.09f, 0.3f, 0.0f);


	glColor3f(g1, g2, g3);

	glVertex3f(-0.09f, -0.3f, 0.0f);


	glColor3f(s1, s2, s3);

	glVertex3f(0.09f, 0.2f, 0.0f);


	glColor3f(g1, g2, g3);

	glVertex3f(0.09f, -0.2f, 0.0f);


	//D horizontal lines
	glColor3f(s1, s2, s3);

	glVertex3f(-0.09f, 0.3f, 0.0f);

	glVertex3f(0.04f, 0.3f, 0.0f);


	glColor3f(g1, g2, g3);

	glVertex3f(-0.09f, -0.3f, 0.0f);

	glVertex3f(0.04f, -0.3f, 0.0f);


	//D diagonal lines

	glColor3f(s1, s2, s3);

	glVertex3f(0.04f, 0.3f, 0.0f);

	glVertex3f(0.09f, 0.2f, 0.0f);


	glColor3f(g1, g2, g3);

	glVertex3f(0.04f, -0.3f, 0.0f);

	glVertex3f(0.09f, -0.2f, 0.0f);


	glEnd();
}

void DrawLetterI2(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(i2x, i2y, -1.5f);

	glLineWidth(60.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.28f, 0.3f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.28f, -0.3f, 0.0f);

	//I horizontal lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.19f, 0.3f, 0.0f);

	glVertex3f(0.37f, 0.3f, 0.0f);

	//I horizontal lines
	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.19f, -0.3f, 0.0f);

	glVertex3f(0.37f, -0.3f, 0.0f);

	glEnd();
}

void DrawLetterA(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(ax, ay, -1.5f);

	glLineWidth(60.0f);

	glBegin(GL_LINES);

	//Letter A
	//A diagonal line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.56f, 0.3f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.47f, -0.3f, 0.0f);


	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.56f, 0.3f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.65f, -0.3f, 0.0f);


	glEnd();
}

//#################################################################################################################

void PositionLetter(void)
{
	//for letter I (1st time - India)

	i1x = i1x + 0.001f;

	if (i1x >= 0.0f)
		i1x = 0.0f;
		

	i1y = i1y + 0.001f;

	if (i1y >= 0.0f)
		i1y = 0.0f;

	//for letter N

	nx = nx + 0.001f;

	if (nx >= 0.0f)
		nx = 0.0f;

	ny = ny - 0.001f;

	if (ny <= 0.0f)
		ny = 0.0f;


	//for letter D

	//glColor3f(1.0f, 0.6f, 0.2f);
	//glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	s1 = s1 + 0.001f;

	if (s1 >= 1.0f)
		s1 = 1.0f;


	s2 = s2 + 0.001f;

	if (s2 >= 0.6f)
		s2 = 0.6f;


	s3 = s3 + 0.001f;

	if (s3 >= 0.2f)
		s3 = 0.2f;


	g1 = g1 + 0.001f;

	if (g1 >= 0.0705882353f)
		g1 = 0.0705882353f;


	g2 = g2 + 0.001f;

	if (g2 >= 0.53333f)
		g2 = 0.53333f;


	g3 = g3 + 0.001f;

	if (g3 >= 0.0274509804f)
		g3 = 0.0274509804f;



	//for letter I (2nd time - indIa)

	(float)i2x = (float)i2x - 0.001;

	if (i2x <= 0.0f)
		i2x = 0.0f;

	(float)i2y = (float)i2y + 0.001;

	if (i2y >= 0.0f)
		i2y = 0.0f;


	//for letter A

	ax = ax - 0.001f;

	if (ax <= 0.0f)
		ax = 0.0f;

	ay = ay + 0.001f;

	if (ay >= 0.0f)
		ay = 0.0f;
}

void PositionPlane(void)
{
	//for movement of middle plane
	p1x = p1x + 0.001;


	//for movement of top plane
	p2x = p2x + 0.001;

	p2y = p2y + 0.001;
	if (p2y >= 0.42)
		p2y = 0.42f;

	if (p2x >=1.6f)
	{
		p2y = p2y - 0.01f;
		if (p2y <= 0.0f)
		{
			p2y = 0.0f;
		}
		
	}

	//for movement of bottom plane
	p3x = p3x + 0.001;

	p3y = p3y + 0.001;
	if (p3y >= 0.42)
		p3y = 0.42f;

	if (p3x >= 1.6f)
	{
		p3y = p3y - 0.01f;
		if (p3y <= 0.0f)
		{
			p3y = 0.0f;
		}

	}

}

//################################################################################################################

void PositionFlag(void)
{
	//glColor3f(1.0f, 0.6f, 0.2f);
	//glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	mfs1 = mfs1 + 0.001f;

	if (mfs1 >= 1.0f)
	{
		mfs1 = 1.0f;
	}
		
	mfs2 = mfs2 + 0.001f;

	if (mfs2 >= 0.6f)
	{
		mfs2 = 0.6f;
	}
		
	mfs3 = mfs3 + 0.001f;

	if (mfs3 >= 0.2f)
	{
		mfs3 = 0.2f;
	}
	
}

//################################################################################################################

void positionFlagToA(void)
{
	//for letter A

	//glColor3f(1.0f, 0.6f, 0.2f);
	//glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);


	//saffron color
	As1 = As1 + 0.001f;

	if (As1 >= 1.0f)
		As1 = 1.0f;


	As2 =As2 + 0.001f;

	if (As2 >= 0.6f)
		As2 = 0.6f;


	As3 = As3 + 0.001f;

	if (As3 >= 0.2f)
		As3 = 0.2f;

	//white color
	Aw1 = Aw1 + 0.001f;

	if (Aw1 >= 1.0f)
		Aw1 = 1.0f;


	Aw2 = Aw2 + 0.001f;

	if (Aw2 >= 1.0f)
		Aw2 = 1.0f;


	Aw3 = Aw3 + 0.001f;

	if (Aw3 >= 1.0f)
		Aw3 = 1.0f;


	//green color
	Ag1 = Ag1 + 0.001f;

	if (Ag1 >= 0.0705882353f)
		Ag1 = 0.0705882353f;


	Ag2 = Ag2 + 0.001f;

	if (Ag2 >= 0.53333f)
		Ag2 = 0.53333f;


	Ag3 = Ag3 + 0.001f;

	if (Ag3 >= 0.0274509804f)
		Ag3 = 0.0274509804f;

}

//#################################################################################################################

void TriFlagMidPlane(void)
{
	//saffron color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p1x, 0.0f, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//saffron color
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.96f, 0.01f, 0.0f);

	glVertex3f(-1.2f, 0.01f, 0.0f);

	glEnd();


	//white color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p1x, 0.0f, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	glColor3f(1.0f, 1.0f, 1.0f);

	glVertex3f(-0.96f, 0.0f, 0.0f);

	glVertex3f(-1.2f, 0.0f, 0.0f);

	glEnd();


	//green color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p1x, 0.0f, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.96f, -0.01f, 0.0f);

	glVertex3f(-1.2f, -0.01f, 0.0f);

	glEnd();
}

void TriFlagTopPlane(void)
{
	//saffron color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p2x, -p2y, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//saffron color
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.96f, 0.41f, 0.0f);

	glVertex3f(-1.2f, 0.41f, 0.0f);

	glEnd();


	//white color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p2x, -p2y, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	glColor3f(1.0f, 1.0f, 1.0f);

	glVertex3f(-0.96f, 0.42f, 0.0f);

	glVertex3f(-1.2f, 0.42f, 0.0f);

	glEnd();


	//green color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p2x, -p2y, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.96f, 0.43f, 0.0f);

	glVertex3f(-1.2f, 0.43f, 0.0f);

	glEnd();
}

void TriFlagBottomPlane(void)
{
	//saffron color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p3x, p3y, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//saffron color
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.96f, -0.43f, 0.0f);

	glVertex3f(-1.2f, -0.43f, 0.0f);

	glEnd();


	//white color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p3x, p3y, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	glColor3f(1.0f, 1.0f, 1.0f);

	glVertex3f(-0.96f, -0.42f, 0.0f);

	glVertex3f(-1.2f, -0.42f, 0.0f);

	glEnd();


	//green color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p3x, p3y, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.96f, -0.41f, 0.0f);

	glVertex3f(-1.2f, -0.41f, 0.0f);

	glEnd();
}

void FlagToA(void)
{
		//saffron color 

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glTranslatef(ax, ay, -1.5f);

		glLineWidth(9.0f);

		glBegin(GL_LINES);

		glColor3f(As1, As2, As3);

		glVertex3f(0.542f, 0.01f, 0.0f);

		glVertex3f(0.578f, 0.01f, 0.0f);

		glEnd();


		//white color 

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glTranslatef(ax, ay, -1.5f);

		glLineWidth(9.0f);

		glBegin(GL_LINES);


		glColor3f(Aw1, Aw2, Aw3);

		glVertex3f(0.54f, 0.0f, 0.0f);

		glVertex3f(0.58f, 0.0f, 0.0f);

		glEnd();


		//green color 

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glTranslatef(ax, ay, -1.5f);

		glLineWidth(9.0f);

		glBegin(GL_LINES);

		glColor3f(Ag1, Ag2, Ag3);

		glVertex3f(0.538f, -0.01f, 0.0f);

		glVertex3f(0.581f, -0.01f, 0.0f);

		glEnd();
}

//***************************************************************************************************************

void display(void)
{
	void DrawLetterI1(void);
	void DrawLetterN(void);
	void DrawLetterD(void);
	void DrawLetterI2(void);
	void DrawLetterA(void);


	void DrawTopPlane(void);
	void DrawMiddlePlane(void);
	void DrawBottomPlane(void);


	void LetterOnMiddlePlaneI(void);
	void LetterOnMiddlePlaneA(void);
	void LetterOnMiddlePlaneF(void);


	void LetterOnTopPlaneI(void);
	void LetterOnTopPlaneA(void);
	void LetterOnTopPlaneF(void);


	void LetterOnBottomPlaneI(void);
	void LetterOnBottomPlaneA(void);
	void LetterOnBottomPlaneF(void);

	


	//---------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------


	glClear(GL_COLOR_BUFFER_BIT);

	//Sleep(0.99999999999999999);
	Sleep(1);

	DrawLetterI1();
	
	DrawLetterN();
	
	DrawLetterD();
	
	DrawLetterI2();
	
	DrawLetterA();
	FlagToA();
		
	//-------------------------------------------------------------------------------------------------------------
	
	//Middle plane   
	
	//Sleep(0.9999);

	DrawMiddlePlane();
	
	LetterOnMiddlePlaneI();
	
	LetterOnMiddlePlaneA();

	LetterOnMiddlePlaneF();

	TriFlagMidPlane();

	//PositionFlag();
	//--------------------------------------------------------------------------------------------------------------

	//Top plane   


	//Sleep(0.9999);

	DrawTopPlane();
	                                                                
	LetterOnTopPlaneI();

	LetterOnTopPlaneA();

	LetterOnTopPlaneF();

	TriFlagTopPlane();

	//--------------------------------------------------------------------------------------------------------------

	//Bottom plane  

	//Sleep(0.9999);

	DrawBottomPlane();

	LetterOnBottomPlaneI();

	LetterOnBottomPlaneA();

	LetterOnBottomPlaneF();

	TriFlagBottomPlane();

	//--------------------------------------------------------------------------------------------------------------


	SwapBuffers(ghdc);

	//--------------------------------------------------------------------------------------------------------------

	//Move letters
	PositionLetter();

	//Move Planes
	PositionPlane();

	//Fade In flag to A
	positionFlagToA();
}

//*****************************************************************************************************************

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
