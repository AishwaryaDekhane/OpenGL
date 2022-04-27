#include <windows.h>
#include <gl/glew.h>
#include <gl/GL.h>
#include<stdio.h>
#include <math.h>
#include "vmath.h"
#include "Sphere.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"Sphere.lib")

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

GLuint vao_pyramid;
GLuint vbo_pyramid_position;
GLuint vbo_pyramid_normal;
GLuint vbo_pyramid_element;

mat4 perspectiveProjectionMatrix;	//mat4 is from vmath

GLuint ModelMatrixUniform;
GLuint ViewMatrixUniform;
GLuint ProjectionMatrixUniform;
GLuint LaUniform_red;
GLuint KaUniform;
GLuint LdUniform_red;
GLuint KdUniform;
GLuint LsUniform_red;
GLuint KsUniform;
GLuint LightPositionUniform_red;
GLuint MaterialShininessUniform;
GLuint LKeyPressedUniform;

GLuint ModelMatrixUniform_blue;
GLuint ViewMatrixUniform_blue;
GLuint ProjectionMatrixUniform_blue;
GLuint LaUniform_blue;
GLuint LdUniform_blue;
GLuint LsUniform_blue;
GLuint KaUniform_blue;
GLuint KdUniform_blue;
GLuint KsUniform_blue;
GLuint LightPositionUniform_blue;
GLuint MaterialShininessUniform_blue;
GLuint LKeyPressedUniform_blue;

GLfloat angle_pyramid = 0.0f;

bool gbAnimate;
bool gbLight;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_element[2280];

GLfloat LightAmbientZero[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat LightDiffuseZero[] = { 1.0f,0.0f,0.0f,1.0f };
GLfloat LightSpecularZero[] = { 1.0f,0.0f,0.0f,1.0f };	//providing same values for diffuse and specular not compulsory but it looks good calculation is done using maths
GLfloat LightPositionZero[] = { -2.0f,0.0f,0.0f,1.0f };

GLfloat LightAmbientOne[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat LightDiffuseOne[] = { 0.0f,0.0f,1.0f,1.0f };
GLfloat LightSpecularOne[] = { 0.0f,0.0f,1.0f,1.0f };
GLfloat LightPositionOne[] = { 2.0f,0.0f,0.0f,1.0f };

GLfloat MaterialAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat MaterialDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat MaterialSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat MaterialShininess[] = { 128.f };

int gNumVertices;
int gNumElements;

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
				//update();
				display();

				if (gbAnimate == true)
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
	void update(void);

	static bool bIsAKeyPressed = false;
	static bool bIsLKeyPressed = false;

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

		case 0x41:	//A or a
			if (bIsAKeyPressed == false)
			{
				gbAnimate = true;
				bIsAKeyPressed = true;
			}
			else
			{
				gbAnimate = false;
				bIsAKeyPressed = false;
			}
			break;

		case 0x4C:  //L or l
			if (bIsLKeyPressed == false)
			{
				gbLight = true;
				bIsLKeyPressed = true;
			}
			else
			{
				gbLight = false;
				bIsLKeyPressed = false;
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
	angle_pyramid = angle_pyramid + 0.05f;
	if (angle_pyramid >= 360.0f)
		angle_pyramid = angle_pyramid - 360.0f;

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
		"in vec3 vNormal;" \
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform mat4 u_view_matrix;" \
		"uniform int u_LKeyPressed;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform float u_matrial_shininess;"
		"out vec3 phong_ADS_light;" \

		"uniform vec3 u_La_red;" \
		"uniform vec3 u_Ld_red;" \
		"uniform vec3 u_Ls_red;" \
		"uniform vec4 u_light_position_red;" \

		"uniform vec3 u_La_blue;" \
		"uniform vec3 u_Ld_blue;" \
		"uniform vec3 u_Ls_blue;" \
		"uniform vec4 u_light_position_blue;" \


		"void main(void)" \
		"{" \
		"if (u_LKeyPressed == 1)" \
		"{" \
		"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;" \
		"vec3 tNorm = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);" \

		"vec3 light_direction_red = normalize(vec3(u_light_position_red - eyeCoordinates));" \
		"float tn_dot_Ld_red = max(dot(light_direction_red, tNorm), 0.0);" \
		"vec3 reflection_vector_red = reflect(-light_direction_red, tNorm);" \
		"vec3 viewer_vector_red = normalize(vec3(-eyeCoordinates.xyz));" \

		"vec3 light_direction_blue = normalize(vec3(u_light_position_blue - eyeCoordinates));" \
		"float tn_dot_Ld_blue = max(dot(light_direction_blue, tNorm), 0.0);" \
		"vec3 reflection_vector_blue = reflect(-light_direction_blue, tNorm);" \
		"vec3 viewer_vector_blue = normalize(vec3(-eyeCoordinates.xyz));" \

		"vec3 ambient = u_La_red * u_Ka + u_La_blue * u_Ka;" \
		"vec3 diffuse = u_Ld_red * u_Kd * tn_dot_Ld_red + u_Ld_blue * u_Kd * tn_dot_Ld_blue;" \
		"vec3 specular = u_Ls_red * u_Ks * pow(max(dot(reflection_vector_red, viewer_vector_red), 0.0), u_matrial_shininess) +  u_Ls_blue * u_Ks * pow(max(dot(reflection_vector_blue, viewer_vector_blue), 0.0), u_matrial_shininess);" \
		"phong_ADS_light = ambient + diffuse + specular;" \

	
		"}" \
		"else" \
		"{" \
		"phong_ADS_light = vec3(1.0, 1.0, 1.0);" \
		"}" \
		"gl_Position = u_projection_matrix * u_model_matrix * u_view_matrix * vPosition;" \
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
		"in vec3 phong_ADS_light;" \
		"out vec4 FragColor;" \
		"uniform int u_LKeyPressed;" \
		"void main(void)" \
		"{" \
		"if (u_LKeyPressed == 1)" \
		"{" \
		"FragColor = vec4(phong_ADS_light,1.0);" \
		"}" \
		"else" \
		"{" \
		"FragColor = vec4(1.0, 1.0, 1.0, 1.0);" \
		"}" \
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
	glBindAttribLocation(ShaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");

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

	ModelMatrixUniform = glGetUniformLocation(ShaderProgramObject, "u_model_matrix");
	ProjectionMatrixUniform = glGetUniformLocation(ShaderProgramObject, "u_projection_matrix");
	ViewMatrixUniform = glGetUniformLocation(ShaderProgramObject, "u_view_matrix");

	LKeyPressedUniform = glGetUniformLocation(ShaderProgramObject, "u_LKeyPressed");
	MaterialShininessUniform = glGetUniformLocation(ShaderProgramObject, "u_matrial_shininess");

	LaUniform_red = glGetUniformLocation(ShaderProgramObject, "u_La_red");
	LaUniform_blue = glGetUniformLocation(ShaderProgramObject, "u_La_blue");
	KaUniform = glGetUniformLocation(ShaderProgramObject, "u_Ka");

	LdUniform_red = glGetUniformLocation(ShaderProgramObject, "u_Ld_red");
	LdUniform_blue = glGetUniformLocation(ShaderProgramObject, "u_Ld_blue");
	KdUniform = glGetUniformLocation(ShaderProgramObject, "u_Kd");

	LsUniform_red = glGetUniformLocation(ShaderProgramObject, "u_Ls_red");
	LsUniform_blue = glGetUniformLocation(ShaderProgramObject, "u_Ls_blue");
	KsUniform = glGetUniformLocation(ShaderProgramObject, "u_Ks");

	LightPositionUniform_red = glGetUniformLocation(ShaderProgramObject, "u_light_position_red");
	LightPositionUniform_blue = glGetUniformLocation(ShaderProgramObject, "u_light_position_blue");

	//declare vertex3f coordinates here

	const GLfloat pyramidVertices[] =
	{
		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,

		0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,

		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
	};

	const GLfloat pyramidNormals[] = 
	{
		0.0f,0.447214f,0.894427f,	//front-top
		0.0f,0.447214f,0.894427f,	//front-left
		0.0f,0.447214f,0.894427f,	//front-right

		0.894427f, 0.447214f, 0.0f,	//right-top
		0.894427f, 0.447214f, 0.0f,	//right-left
		0.894427f, 0.447214f, 0.0f,	//right-right

		0.0f, 0.447214f, -0.894427f,	//back-top
		0.0f, 0.447214f, -0.894427f,	//back-left
		0.0f, 0.447214f, -0.894427f,	//back-right

		-0.894427f, 0.447214f, 0.0f,	//left-top
		-0.894427f, 0.447214f, 0.0f,	//left-left
		-0.894427f, 0.447214f, 0.0f,	//left-right

	};


	//--------------------------------------------------------------------------------------------------------------

	//For sphere

	//vao = VertexArrayObject
	glGenVertexArrays(1, &vao_pyramid);

	//bind array with vao
	glBindVertexArray(vao_pyramid);

	//-------------------------------------------------------------------------------------------------------

	//For position

	//create vbo
	glGenBuffers(1, &vbo_pyramid_position);

	//target vbo <-> GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_position);

	//fill the buffer data statically
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

	//give strides to vPosition
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);  //GL_FALSE = we are not sending normalized co-or

																					//enable array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp

														//unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	//-------------------------------------------------------------------------------------------------------


	//For Normals

	//create vbo
	glGenBuffers(1, &vbo_pyramid_normal);

	//target vbo <-> GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_normal);

	//fill the buffer data statically
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNormals), pyramidNormals, GL_STATIC_DRAW);

	//give strides to vPosition
	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);  //GL_FALSE = we are not sending normalized co-or

																				  //enable array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);	//AMC = AstroMediComp

														//unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	//unbind vao
	glBindVertexArray(0);
	//----------------------------------------------------------------------------------------------------------
	//for background color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


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

	gbAnimate = false;
	gbLight = false;

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

	//for lights
	if (gbLight == true)
	{
		//for red light

		glUniform1i(LKeyPressedUniform, 1);
		glUniform1f(MaterialShininessUniform, 1);

		glUniform3fv(LaUniform_red, 1, LightAmbientZero);
		glUniform3fv(KaUniform, 1, MaterialAmbient);

		glUniform3fv(LdUniform_red, 1, LightDiffuseZero);
		glUniform3fv(KdUniform, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_red, 1, LightSpecularZero);
		glUniform3fv(KsUniform, 1, MaterialSpecular);

		float lightPosition_red[] = { -2.0f,0.0f,0.0f,1.0f };
		glUniform4fv(LightPositionUniform_red, 1, (GLfloat *)lightPosition_red);

		//for blue light

		glUniform3fv(LaUniform_blue, 1, LightAmbientOne);
		
		glUniform3fv(LdUniform_blue, 1, LightDiffuseOne);
		
		glUniform3fv(LsUniform_blue, 1, LightSpecularOne);
		
		float lightPosition_blue[] = { 2.0f,0.0f,0.0f,1.0f };
		glUniform4fv(LightPositionUniform_blue, 1, (GLfloat *)lightPosition_blue);
	}
	else
	{
		glUniform1i(LKeyPressedUniform, 0);
	}

	// OpenGL Drawing
	//declaration of matrices
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;

	//for rotation
	mat4 ratationMatrix;
	mat4 translationMatrix;


	//For rectangle
	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	//do necessary transformation if ant required
	translationMatrix = translate(0.0f, 0.0f, -5.0f);

	ratationMatrix = rotate(0.0f, angle_pyramid, 0.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	//bind with vao(this will avoid many repetitive binding with vbo)
	glBindVertexArray(vao_pyramid);

	//similarly bind the textures if any

	//draw necessary scene
	//glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	// glDrawArrays(GLenum, GLint , GLsizei)
	// (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

	//unbind va0
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

	if (vao_pyramid)
	{
		glDeleteBuffers(1, &vao_pyramid);
		vao_pyramid = 0;
	}

	if (vbo_pyramid_position)
	{
		glDeleteBuffers(1, &vbo_pyramid_position);
		vbo_pyramid_position = 0;
	}

	if (vbo_pyramid_normal)
	{
		glDeleteBuffers(1, &vbo_pyramid_normal);
		vbo_pyramid_normal = 0;
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
