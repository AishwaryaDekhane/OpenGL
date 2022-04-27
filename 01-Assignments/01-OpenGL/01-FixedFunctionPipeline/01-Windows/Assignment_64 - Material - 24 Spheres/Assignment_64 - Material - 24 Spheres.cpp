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

GLfloat AngleOfXRotation = 0.0f;
GLfloat AngleOfYRotation = 0.0f;
GLfloat AngleOfZRotation = 0.0f;

GLint KeyPress = 0;

bool bLight = false;

GLfloat LightAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat LightDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat LightPosition[] = { 0.0f,0.0f,0.0f,1.0f };

GLfloat Light_Model_Ambient[] = { 0.2f,0.2f,0.2f,1.0f };
GLfloat Light_Model_Local_Viewer[] = { 0.0f };

GLUquadric *quadric [26];

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbActiveWindow = false;

bool gbFullscreen = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//fuction declaration
	int initialize(void);
	void display(void);
	void update(void);


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
				update();
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

	case WM_CHAR:
		switch (wParam)
		{
		case 'L':						//lights
		case 'l':
			if (bLight == false)
			{
				bLight = true;
				glEnable(GL_LIGHTING);
			}
			else
			{
				bLight = false;
				glDisable(GL_LIGHTING);
			}
			break;

		case 'X':
		case 'x':
			KeyPress = 1;
			AngleOfXRotation = 0.0f;
			break;

		case 'y':
		case 'Y':
			KeyPress = 2;
			AngleOfYRotation = 0.0f;
			break;

		case 'z':
		case 'Z':
			KeyPress = 3;
			AngleOfZRotation = 0.0f;
			break;


		default:
			break;
		}
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

void update(void)
{
	AngleOfXRotation = AngleOfXRotation + 0.05f;
	if (AngleOfXRotation >= 360.0f)
		AngleOfXRotation = AngleOfXRotation - 360.0f;

	AngleOfYRotation = AngleOfYRotation + 0.05f;
	if (AngleOfYRotation >= 360.0f)
		AngleOfYRotation = AngleOfYRotation - 360.0f;

	AngleOfZRotation = AngleOfZRotation + 0.05f;
	if (AngleOfZRotation >= 360.0f)
		AngleOfZRotation = AngleOfZRotation - 360.0f;

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
	pfd.cDepthBits = 32;	//full depth

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


	glClearColor(0.25f, 0.25f, 0.25f, 0.0f); //dark gray

	glShadeModel(GL_SMOOTH);		//to avoid barel distortion in OpenGL as we are in frustum - use lighting model as smooth

	glClearDepth(1.0f);	//give existance to depth buffer

	glEnable(GL_DEPTH_TEST);	//for 3D geometry enable depth test i.e z axis 

	glDepthFunc(GL_LEQUAL);		//LEQUAL = less than or equal to test compared to the max value which is set to 1.0f

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);		// Do interpolation - give color of pixel according to all other pixels

	//for lights
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);

	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, Light_Model_Ambient);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, Light_Model_Local_Viewer);

	glEnable(GL_LIGHT0);

	for (int i = 0;i <= 24;i++)
	{
		quadric[i] = gluNewQuadric();
	}

	resize(WIN_WIDTH, WIN_HEIGHT);

	return 0;
}

void Draw24Spheres(void)
{
	GLfloat MaterialAmbient[4];
	GLfloat MaterialDiffuse[4];;
	GLfloat MaterialSpecular[4];
	GLfloat MaterialShininess[1];

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//-------------------- 1st sphere on 1st column, emerald ------------------------------

	MaterialAmbient[0] = 0.0215f;	//r
	MaterialAmbient[1] = 0.1745f;	//g
	MaterialAmbient[2] = 0.0215f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.07568f;	//r
	MaterialDiffuse[1] = 0.61424f;	//g
	MaterialDiffuse[2] = 0.07568f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.633f;	//r
	MaterialSpecular[1] = 0.727811f;	//g
	MaterialSpecular[2] = 0.633f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.6f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(1.5f, 14.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30); //draw sphere

										 //-------------------- 2nd sphere on 1st column, jade ------------------------------

	MaterialAmbient[0] = 0.135f;	//r
	MaterialAmbient[1] = 0.2225f;	//g
	MaterialAmbient[2] = 0.1575f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.54f;	//r
	MaterialDiffuse[1] = 0.89f;	//g
	MaterialDiffuse[2] = 0.63f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.316228f;	//r
	MaterialSpecular[1] = 0.316228f;	//g
	MaterialSpecular[2] = 0.316228f; //b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.1f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(1.5f, 11.65f, 0.0f);
	gluSphere(quadric[1], 1.0f, 30, 30); //draw sphere

										 //-------------------- 3rd sphere on 1st column, obsidian ------------------------------

	MaterialAmbient[0] = 0.05375f;	//r
	MaterialAmbient[1] = 0.05f;	//g
	MaterialAmbient[2] = 0.06625f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.18275f;	//r
	MaterialDiffuse[1] = 0.17f;	//g
	MaterialDiffuse[2] = 0.22525f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.332741f;	//r
	MaterialSpecular[1] = 0.328634f;	//g
	MaterialSpecular[2] = 0.346435f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.3f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(1.5f, 9.32f, 0.0f);
	gluSphere(quadric[2], 1.0f, 30, 30); //draw sphere

										 //-------------------- 4th sphere on 1st column, pearl ------------------------------

	MaterialAmbient[0] = 0.25f;	//r
	MaterialAmbient[1] = 0.20725f;	//g
	MaterialAmbient[2] = 0.20725f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 1.0f;	//r
	MaterialDiffuse[1] = 0.829f;	//g
	MaterialDiffuse[2] = 0.829f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.296648f;	//r
	MaterialSpecular[1] = 0.296648f;	//g
	MaterialSpecular[2] = 0.296648f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.088f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(1.5f, 6.99f, 0.0f);
	gluSphere(quadric[3], 1.0f, 30, 30); //draw sphere

										 //-------------------- 5th sphere on 1st column, ruby ------------------------------

	MaterialAmbient[0] = 0.1745f;	//r
	MaterialAmbient[1] = 0.01175f;	//g
	MaterialAmbient[2] = 0.01175f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.61424f;	//r
	MaterialDiffuse[1] = 0.04136f;	//g
	MaterialDiffuse[2] = 0.04136f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.727811f;	//r
	MaterialSpecular[1] = 0.626959f;	//g
	MaterialSpecular[2] = 0.626959f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.6f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(1.5f, 4.66f, 0.0f);
	gluSphere(quadric[4], 1.0f, 30, 30); //draw sphere

										 //-------------------- 6th sphere on 1st column, turquoise ------------------------------

	MaterialAmbient[0] = 0.1f;	//r
	MaterialAmbient[1] = 0.18725f;	//g
	MaterialAmbient[2] = 0.1745f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.396f;	//r
	MaterialDiffuse[1] = 0.74151f;	//g
	MaterialDiffuse[2] = 0.69102f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.297254f;	//r
	MaterialSpecular[1] = 0.30829f;	//g
	MaterialSpecular[2] = 0.306678f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.1f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(1.5f, 2.33f, 0.0f);
	gluSphere(quadric[5], 1.0f, 30, 30); //draw sphere

										 //-------------------- 7- 1st sphere on 2nd column, brass ------------------------------

	MaterialAmbient[0] = 0.329412f;	//r
	MaterialAmbient[1] = 0.223529f;	//g
	MaterialAmbient[2] = 0.027451f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.780392f;	//r
	MaterialDiffuse[1] = 0.568627f;	//g
	MaterialDiffuse[2] = 0.113725f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.992157f;	//r
	MaterialSpecular[1] = 0.941176f;	//g
	MaterialSpecular[2] = 0.807843f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.21794872f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(9.5f, 14.0f, 0.0f);
	gluSphere(quadric[6], 1.0f, 30, 30); //draw sphere

										 //-------------------- 8- 2nd sphere on 2nd column, bronze ------------------------------

	MaterialAmbient[0] = 0.2125f;	//r
	MaterialAmbient[1] = 0.1275f;	//g
	MaterialAmbient[2] = 0.054f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.714f;	//r
	MaterialDiffuse[1] = 0.4284f;	//g
	MaterialDiffuse[2] = 0.18144f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.393548f;	//r
	MaterialSpecular[1] = 0.271906f;	//g
	MaterialSpecular[2] = 0.166721f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.2f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(9.5f, 11.65f, 0.0f);
	gluSphere(quadric[8], 1.0f, 30, 30); //draw sphere

										 //-------------------- 9- 3rd sphere on 2nd column, chrome ------------------------------

	MaterialAmbient[0] = 0.25f;	//r
	MaterialAmbient[1] = 0.25f;	//g
	MaterialAmbient[2] = 0.25f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.4f;	//r
	MaterialDiffuse[1] = 0.4f;	//g
	MaterialDiffuse[2] = 0.4f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.774597f;	//r
	MaterialSpecular[1] = 0.774597f;	//g
	MaterialSpecular[2] = 0.774597f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.6f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(9.5f, 9.32f, 0.0f);
	gluSphere(quadric[9], 1.0f, 30, 30); //draw sphere

										 //-------------------- 10- 4th sphere on 2nd column, copper ------------------------------

	MaterialAmbient[0] = 0.19125f;	//r
	MaterialAmbient[1] = 0.0735f;	//g
	MaterialAmbient[2] = 0.0225f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.7038f;	//r
	MaterialDiffuse[1] = 0.27048f;	//g
	MaterialDiffuse[2] = 0.0828f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.256777f;	//r
	MaterialSpecular[1] = 0.137622f;	//g
	MaterialSpecular[2] = 0.086014f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.1f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(9.5f, 6.99f, 0.0f);
	gluSphere(quadric[10], 1.0f, 30, 30); //draw sphere

										 //-------------------- 11- 5th sphere on 2nd column, gold ------------------------------

	MaterialAmbient[0] = 0.24725f;	//r
	MaterialAmbient[1] = 0.1995f;	//g
	MaterialAmbient[2] = 0.0745f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.75164f;	//r
	MaterialDiffuse[1] = 0.60648f;	//g
	MaterialDiffuse[2] = 0.22648f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.628281f;	//r
	MaterialSpecular[1] = 0.555802f;	//g
	MaterialSpecular[2] = 0.366065f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.4f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(9.5f, 4.66f, 0.0f);
	gluSphere(quadric[11], 1.0f, 30, 30); //draw sphere

										 //-------------------- 12- 6th sphere on 2nd column, silver ------------------------------

	MaterialAmbient[0] = 0.19225f;	//r
	MaterialAmbient[1] = 0.19225f;	//g
	MaterialAmbient[2] = 0.19225f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.50754f;	//r
	MaterialDiffuse[1] = 0.50754f;	//g
	MaterialDiffuse[2] = 0.50754f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.508273f;	//r
	MaterialSpecular[1] = 0.508273f;	//g
	MaterialSpecular[2] = 0.508273f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.4f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(9.5f, 2.33f, 0.0f);
	gluSphere(quadric[12], 1.0f, 30, 30); //draw sphere

										 //-------------------- 13- 1st sphere on 3rd column, black plastic ------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.01f;	//r
	MaterialDiffuse[1] = 0.01f;	//g
	MaterialDiffuse[2] = 0.01f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.50f;	//r
	MaterialSpecular[1] = 0.50f;	//g
	MaterialSpecular[2] = 0.50f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(17.5f, 14.0f, 0.0f);
	gluSphere(quadric[13], 1.0f, 30, 30); //draw sphere

										 //-------------------- 14- 2nd sphere on 3rd column, cyan plastic ------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.1f;	//g
	MaterialAmbient[2] = 0.06f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.0f;	//r
	MaterialDiffuse[1] = 0.50980392f;	//g
	MaterialDiffuse[2] = 0.50980392f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.50196078f;	//r
	MaterialSpecular[1] = 0.50196078f;	//g
	MaterialSpecular[2] = 0.50196078f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(17.5f, 11.65f, 0.0f);
	gluSphere(quadric[14], 1.0f, 30, 30); //draw sphere

										 //-------------------- 15- 3rd sphere on 2nd column, green plastic------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.1f;	//r
	MaterialDiffuse[1] = 0.35f;	//g
	MaterialDiffuse[2] = 0.1f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.45f;	//r
	MaterialSpecular[1] = 0.55f;	//g
	MaterialSpecular[2] = 0.45f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(17.5f, 9.32f, 0.0f);
	gluSphere(quadric[15], 1.0f, 30, 30); //draw sphere

										 //-------------------- 16- 4th sphere on 3rd column, red plastic------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.5f;	//r
	MaterialDiffuse[1] = 0.0f;	//g
	MaterialDiffuse[2] = 0.0f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.7f;	//r
	MaterialSpecular[1] = 0.6f;	//g
	MaterialSpecular[2] = 0.6f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(17.5f, 6.99f, 0.0f);
	gluSphere(quadric[16], 1.0f, 30, 30); //draw sphere

										 //-------------------- 17- 5th sphere on 3rd column, white plastic------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.02f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.55f;	//r
	MaterialDiffuse[1] = 0.55f;	//g
	MaterialDiffuse[2] = 0.55f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.70f;	//r
	MaterialSpecular[1] = 0.70f;	//g
	MaterialSpecular[2] = 0.70f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(17.5f, 4.66f, 0.0f);
	gluSphere(quadric[17], 1.0f, 30, 30); //draw sphere

										 //-------------------- 18- 6th sphere on 3rd column, yellow plastic ------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.5f;	//r
	MaterialDiffuse[1] = 0.5f;	//g
	MaterialDiffuse[2] = 0.0f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.60f;	//r
	MaterialSpecular[1] = 0.60f;	//g
	MaterialSpecular[2] = 0.50f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(17.5f, 2.33f, 0.0f);
	gluSphere(quadric[18], 1.0f, 30, 30); //draw sphere

										 //-------------------- 19- 1st sphere on 4th column, black rubber ------------------------------

	MaterialAmbient[0] = 0.02f;	//r
	MaterialAmbient[1] = 0.02f;	//g
	MaterialAmbient[2] = 0.02f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.01f;	//r
	MaterialDiffuse[1] = 0.01f;	//g
	MaterialDiffuse[2] = 0.01f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.4f;	//r
	MaterialSpecular[1] = 0.4f;	//g
	MaterialSpecular[2] = 0.4f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(25.5f, 14.0f, 0.0f);
	gluSphere(quadric[19], 1.0f, 30, 30); //draw sphere

										 //-------------------- 20- 2nd sphere on 4th column, cyan rubber ------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.05f;	//g
	MaterialAmbient[2] = 0.05f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.4f;	//r
	MaterialDiffuse[1] = 0.5f;	//g
	MaterialDiffuse[2] = 0.5f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.04f;	//r
	MaterialSpecular[1] = 0.7f;	//g
	MaterialSpecular[2] = 0.7f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(25.5f, 11.65f, 0.0f);
	gluSphere(quadric[20], 1.0f, 30, 30); //draw sphere

										 //-------------------- 21- 3rd sphere on 4th column, green rubber ------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.05f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.4f;	//r
	MaterialDiffuse[1] = 0.5f;	//g
	MaterialDiffuse[2] = 0.4f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.04f;	//r
	MaterialSpecular[1] = 0.7f;	//g
	MaterialSpecular[2] = 0.04f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(25.5f, 9.32f, 0.0f);
	gluSphere(quadric[21], 1.0f, 30, 30); //draw sphere

										 //-------------------- 22- 4th sphere on 4th column, red rubber------------------------------

	MaterialAmbient[0] = 0.05f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.5f;	//r
	MaterialDiffuse[1] = 0.4f;	//g
	MaterialDiffuse[2] = 0.4f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.7f;	//r
	MaterialSpecular[1] = 0.04f;	//g
	MaterialSpecular[2] = 0.04f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(25.5f, 6.99f, 0.0f);
	gluSphere(quadric[22], 1.0f, 30, 30); //draw sphere

										 //-------------------- 23- 5th sphere on 4th column, white ------------------------------

	MaterialAmbient[0] = 0.05f;	//r
	MaterialAmbient[1] = 0.05f;	//g
	MaterialAmbient[2] = 0.05f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.5f;	//r
	MaterialDiffuse[1] = 0.5f;	//g
	MaterialDiffuse[2] = 0.5f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.7f;	//r
	MaterialSpecular[1] = 0.7f;	//g
	MaterialSpecular[2] = 0.7f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(25.5f, 4.66f, 0.0f);
	gluSphere(quadric[23], 1.0f, 30, 30); //draw sphere

										 //-------------------- 24- 6th sphere on 4th column, yellow rubber ------------------------------

	MaterialAmbient[0] = 0.05f;	//r
	MaterialAmbient[1] = 0.05f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.5f;	//r
	MaterialDiffuse[1] = 0.5f;	//g
	MaterialDiffuse[2] = 0.4f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.7f;	//r
	MaterialSpecular[1] = 0.7f;	//g
	MaterialSpecular[2] = 0.04f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(25.5f, 2.33f, 0.0f);
	gluSphere(quadric[24], 1.0f, 30, 30); //draw sphere*/

}

void display(void)
{
	//prototype
	void Draw24Spheres(void);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	
	if (KeyPress == 1)
	{
		glRotatef(AngleOfXRotation, 1.0f, 0.0f, 0.0f);
		LightPosition[1] = AngleOfXRotation;
	}
	else if (KeyPress == 2)
	{
		glRotatef(AngleOfYRotation, 0.0f, 1.0f, 0.0f);
		LightPosition[2] = AngleOfYRotation;
	}
	else if (KeyPress == 3)
	{
		glRotatef(AngleOfZRotation, 0.0f, 0.0f, 1.0f);
		LightPosition[0] = AngleOfZRotation;
	}

	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
	Draw24Spheres();

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

	if (width <= height)
		glOrtho(0.0f, 15.5f, 0.0f, (15.5f*((GLfloat)height / (GLfloat)width)), -10.0f, 10.0f);//multiply to width - (left,right,bottom,top,near,far)
	else
		glOrtho(0.0f, (15.5f*((GLfloat)width / (GLfloat)height)), 0.0f, 15.5f, -10.0f, 10.0f);//multiply to height

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

	/*for (int i = 0;i < 24;i++)
	{
		gluDeleteQuadric[i];
	}*/
}
