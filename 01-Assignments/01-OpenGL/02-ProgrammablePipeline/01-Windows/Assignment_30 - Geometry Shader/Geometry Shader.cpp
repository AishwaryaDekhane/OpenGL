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
GLuint gGeometryShaderObject;

GLuint vao;
GLuint vbo_vertices;
GLuint vbo_color;
GLuint mvpUniform;					//mvp = ModelViewProjection
mat4 perspectiveProjectionMatrix;	//mat4 is from vmath

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
		"uniform mat4 u_mvp_matrix;" \
		"out vec4 out_color;" \
		"void main(void)" \
		"{" \
		"out_color = vColor;" \
		"gl_Position = u_mvp_matrix * vPosition;" \
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

	// *** GEOMETRY SHADER ***

	// create shader
	gGeometryShaderObject = glCreateShader(GL_GEOMETRY_SHADER);

	// provide source code to shader
	const GLchar *geometryShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"layout(triangles)in;" \
		"layout(triangle_strip, max_vertices = 9)out;" \
		"uniform mat4 u_mvp_matrix;" \
		"in vec4 out_color[];" \
		"out vec4 glColor;" \
		"void main(void)" \
		"{" \
		"for(int vertex = 0; vertex < 3; vertex++)" \
		"{" \

		"gl_Position = u_mvp_matrix * (gl_in[vertex].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));" \
		"glColor = out_color[vertex];" \
		"EmitVertex();" \

		"gl_Position = u_mvp_matrix * (gl_in[vertex].gl_Position + vec4(-1.0, -1.0, 0.0, 0.0));" \
		"glColor = out_color[vertex];" \
		"EmitVertex();" \

		"gl_Position = u_mvp_matrix * (gl_in[vertex].gl_Position + vec4(1.0, -1.0, 0.0, 0.0));" \
		"glColor = out_color[vertex];" \
		"EmitVertex();" \

		"EndPrimitive();" \
		"}" \
		"}";

	glShaderSource(gGeometryShaderObject, 1, (const GLchar **) &geometryShaderSourceCode, NULL);

	// compile shadeh
	glCompileShader(gGeometryShaderObject);
	// re-initialize
	iInfoLogLength = 0;
	iShaderCompileStatus = 0;
	szInfoLog = NULL;
	glGetShaderiv(gGeometryShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gGeometryShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gGeometryShaderObject, iInfoLogLength,&written, szInfoLog);
				fprintf(gpFile, "Geometry Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
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
		"in vec4 glColor;" \
		"void main(void)" \
		"{" \
		"FragColor = glColor;" \
		"}";

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

	//attch geometry control shader to shader program
	glAttachShader(ShaderProgramObject, gGeometryShaderObject);

	//add fragment shader
	glAttachShader(ShaderProgramObject, gFragmentShaderObject);

	//Prelinking binding to vertex attributes
	glBindAttribLocation(ShaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");

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

	// *** vertices, colors, shader attribs, vbo, vao initializations ***
	//triangle vertices declaration
	const GLfloat triangleVertices[] =
	{
		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f
	};

	//triangle color declaration
	const GLfloat triangleColor[] =
	{
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};

	//create vao and vbo
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	glGenBuffers(1, &vbo_color);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColor), triangleColor, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//for background color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


	//For Depth

	//glShadeModel(GL_SMOOTH);

	glClearDepth(1.0f);	//give existance to depth buffer

	glEnable(GL_DEPTH_TEST);	//for 3D geometry enable depth test i.e z axis 

	glDepthFunc(GL_LEQUAL);		//LEQUAL = less than or equal to test compared to the max value which is set to 1.0f

								//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

								//identity() is in mat4
	perspectiveProjectionMatrix = mat4::identity();

	//warmup call
	resize(WIN_WIDTH, WIN_HEIGHT);

	return 0;
}

void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//binding
	glUseProgram(ShaderProgramObject);

	// OpenGL Drawing

	//declaration of metrices
	vmath::mat4 modelViewMatrix;
	vmath::mat4 modelViewProjectionMatrix;

	//init above metrices to identity
	modelViewMatrix = vmath::mat4::identity();
	modelViewProjectionMatrix = vmath::mat4::identity();

	//do necessary transformations here
	modelViewMatrix = vmath::translate(0.0f, 0.0f, -5.0f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	//send necessary matrics to shaders in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	//bind with vao
	glBindVertexArray(vao);

	//draw scene
	glDrawArrays(GL_TRIANGLES, 0, 3);

	//unbind vaos
	glBindVertexArray(0);

	//unbinding - unused program
	glUseProgram(0);

	SwapBuffers(ghdc);
}

void resize(int width, int height)
{
	//code
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

	if (vao)
	{
		glDeleteBuffers(1, &vao);
		vao = 0;
	}

	if (vbo_color)
	{
		glDeleteBuffers(1, &vbo_color);
		vbo_color = 0;
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
