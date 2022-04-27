#include <windows.h>
#include <gl/glew.h>
#include <gl/GL.h>
#include<stdio.h>
#include <math.h>
#include "vmath.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define PI 3.141592653589793238463

using namespace vmath;

enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXCOORD0,
};

//Prototype Of WndProc() declared Globally
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//Global variable declarations
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
FILE *gpFile = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbActiveWindow = false;
bool gbFullscreen = false;

GLenum result;
GLuint ShaderProgramObject;

GLuint vao_triangle;
GLuint vbo_triangle_pos;
GLuint vbo_triangle_col;

GLuint vao_line;
GLuint vbo_line_pos;
GLuint vbo_line_col;

GLuint vao_inner_circle;
GLuint vbo_inner_circle_pos;
GLuint vbo_inner_circle_col;

GLuint mvpUniform;					//mvp = ModelViewProjection
mat4 perspectiveProjectionMatrix;	//mat4 is from vmath

GLfloat angle_Tri = 0.0f;
GLfloat angle_Circle = 0.0f;
GLfloat angle_line = 0.0f;


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
		TEXT("OpenGL Programmable Pipeline"),
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
	void uninitialize(void);
	void resize(int, int);
	void initializeTriangle();
	void initializeLine();
	void initializeOuterCircle();
	void initializeInnerCircle();

	//variable declaration
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	GLuint gVertexShaderObject;
	GLuint gFragmentShaderObject;

	//for shaders - variable declration
	GLint iShaderCompileStatus = 0;
	GLint iInfoLogLength = 0;
	GLchar *szInfoLog = NULL;
	GLint iProgramLinkStatus = 0;

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

	result = glewInit();

	if (result != GLEW_OK)
	{
		fprintf(gpFile, "glewInit failed\n\n");
		uninitialize();
		DestroyWindow(ghwnd);

	}

	//VERTEX SHADER

	//define vertex shader object
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	//write vertex shader code
	const GLchar *vertexShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec4 vColor;" \
		"out vec4 out_color;" \
		"uniform mat4 u_mvp_matrix;" \
		"void main(void)" \
		"{" \
		"gl_Position = u_mvp_matrix * vPosition;" \
		"out_color = vColor;" \
		"}";

	//specify above source code to vertex shader object
	glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

	/*

	*	void glShaderSource(	GLuint shader,
	*	GLsizei count,
	*	const GLchar **string,
	*	const GLint *length);

	*	Parameters
	*	shader
	*	Specifies the handle of the shader object whose source code is to be replaced.

	*	count
	*	Specifies the number of elements in the string and length arrays.

	*	string
	*	Specifies an array of pointers to strings containing the source code to be loaded into the shader.

	*	length
	*	Specifies an array of string lengths.
	*/


	//compile the vertex shader
	glCompileShader(gVertexShaderObject);

	/***Steps For Error Checking***/
	/*
	1.	Call glGetShaderiv(), and get the compile status of that object.
	2.	check that compile status, if it is GL_FALSE then shader has compilation error.
	3.	if(GL_FALSE) call again the glGetShaderiv() function and get the
	infoLogLength.
	4.	if(infoLogLength > 0) then call glGetShaderInfoLog() function to get the error
	information.
	5.	Print that obtained logs in file.
	*/

	//Error checking for VS

	//step 1
	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);


	//step 2
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);

		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);

			if (szInfoLog != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf_s(gpFile, "Vertex Shader Error : \n %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				DestroyWindow(ghwnd);
				exit(0);

			}
		}
	}


	//FRAGMENT SHADER

	//define fragment shader object
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//write fragment shader code
	const GLchar *fragmentShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"out vec4 FragColor;" \
		"in vec4 out_color;" \
		"void main(void)" \
		"{" \
		"FragColor = out_color;" \
		"}";

	//color of triangle is yellow mentioned here in above code


	//specify above sourec code to fragment shader object
	glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

	//compile the shader
	glCompileShader(gFragmentShaderObject);

	//Error checking for FS

	iShaderCompileStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);

	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);

		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);

			if (szInfoLog != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf_s(gpFile, "Fragment Shader Errors : \n %s \n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				DestroyWindow(ghwnd);
				exit(0);

			}
		}
	}

	//SHADER PROGRAM

	//create obj
	ShaderProgramObject = glCreateProgram();

	//add vertex shader
	glAttachShader(ShaderProgramObject, gVertexShaderObject);

	//add fragment shader
	glAttachShader(ShaderProgramObject, gFragmentShaderObject);

	//Prelinking binding to vertex attributes
	glBindAttribLocation(ShaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(ShaderProgramObject, AMC_ATTRIBUTE_COLOR, "vColor");

	glLinkProgram(ShaderProgramObject);

	//Error checking for shader

	iInfoLogLength = 0;
	szInfoLog = NULL;

	glGetProgramiv(ShaderProgramObject, GL_LINK_STATUS, &iProgramLinkStatus);

	if (iProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(ShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);

		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);

			if (szInfoLog != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(ShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				fprintf_s(gpFile, "Program Link Error : \n %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				DestroyWindow(ghwnd);
				exit(0);

			}
		}
	}

	//Postlinking retriving uniform locations
	mvpUniform = glGetUniformLocation(ShaderProgramObject, "u_mvp_matrix");

	//declare vertex3f coordinates here

	//--------------------------------------------------------------------------------------------------------------
	initializeInnerCircle();
	initializeTriangle();
	initializeLine();

	//-------------------------------------------------------------------------------------------------------------

	//for background color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


	//For Depth

	//glShadeModel(GL_SMOOTH);

	glClearDepth(1.0f);	//give existance to depth buffer

	glEnable(GL_DEPTH_TEST);	//for 3D geometry enable depth test i.e z axis 

	glDepthFunc(GL_LEQUAL);		//LEQUAL = less than or equal to test compared to the max value which is set to 1.0f

								//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

								//for animation - rotation
								//glEnable(GL_CULL_FACE);

								//identity() is in mat4
	perspectiveProjectionMatrix = mat4::identity();

	//warmup call
	resize(WIN_WIDTH, WIN_HEIGHT);

	return 0;
}

void initializeTriangle()
{

	const GLfloat triangle_pos[] =
	{
		0.0f,0.5f,0.0f,
		-0.5f,-0.5f,0.0f,
		-0.5f,-0.5f,0.0f,
		0.5f,-0.5f,0.0f,
		0.5f,-0.5f,0.0f,
		0.0f,0.5f,0.0f,
	};

	const GLfloat triangle_col[] =
	{
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
	};


	//create vao and vbo
	glGenVertexArrays(1, &vao_triangle);
	glBindVertexArray(vao_triangle);

	//yellow
	glGenBuffers(1, &vbo_triangle_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_pos), triangle_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_triangle_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_col), triangle_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void initializeLine()
{
	GLfloat linepos[] = {
		0.0f, 0.5f, 0.0f,
		0.0f, -0.5f, 0.0f
	};

	GLfloat lineCol[] = {
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &vao_line);
	glBindVertexArray(vao_line);

	glGenBuffers(1, &vbo_line_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_line_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(linepos), linepos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_line_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_line_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineCol), lineCol, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

}

void initializeInnerCircle()
{
	GLfloat innerCircleVertex[10];
	GLfloat radius_inner = 0.308;

	//glVertex3f(radius*cos(angle) + x_coordinate_of_center, radius*sin(angle) + y_coordinate_of_center,0.0f);




	//Calculation done here and their ans

	/*radius of incircle = area of triangle/ semiperimeter	= A/S								 0.3088 = 0.4998 / 1.618

	area of triangle(A) = sqrt(S*(S-a)*(S-b)*(S-c))             //sqrt= square root of()		 0.4988 = sqrt(1.618*(1.618-1.118)*(1.618-1.118)*(1.618-1))

	semiperimeter= a+b+c/2                // where a,b,c are sides of triangle					 1.618 = (1.118+1.118+1) /2

	To find lenght of triangle use distance formula -
	dist = sqrt((x2-x1)^2 + (y2-y1)^2 + (z2-z1)^2)												 for side1 - 1.118 = sqrt((0.5-0.0)^2 + (-0.5-0.5)^2)

	center of triangle -
	x = (x1+x2+x3) / 3																			 x=0.0+(-0.5)+0.5 / 3
	y = (y1+y2+y3) / 3																			 y=0.5+(-0.5)+(-0.5) / 3
	*/

	GLfloat innerCircleCol[] = {
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
	};

	glGenVertexArrays(1, &vao_inner_circle);
	glBindVertexArray(vao_inner_circle);

	for (GLfloat angle = 0.0f; angle < (2.0f * PI); angle = angle + 0.01f)
	{
		innerCircleVertex[0] = ((cos(angle) * radius_inner));
		innerCircleVertex[1] = ((sin(angle) * radius_inner));
		innerCircleVertex[2] = 0.0f;
	}

	glGenBuffers(1, &vbo_inner_circle_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_inner_circle_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(innerCircleVertex), innerCircleVertex, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_inner_circle_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_inner_circle_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(innerCircleCol), innerCircleCol, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void update(void)
{
	angle_Tri = angle_Tri + 0.05f;
	if (angle_Tri >= 360.0f)
		angle_Tri = angle_Tri - 360.0f;

	angle_Circle = angle_Circle + 0.05f;
	if (angle_Circle >= 360.0f)
		angle_Circle = angle_Circle - 360.0f;

	angle_line = angle_line + 0.05f;
	if (angle_line >= 360.0f)
		angle_line = angle_line - 360.0f;

}


void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//binding
	glUseProgram(ShaderProgramObject);

	// OpenGL Drawing
	//declaration of matrices
	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;
	mat4 ratationMatrix;

	//-------------------------------------------------------------------------------------------------------------------

	//for triangle

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	ratationMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(0.0f, 0.001f, -2.0f);

	ratationMatrix = rotate(angle_Tri, 0.0f, 1.0f, 0.0f);

	//do necessary matrix multiplication
	modelViewMatrix = modelViewMatrix * ratationMatrix;

	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	//bind with vao
	glBindVertexArray(vao_triangle);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//------------------------------------------------------------------------------------------------------------------

	//for line

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	ratationMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(0.0f, 0.0f, -2.0f);

	ratationMatrix = rotate(angle_line, 0.0f, 1.0f, 0.0f);

	//do necessary matrix multiplication
	modelViewMatrix = modelViewMatrix * ratationMatrix;

	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	//bind with vao
	glBindVertexArray(vao_line);

	//draw scene
	//glLineWidth(2.0f);

	glDrawArrays(GL_LINES, 0, 2);

	//unbind vao
	glBindVertexArray(0);
	//------------------------------------------------------------------------------------------------------------------

	//for inner circle

	GLfloat innerCircleVertex[10];
	GLfloat radius_inner = 0.308;

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	ratationMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(0.0f, -0.19f, -2.0f);

	ratationMatrix = rotate(angle_Circle, 0.0f, 1.0f, 0.0f);

	//do necessary matrix multiplication
	modelViewMatrix = modelViewMatrix * ratationMatrix;

	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glBindVertexArray(vao_inner_circle);

	for (GLfloat angle = 0.0f; angle < (2.0f * PI); angle = angle + 0.01f)
	{
		innerCircleVertex[0] = ((cos(angle) * radius_inner));
		innerCircleVertex[1] = ((sin(angle) * radius_inner));
		innerCircleVertex[2] = 0.0f;

		glBindBuffer(GL_ARRAY_BUFFER, vbo_inner_circle_pos);
		glBufferData(GL_ARRAY_BUFFER, sizeof(innerCircleVertex), innerCircleVertex, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//draw scene
		glPointSize(2.0f);
		glDrawArrays(GL_POINTS, 0, 1);
	}

	//unbind vao
	glBindVertexArray(0);

	//--------------------------------------------------------------------------------------------------------------------

	//unbinding - unused program
	glUseProgram(0);

	SwapBuffers(ghdc);
}

void resize(int width, int height)
{
	//code
	//(0, 0, (GLsizei)width, (GLsizei)height);

	if (height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	perspectiveProjectionMatrix = perspective(45.0f, ((GLfloat)width / (GLfloat)height), 0.1f, 100.0f);
}

void uninitialize(void)
{

	//declare variables
	GLsizei shaderCount;
	GLsizei shaderNumber;


	if (gbFullscreen == true)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);

	}

	if (vao_triangle)
	{
		glDeleteVertexArrays(1, &vao_triangle);
		vao_triangle = 0;
	}

	if (vbo_triangle_pos)
	{
		glDeleteVertexArrays(1, &vbo_triangle_pos);
		vbo_triangle_pos = 0;
	}

	if (vbo_triangle_col)
	{
		glDeleteVertexArrays(1, &vbo_triangle_col);
		vbo_triangle_col = 0;
	}

	if (vao_inner_circle)
	{
		glDeleteVertexArrays(1, &vao_inner_circle);
		vao_inner_circle = 0;
	}

	if (vbo_inner_circle_pos)
	{
		glDeleteVertexArrays(1, &vbo_inner_circle_pos);
		vbo_inner_circle_pos = 0;
	}

	if (vbo_inner_circle_col)
	{
		glDeleteVertexArrays(1, &vbo_inner_circle_col);
		vbo_inner_circle_col = 0;
	}

	if (vao_line)
	{
		glDeleteVertexArrays(1, &vao_line);
		vao_line = 0;
	}

	if (vbo_line_pos)
	{
		glDeleteVertexArrays(1, &vbo_line_pos);
		vbo_line_pos = 0;
	}

	if (vbo_line_col)
	{
		glDeleteVertexArrays(1, &vbo_line_col);
		vbo_line_col = 0;
	}

	//safe release

	if (ShaderProgramObject)
	{
		glUseProgram(ShaderProgramObject);

		//ask program how many shaders are attached to you

		glGetProgramiv(ShaderProgramObject, GL_ATTACHED_SHADERS, &shaderCount);

		GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);

		if (pShaders)
		{
			glGetAttachedShaders(ShaderProgramObject, shaderCount, &shaderCount, pShaders);

			for (shaderNumber = 0;shaderNumber < shaderCount;shaderNumber++)
			{
				glDetachShader(ShaderProgramObject, pShaders[shaderNumber]);

				//delete detach shaders
				glDeleteShader(pShaders[shaderNumber]);

				pShaders[shaderNumber] = 0;
			}

			free(pShaders);
		}

		glDeleteProgram(ShaderProgramObject);
		ShaderProgramObject = 0;

		glUseProgram(0);
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
		fprintf(gpFile, "Log file is closed successfully \n\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}
