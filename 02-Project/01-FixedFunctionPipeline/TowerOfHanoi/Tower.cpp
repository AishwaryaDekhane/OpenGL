#include <windows.h>
#include <gl/GL.h>
#include<stdio.h>
#include<math.h>
#include<gl/GLU.h>

#pragma comment(lib,"opengl32.lib")

#pragma comment(lib,"glu32.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define PI_ 3.141592653

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
FILE *gpFile = NULL;


DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };


bool gbActiveWindow = false;

bool gbFullscreen = false;


int position[16] = { 10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85 };

int stand[3] = { 50,150,250 };	//dist

int move[1000][3];

int moves_max;

int rods[3][10];

int top[3] = { -1,-1,-1 };

int disk_no = 3;

int cnt, counter, s = 1;

float YCoor;
int FlagAnimate = 1;


//GLuint theTorus;


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
		//MessageBox(NULL, TEXT("Log file can not be created"), TEXT("Error"), MB_OK);
		//exit(0);
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
		TEXT("OpenGL : Tower Of Hanoi"),
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
	void push(int, int);
	void pop(int);
	void update(int, int, int);
	void add_disk();

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

	case WM_CHAR:
		switch (wParam)
		{
		case 'a':
		case 'A':

			add_disk();
			break;

		case 'm':
		case 'M':

			if (counter<moves_max)
			{
				pop(move[counter][1]);


				if (FlagAnimate)
					update(move[counter][0], move[counter][1], move[counter][2]);
				push(move[counter][2], move[counter][0]);


				counter++;
			}

			
			break;

		default:
			break;
		}
	
	case WM_LBUTTONDOWN:			
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


	glShadeModel(GL_SMOOTH);		//to avoid barel distortion in OpenGL as we are in frustum - use lighting model as smooth


	glClearDepth(1.0f);	//give existance to depth buffer


	glEnable(GL_DEPTH_TEST);	//for 3D geometry enable depth test i.e z axis 


	glDepthFunc(GL_LEQUAL);		//LEQUAL = less than or equal to test compared to the max value which is set to 1.0f


	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);		// Do interpolation - give color of pixel according to all other pixels


	resize(WIN_WIDTH, WIN_HEIGHT);



	return 0;
}

void push(int p, int d)
{
	rods[p][++top[p]] = d;	//disk - d
}

void pop(int p)
{
	top[p]--;
}

void TowerOfHanoi(int n, int src, int temp, int dst)
{
	if (n>0)
	{
		TowerOfHanoi(n - 1, src, dst, temp);

		move[cnt][0] = n;

		move[cnt][1] = src;

		move[cnt][2] = dst;

		cnt++;

		TowerOfHanoi(n - 1, temp, src, dst);
	}
}

//TO draw cone - stand of tower
void solidCone(GLdouble base, GLdouble height, GLint slices, GLint stacks)
 {
	   glBegin(GL_LINE_LOOP);

	   GLUquadricObj* quadric = gluNewQuadric();

	   gluQuadricDrawStyle(quadric, GLU_FILL);

	
	 gluCylinder(quadric, base, 0, height, slices, stacks);

	 gluDeleteQuadric(quadric);
	   glEnd();
}

//To draw disks in stand
void solidTorrus(GLdouble innerRadius, GLdouble outerRadius, GLint slices, GLint loops)
{

	GLUquadricObj* quadric = gluNewQuadric();

	//gluQuadricDrawStyle(quadric, GLU_FILL);

	/*void WINAPI gluDisk(
		GLUquadric *qobj,
		GLdouble   innerRadius,
		GLdouble   outerRadius,
		GLint      slices,
		GLint      loops
		);*/

	gluDisk(quadric, innerRadius, outerRadius, slices, loops);

	gluDeleteQuadric(quadric);

}

void draw_torrus()
{
	glRotatef(180.0f, 0.0f, 1.0f, 1.0f);	//for torrus

	glRotatef(20.0f, 1.0f, 0.0f, 1.0f);		//for torrus

											//void solidTorrus(GLdouble innerRadius, GLdouble outerRadius, GLint slices, GLint loops)
	solidTorrus(30, 45, 30, 30);
}

void draw_cone()
{
	glRotatef(90.0f, -1.0f, 0.0f, 0.0f); //for cone

										 //solidCone(GLdouble base, GLdouble height, GLint slices, GLint stacks)
	solidCone(2, 70, 30, 30);  //80,50,30,30
}

void drawStand()
{
	int i;

	//glColor3f(1.0f, 0.0f, 0.0f); 
	//glColor3f(0.5f, 0.0, 0.1f);

	glColor3f(0.435294f, 0.039321f, 0.329411f);		//purple shade

	for (i = 0;i<3;i++)
	{
		//glTranslatef(stand[i], 5, 0);
		glTranslatef(100, 0, 0);		//100,0,0
		//glRotatef(-90, 1, 0, 0);
		
		glPushMatrix();
		draw_torrus();
		glPopMatrix();

		glPushMatrix();
		draw_cone();
		glPopMatrix();

	}

}


void update(int n, int src, int dest)
{
	int i = 0;
	if (s <= 0)		//speed
	{
		s = 1;
	}
		
	Sleep(100);

	for (i = position[top[src] + 1];i<90;i += s)
	{
		glPushMatrix();

		glTranslatef(50.0f, 0.0f, 0.0f);
		glTranslatef(stand[src], i, 0);

		//glTranslatef(stand[i], 5, 0);

		glRotatef(85, 1, 0, 0);
		
		glColor3f(0.0f, 0.3f * n, 0.3f * n);

		solidTorrus(3.0, 10 * n, 20, 20);

		glPopMatrix();
	
		SwapBuffers(ghdc);
	
	}
	if (stand[src]<stand[dest])
		for (i = stand[src];i <= stand[dest];i += s)
		{
			glPushMatrix();

			glTranslatef(50.0f, 0.0f, 0.0f);
			glTranslatef(i, 90.0f, 0.0f);

			//glTranslatef(stand[i], 5, 0);

			glRotatef(85, 1, 0, 0);

			glColor3f(0.0f, 0.3f * n, 0.3f * n);

			solidTorrus(3.0, 10 * n, 20, 20);
			glPopMatrix();
			
			SwapBuffers(ghdc);
			
		}
	else
		for (i = stand[src];i >= stand[dest];i -= s)
		{
			glPushMatrix();

			glTranslatef(50.0f, 0.0f, 0.0f);
			glTranslatef(i, 90.0f, 0.0f);

			//glTranslatef(stand[i], 5, 0);

			glRotatef(85, 1, 0, 0);

		
			glColor3f(0.0f, 0.3f * n, 0.3f * n);

			solidTorrus(3.0, 10 * n, 20, 20);
			glPopMatrix();
		
			SwapBuffers(ghdc);
			
		}
	for (i = 70;i>position[top[dest] + 1];i -= s)
	{
		glPushMatrix();

		glTranslatef(50.0f, 0.0f, 0.0f);
		glTranslatef(stand[dest], i, 0);

		//glTranslatef(stand[i], 5, 0);

		glRotatef(85, 1, 0, 0);

	
		glColor3f(0.0f, 0.3f * n, 0.3f * n);

		solidTorrus(3.0, 10 * n, 20, 20);
		glPopMatrix();
		
		SwapBuffers(ghdc);
		
	}
}

void add_disk()
{
	int i;
	cnt = 0, counter = 0;
	YCoor = 0.1f; //0.1

	memset(rods, 0, sizeof(rods));

	memset(move, 0, sizeof(rods));

	memset(top, -1, sizeof(top));	//empty stack

	
	moves_max = pow(2, disk_no) - 1;

	for (i = disk_no;i>0;i--)
	{
		push(0, i);				//push disks
	}

	TowerOfHanoi(disk_no, 0, 1, 2);		//call to func of TOH
}


void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();


	int i, j, k;


	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//gluLookAt(0, YCoor, 0, 0, 0, -1, 0, 1, 0);

	glPushMatrix();

	gluLookAt(0, YCoor, 0, 0, 0, -1, 0, 1, 0);

	drawStand();

	glPopMatrix();



	glPushMatrix();

	gluLookAt(0, YCoor, 0, 0, 0, -1, 0, 1, 0);

	/*void gluLookAt(	GLdouble eyeX,
 	GLdouble eyeY,
 	GLdouble eyeZ,
 	GLdouble centerX,
 	GLdouble centerY,
 	GLdouble centerZ,
 	GLdouble upX,
 	GLdouble upY,
 	GLdouble upZ);
*/
	
	for (i = 0;i<3;i++)
	{
		k = 0;

		// loop for each disk

		for (j = 0;j <= top[i];j++)
		{
			glPushMatrix();
			
			//glTranslatef(stand[i], 5, 0);

			glTranslatef(50, 0, 0);

			glTranslatef(stand[i],position[k++], 0);

			glRotatef(90, 1, 0, 0);

			//glColor3f(0.0f, 1.0f, 0.0f);

			glColor3f(0.0f, 0.3f * rods[i][j], 0.3f * rods[i][j]);		//cyan shades

			solidTorrus(3.0, 10 * rods[i][j], 20, 20);

			glPopMatrix();
		}
	}

	glPopMatrix();
	
	SwapBuffers(ghdc);
}

void resize(int width, int height)
{

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	//glOrtho (left, right, bottom, top, near, far); 

	glOrtho(-50, 500, -50, 100, -50, 100);


	/*if (height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);


	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	

	if (width <= height)
		glOrtho(-100.0f, 100.0f, (-100.0f*((GLfloat)height / (GLfloat)width)), (100.0f*((GLfloat)height / (GLfloat)width)), -1.0f, 1.0f);
	else
		//glOrtho((-100.0f*((GLfloat)width / (GLfloat)height)), (100.0f*((GLfloat)width / (GLfloat)height)), -100.0f, 100.0f, -100.0f, 100.0f);
*/



	//glClearColor(0.0, 0.0, 0.0, 0);

	//glOrtho(0, 1000, 0, 1000, -1, 1);
	//glMatrixMode(GL_MODELVIEW);
	//glEnable(GL_DEPTH_TEST);
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
