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

GLuint ShaderProgramObject_pv;
GLuint ShaderProgramObject_pf;

GLuint vao_sphere;
GLuint vbo_sphere_position;
GLuint vbo_sphere_normal;
GLuint vbo_sphere_element;

mat4 perspectiveProjectionMatrix;	//mat4 is from vmath

GLuint ModelMatrixUniform_pv;
GLuint ViewMatrixUniform_pv;
GLuint ProjectionMatrixUniform_pv;
GLuint MaterialShininessUniform_pv;
GLuint LKeyPressedUniform_pv;
GLuint KaUniform_pv;
GLuint KdUniform_pv;
GLuint KsUniform_pv;

GLuint LaUniform_pv_zero;
GLuint LdUniform_pv_zero;
GLuint LsUniform_pv_zero;
GLuint LightPositionUniform_pv_zero;

GLuint LaUniform_pv_one;
GLuint LdUniform_pv_one;
GLuint LsUniform_pv_one;
GLuint LightPositionUniform_pv_one;

GLuint LaUniform_pv_two;
GLuint LdUniform_pv_two;
GLuint LsUniform_pv_two;
GLuint LightPositionUniform_pv_two;


GLuint ModelMatrixUniform_pf;
GLuint ViewMatrixUniform_pf;
GLuint ProjectionMatrixUniform_pf;
GLuint MaterialShininessUniform_pf;
GLuint LKeyPressedUniform_pf;
GLuint KaUniform_pf;
GLuint KdUniform_pf;
GLuint KsUniform_pf;

GLuint LaUniform_pf_zero;
GLuint LdUniform_pf_zero;
GLuint LsUniform_pf_zero;
GLuint LightPositionUniform_pf_zero;

GLuint LaUniform_pf_one;
GLuint LdUniform_pf_one;
GLuint LsUniform_pf_one;
GLuint LightPositionUniform_pf_one;

GLuint LaUniform_pf_two;
GLuint LdUniform_pf_two;
GLuint LsUniform_pf_two;
GLuint LightPositionUniform_pf_two;

bool gbAnimate;
bool gbLight;
bool gbVertex;
bool gbFragment;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_element[2280];

//LIGHT0
GLfloat LightAmbientZero[4] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat LightDiffuseZero[4] = { 1.0f,0.0f,0.0f,1.0f };
GLfloat LightSpecularZero[4] = { 1.0f,0.0f,0.0f,1.0f };
GLfloat LightPositionZero[4] = { 100.0f,0.0f,0.0f,1.0f };

//LIGHT1
GLfloat LightAmbientOne[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat LightDiffuseOne[] = { 0.0f,1.0f,0.0f,1.0f };
GLfloat LightSpecularOne[] = { 0.0f,1.0f,0.0f,1.0f };
GLfloat LightPositionOne[] = { 0.0f,-100.0f,0.0f,1.0f };

//LIGHT2
GLfloat LightAmbientTwo[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat LightDiffuseTwo[] = { 0.0f,0.0f,1.0f,1.0f };
GLfloat LightSpecularTwo[] = { 0.0f,0.0f,1.0f,1.0f };
GLfloat LightPositionTwo[] = { 0.0f,0.0f,100.0f,1.0f };

GLfloat MaterialAmbient[4] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat MaterialDiffuse[4] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat MaterialSpecular[4] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat MaterialShininess[1] = { 128.0f };

GLfloat LightAngleZero = 0.0f;
GLfloat LightAngleOne = 0.0f;
GLfloat LightAngleTwo = 0.0f;

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
	static bool bIsFKeyPressed = false;
	static bool bIsVKeyPressed = false;

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
		case VK_ESCAPE:		//for full screen
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

		case 0x51: //for 'q' or 'Q' Quit application
			DestroyWindow(hwnd);
			break;

		case 0x41:	//A or a	Animate Application
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

		case 0x4C:  //L or l	Lights on / off
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

		case 0x46:	//for f or F	for Fragment shader
			if (bIsFKeyPressed == false)
			{
				gbFragment = true;
				bIsFKeyPressed = true;
			}
			else
			{
				gbFragment = false;
				bIsFKeyPressed = false;
			}
			break;

		case 0x56:	//for v or V	for vertex shader
			if (bIsVKeyPressed == false)
			{
				gbVertex = true;
				bIsVKeyPressed = true;
			}
			else
			{
				gbVertex = false;
				bIsVKeyPressed = false;
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
	LightAngleZero = LightAngleZero + 0.0005f;
	if (LightAngleZero >= 360.0f)
		LightAngleZero = LightAngleZero - 360.0f;

	LightAngleOne = LightAngleOne + 0.0005f;
	if (LightAngleOne >= 360.0f)
		LightAngleOne = LightAngleOne - 360.0f;

	LightAngleTwo = LightAngleTwo + 0.0005f;
	if (LightAngleTwo >= 360.0f)
		LightAngleTwo = LightAngleTwo - 360.0f;

}

int initialize(void)
{
	//function declaration
	void uninitialize(void);
	void resize(int, int);

	//variable declaration
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	GLuint gVertexShaderObject_pv;
	GLuint gFragmentShaderObject_pv;

	GLuint gVertexShaderObject_pf;
	GLuint gFragmentShaderObject_pf;

	//for shaders - variable declration
	GLint iShaderCompileStatus_pv = 0;
	GLint iInfoLogLength_pv = 0;
	GLchar *szInfoLog_pv = NULL;
	GLint iProgramLinkStatus_pv = 0;

	GLint iShaderCompileStatus_pf = 0;
	GLint iInfoLogLength_pf = 0;
	GLchar *szInfoLog_pf = NULL;
	GLint iProgramLinkStatus_pf = 0;

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

	//VERTEX SHADER for per vertex

	//define vertex shader object
	gVertexShaderObject_pv = glCreateShader(GL_VERTEX_SHADER);

	//write vertex shader code
	const GLchar *vertexShaderSourceCode_pv =
		"#version 450 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform mat4 u_view_matrix;" \
		"uniform int u_LKeyPressed;" \
		"uniform float u_matrial_shininess;" \
		"out vec3 phong_ADS_light;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \

		"uniform vec3 u_La_zero;" \
		"uniform vec3 u_Ld_zero;" \
		"uniform vec3 u_Ls_zero;" \
		"uniform vec4 u_light_position_zero;" \

		"uniform vec3 u_La_one;" \
		"uniform vec3 u_Ld_one;" \
		"uniform vec3 u_Ls_one;" \
		"uniform vec4 u_light_position_one;" \

		"uniform vec3 u_La_two;" \
		"uniform vec3 u_Ld_two;" \
		"uniform vec3 u_Ls_two;" \
		"uniform vec4 u_light_position_two;" \

		"void main(void)" \
		"{" \
		"if (u_LKeyPressed == 1)" \
		"{" \
		"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;" \
		"vec3 tNorm = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);" \

		"vec3 light_direction_zero = normalize(vec3(u_light_position_zero - eyeCoordinates));" \
		"float tn_dot_Ld_zero = max(dot(light_direction_zero, tNorm), 0.0);" \
		"vec3 reflection_vector_zero = reflect(-light_direction_zero, tNorm);" \

		"vec3 light_direction_one = normalize(vec3(u_light_position_one - eyeCoordinates));" \
		"float tn_dot_Ld_one = max(dot(light_direction_one, tNorm), 0.0);" \
		"vec3 reflection_vector_one = reflect(-light_direction_one, tNorm);" \

		"vec3 light_direction_two = normalize(vec3(u_light_position_two - eyeCoordinates));" \
		"float tn_dot_Ld_two = max(dot(light_direction_two, tNorm), 0.0);" \
		"vec3 reflection_vector_two = reflect(-light_direction_two, tNorm);" \

		"vec3 viewer_vector = normalize(vec3(-eyeCoordinates.xyz));" \


		"vec3 ambient = u_La_zero * u_Ka + u_La_one * u_Ka + u_La_two * u_Ka;" \
		"vec3 diffuse = u_Ld_zero * u_Kd * tn_dot_Ld_zero + u_Ld_one * u_Kd * tn_dot_Ld_one + u_Ld_two * u_Kd * tn_dot_Ld_two;" \
		"vec3 specular = u_Ls_zero * u_Ks * pow(max(dot(reflection_vector_zero, viewer_vector), 0.0), u_matrial_shininess) + u_Ls_one * u_Ks * pow(max(dot(reflection_vector_one, viewer_vector), 0.0), u_matrial_shininess) +  u_Ls_two * u_Ks * pow(max(dot(reflection_vector_two, viewer_vector), 0.0), u_matrial_shininess);" \

		"phong_ADS_light = ambient + diffuse + specular;" \
		"}" \
		"else" \
		"{" \
		"phong_ADS_light = vec3(1.0, 1.0, 1.0);" \
		"}" \
		"gl_Position = u_projection_matrix * u_model_matrix * u_view_matrix * vPosition;" \
		"}";

	//specify above source code to vertex shader object
	glShaderSource(gVertexShaderObject_pv, 1, (const GLchar **)&vertexShaderSourceCode_pv, NULL);

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
	glCompileShader(gVertexShaderObject_pv);

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
	glGetShaderiv(gVertexShaderObject_pv, GL_COMPILE_STATUS, &iShaderCompileStatus_pv);


	//step 2
	if (iShaderCompileStatus_pv == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject_pv, GL_INFO_LOG_LENGTH, &iInfoLogLength_pv);

		if (iInfoLogLength_pv > 0)
		{
			szInfoLog_pv = (GLchar*)malloc(iInfoLogLength_pv);

			if (szInfoLog_pv != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(gVertexShaderObject_pv, iInfoLogLength_pv, &written, szInfoLog_pv);
				fprintf_s(gpFile, "Vertex Shader Error : \n %s\n", szInfoLog_pv);
				free(szInfoLog_pv);
				uninitialize();
				DestroyWindow(ghwnd);
				exit(0);

			}
		}
	}

	//VERTEX SHADER for per fragment

	//define vertex shader object
	gVertexShaderObject_pf = glCreateShader(GL_VERTEX_SHADER);

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
		"out vec3 phong_ADS_light;" \
		"out vec3 tNorm;" \
		"out vec3 view_vector;" \

		"uniform vec4 u_light_position_zero;" \
		"out vec3 light_direction_zero;" \

		"uniform vec4 u_light_position_one;" \
		"out vec3 light_direction_one;" \

		"uniform vec4 u_light_position_two;" \
		"out vec3 light_direction_two;" \
		

		"void main(void)" \
		"{" \
		"if (u_LKeyPressed == 1)" \
		"{" \
		"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;" \
		"tNorm = mat3(u_view_matrix * u_model_matrix) * vNormal;" \

		"light_direction_zero = vec3(u_light_position_zero - eyeCoordinates);" \
		"light_direction_one = vec3(u_light_position_one - eyeCoordinates);" \
		"light_direction_two = vec3(u_light_position_two - eyeCoordinates);" \

		"vec3 viewer_vector = vec3(-eyeCoordinates.xyz);" \

		"}" \
		"gl_Position = u_projection_matrix * u_model_matrix * u_view_matrix * vPosition;" \
		"}";

	//specify above source code to vertex shader object
	glShaderSource(gVertexShaderObject_pf, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

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
	glCompileShader(gVertexShaderObject_pf);

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
	glGetShaderiv(gVertexShaderObject_pf, GL_COMPILE_STATUS, &iShaderCompileStatus_pf);


	//step 2
	if (iShaderCompileStatus_pf == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject_pf, GL_INFO_LOG_LENGTH, &iInfoLogLength_pf);

		if (iInfoLogLength_pf > 0)
		{
			szInfoLog_pf = (GLchar*)malloc(iInfoLogLength_pf);

			if (szInfoLog_pf != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(gVertexShaderObject_pf, iInfoLogLength_pf, &written, szInfoLog_pf);
				fprintf_s(gpFile, "Vertex Shader Error : \n %s\n", szInfoLog_pf);
				free(szInfoLog_pf);
				uninitialize();
				DestroyWindow(ghwnd);
				exit(0);

			}
		}
	}


	//FRAGMENT SHADER for per vertex

	//define fragment shader object
	gFragmentShaderObject_pv = glCreateShader(GL_FRAGMENT_SHADER);

	//write fragment shader code
	const GLchar *fragmentShaderSourceCode_pv =
		"#version 450 core" \
		"\n" \
		"in vec3 phong_ADS_light;" \
		"out vec4 FragColor;" \
		"uniform int u_LKeyPressed;" \
		"void main(void)" \
		"{" \
		"vec4 color;" \
		"if (u_LKeyPressed == 1)" \
		"{" \
		"color = vec4(phong_ADS_light,1.0);" \
		"}" \
		"else" \
		"{" \
		"color = vec4(1.0, 1.0, 1.0, 1.0);" \
		"}" \
		"FragColor = color;" \
		"}";

	//color of triangle is yellow mentioned here in above code


	//specify above sourec code to fragment shader object
	glShaderSource(gFragmentShaderObject_pv, 1, (const GLchar **)&fragmentShaderSourceCode_pv, NULL);

	//compile the shader
	glCompileShader(gFragmentShaderObject_pv);

	//Error checking for FS

	iShaderCompileStatus_pv = 0;
	iInfoLogLength_pv = 0;
	szInfoLog_pv = NULL;

	glGetShaderiv(gFragmentShaderObject_pv, GL_COMPILE_STATUS, &iShaderCompileStatus_pv);

	if (iShaderCompileStatus_pv == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject_pv, GL_INFO_LOG_LENGTH, &iInfoLogLength_pv);

		if (iInfoLogLength_pv > 0)
		{
			szInfoLog_pv = (GLchar*)malloc(iInfoLogLength_pv);

			if (szInfoLog_pv != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(gFragmentShaderObject_pv, iInfoLogLength_pv, &written, szInfoLog_pv);
				fprintf_s(gpFile, "Fragment Shader Errors : \n %s \n", szInfoLog_pv);
				free(szInfoLog_pv);
				uninitialize();
				DestroyWindow(ghwnd);
				exit(0);

			}
		}
	}

	//FRAGMENT SHADER for per fragment

	//define fragment shader object
	gFragmentShaderObject_pf = glCreateShader(GL_FRAGMENT_SHADER);

	//write fragment shader code
	const GLchar *fragmentShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform float u_material_shininess;" \
		"in vec3 phong_ADS_light;" \
		"in vec3 tNorm;" \
		"out vec4 FragColor;" \
		"uniform int u_LKeyPressed;" \
		"in vec3 view_vector;" \

		"uniform vec3 u_La_zero;" \
		"uniform vec3 u_Ld_zero;" \
		"uniform vec3 u_Ls_zero;" \
		"in vec3 light_direction_zero;" \

		"uniform vec3 u_La_one;" \
		"uniform vec3 u_Ld_one;" \
		"uniform vec3 u_Ls_one;" \
		"in vec3 light_direction_one;" \

		"uniform vec3 u_La_two;" \
		"uniform vec3 u_Ld_two;" \
		"uniform vec3 u_Ls_two;" \
		"in vec3 light_direction_two;" \
		

		"void main(void)" \
		"{" \
		"vec4 color;" \
		"if (u_LKeyPressed == 1)" \
		"{" \
		"vec3 normalized_tNorm = normalize(tNorm);"
		"vec3 normalized_view_vector = normalize(view_vector);" \

		"vec3 normalized_light_direction_zero = normalize(light_direction_zero);" \
		"vec3 reflection_vector_zero = reflect(-normalized_light_direction_zero,normalized_tNorm);" \
		"float tn_dot_ld_zero = max(dot(normalized_light_direction_zero.xyz,normalized_tNorm),0.0f);" \

		"vec3 normalized_light_direction_one = normalize(light_direction_one);" \
		"vec3 reflection_vector_one = reflect(-normalized_light_direction_one,normalized_tNorm);" \
		"float tn_dot_ld_one = max(dot(normalized_light_direction_one.xyz,normalized_tNorm),0.0f);" \

		"vec3 normalized_light_direction_two = normalize(light_direction_two);" \
		"vec3 reflection_vector_two = reflect(-normalized_light_direction_two,normalized_tNorm);" \
		"float tn_dot_ld_two = max(dot(normalized_light_direction_two.xyz,normalized_tNorm),0.0f);" \

		"vec3 ambient = u_La_zero * u_Ka + u_La_one * u_Ka + u_La_two * u_Ka;" \
		"vec3 diffuse = u_Ld_zero  * u_Kd * tn_dot_ld_zero + u_Ld_one  * u_Kd * tn_dot_ld_one + u_Ld_two  * u_Kd * tn_dot_ld_two;" \
		"vec3 specular = u_Ls_zero  * u_Ks * pow(max(dot(reflection_vector_zero,normalized_view_vector),0.0f),u_material_shininess) + u_Ls_one  * u_Ks * pow(max(dot(reflection_vector_one,normalized_view_vector),0.0f),u_material_shininess) + u_Ls_two  * u_Ks * pow(max(dot(reflection_vector_two,normalized_view_vector),0.0f),u_material_shininess);" \
		"vec3 phong_ADS_light = ambient + diffuse + specular;" \

		"color = vec4(phong_ADS_light,1.0);" \
		"}" \
		"else" \
		"{" \
		"color = vec4(1.0, 1.0, 1.0, 1.0);" \
		"}" \
		"FragColor = color;" \
		"}";

	//color of triangle is yellow mentioned here in above code


	//specify above sourec code to fragment shader object
	glShaderSource(gFragmentShaderObject_pf, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

	//compile the shader
	glCompileShader(gFragmentShaderObject_pf);

	//Error checking for FS

	iShaderCompileStatus_pf = 0;
	iInfoLogLength_pf = 0;
	szInfoLog_pf = NULL;

	glGetShaderiv(gFragmentShaderObject_pf, GL_COMPILE_STATUS, &iShaderCompileStatus_pf);

	if (iShaderCompileStatus_pf == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject_pf, GL_INFO_LOG_LENGTH, &iInfoLogLength_pf);

		if (iInfoLogLength_pf > 0)
		{
			szInfoLog_pf = (GLchar*)malloc(iInfoLogLength_pf);

			if (szInfoLog_pf != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(gFragmentShaderObject_pf, iInfoLogLength_pf, &written, szInfoLog_pf);
				fprintf_s(gpFile, "Fragment Shader Errors : \n %s \n", szInfoLog_pf);
				free(szInfoLog_pf);
				uninitialize();
				DestroyWindow(ghwnd);
				exit(0);

			}
		}
	}

	//SHADER PROGRAM for pv

	//create obj
	ShaderProgramObject_pv = glCreateProgram();

	//add vertex shader
	glAttachShader(ShaderProgramObject_pv, gVertexShaderObject_pv);

	//add fragment shader
	glAttachShader(ShaderProgramObject_pv, gFragmentShaderObject_pv);

	//Prelinking binding to vertex attributes
	glBindAttribLocation(ShaderProgramObject_pv, AMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(ShaderProgramObject_pv, AMC_ATTRIBUTE_NORMAL, "vNormal");

	glLinkProgram(ShaderProgramObject_pv);

	//Error checking for shader

	iInfoLogLength_pv = 0;
	szInfoLog_pv = NULL;

	glGetProgramiv(ShaderProgramObject_pv, GL_LINK_STATUS, &iProgramLinkStatus_pv);

	if (iProgramLinkStatus_pv == GL_FALSE)
	{
		glGetProgramiv(ShaderProgramObject_pv, GL_INFO_LOG_LENGTH, &iInfoLogLength_pv);

		if (iInfoLogLength_pv > 0)
		{
			szInfoLog_pv = (GLchar*)malloc(iInfoLogLength_pv);

			if (szInfoLog_pv != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(ShaderProgramObject_pv, iInfoLogLength_pv, &written, szInfoLog_pv);
				fprintf_s(gpFile, "Program Link Error : \n %s\n", szInfoLog_pv);
				free(szInfoLog_pv);
				uninitialize();
				DestroyWindow(ghwnd);
				exit(0);

			}
		}
	}

	//Postlinking retriving uniform locations
	//mvpUniform = glGetUniformLocation(ShaderProgramObject, "u_mvp_matrix");

	ModelMatrixUniform_pv = glGetUniformLocation(ShaderProgramObject_pv, "u_model_matrix");
	ProjectionMatrixUniform_pv = glGetUniformLocation(ShaderProgramObject_pv, "u_projection_matrix");
	ViewMatrixUniform_pv = glGetUniformLocation(ShaderProgramObject_pv, "u_view_matrix");

	LKeyPressedUniform_pv = glGetUniformLocation(ShaderProgramObject_pv, "u_LKeyPressed");
	MaterialShininessUniform_pv = glGetUniformLocation(ShaderProgramObject_pv, "u_matrial_shininess");

	LaUniform_pv_zero = glGetUniformLocation(ShaderProgramObject_pv, "u_La_zero");
	LaUniform_pv_one = glGetUniformLocation(ShaderProgramObject_pv, "u_La_one");
	LaUniform_pv_two = glGetUniformLocation(ShaderProgramObject_pv, "u_La_two");
	KaUniform_pv = glGetUniformLocation(ShaderProgramObject_pv, "u_Ka");

	LdUniform_pv_zero = glGetUniformLocation(ShaderProgramObject_pv, "u_Ld_zero");
	LdUniform_pv_one = glGetUniformLocation(ShaderProgramObject_pv, "u_Ld_one");
	LdUniform_pv_two = glGetUniformLocation(ShaderProgramObject_pv, "u_Ld_two");
	KdUniform_pv = glGetUniformLocation(ShaderProgramObject_pv, "u_Kd");

	LsUniform_pv_zero = glGetUniformLocation(ShaderProgramObject_pv, "u_Ls_zero");
	LsUniform_pv_one = glGetUniformLocation(ShaderProgramObject_pv, "u_Ls_one");
	LsUniform_pv_two = glGetUniformLocation(ShaderProgramObject_pv, "u_Ls_two");
	KsUniform_pv = glGetUniformLocation(ShaderProgramObject_pv, "u_Ks");

	LightPositionUniform_pv_zero = glGetUniformLocation(ShaderProgramObject_pv, "u_light_position_zero");
	LightPositionUniform_pv_one = glGetUniformLocation(ShaderProgramObject_pv, "u_light_position_one");
	LightPositionUniform_pv_two = glGetUniformLocation(ShaderProgramObject_pv, "u_light_position_two");


	//SHADER PROGRAM for pf

	//create obj
	ShaderProgramObject_pf = glCreateProgram();

	//add vertex shader
	glAttachShader(ShaderProgramObject_pf, gVertexShaderObject_pf);

	//add fragment shader
	glAttachShader(ShaderProgramObject_pf, gFragmentShaderObject_pf);

	//Prelinking binding to vertex attributes
	glBindAttribLocation(ShaderProgramObject_pf, AMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(ShaderProgramObject_pf, AMC_ATTRIBUTE_NORMAL, "vNormal");

	glLinkProgram(ShaderProgramObject_pf);

	//Error checking for shader

	iInfoLogLength_pf = 0;
	szInfoLog_pf = NULL;

	glGetProgramiv(ShaderProgramObject_pf, GL_LINK_STATUS, &iProgramLinkStatus_pf);

	if (iProgramLinkStatus_pf == GL_FALSE)
	{
		glGetProgramiv(ShaderProgramObject_pf, GL_INFO_LOG_LENGTH, &iInfoLogLength_pf);

		if (iInfoLogLength_pf > 0)
		{
			szInfoLog_pf = (GLchar*)malloc(iInfoLogLength_pf);

			if (szInfoLog_pf != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(ShaderProgramObject_pf, iInfoLogLength_pf, &written, szInfoLog_pf);
				fprintf_s(gpFile, "Program Link Error : \n %s\n", szInfoLog_pf);
				free(szInfoLog_pf);
				uninitialize();
				DestroyWindow(ghwnd);
				exit(0);

			}
		}
	}

	//Postlinking retriving uniform locations
	//mvpUniform = glGetUniformLocation(ShaderProgramObject, "u_mvp_matrix");

	ModelMatrixUniform_pf = glGetUniformLocation(ShaderProgramObject_pf, "u_model_matrix");
	ProjectionMatrixUniform_pf = glGetUniformLocation(ShaderProgramObject_pf, "u_projection_matrix");
	ViewMatrixUniform_pf = glGetUniformLocation(ShaderProgramObject_pf, "u_view_matrix");

	LKeyPressedUniform_pf = glGetUniformLocation(ShaderProgramObject_pf, "u_LKeyPressed");
	MaterialShininessUniform_pf = glGetUniformLocation(ShaderProgramObject_pf, "u_material_shininess");

	LaUniform_pf_zero = glGetUniformLocation(ShaderProgramObject_pf, "u_La_zero");
	LaUniform_pf_one = glGetUniformLocation(ShaderProgramObject_pf, "u_La_one");
	LaUniform_pf_two = glGetUniformLocation(ShaderProgramObject_pf, "u_La_two");
	KaUniform_pf = glGetUniformLocation(ShaderProgramObject_pf, "u_Ka");

	LdUniform_pf_zero = glGetUniformLocation(ShaderProgramObject_pf, "u_Ld_zero");
	LdUniform_pf_one = glGetUniformLocation(ShaderProgramObject_pf, "u_Ld_one");
	LdUniform_pf_two = glGetUniformLocation(ShaderProgramObject_pf, "u_Ld_two");
	KdUniform_pf = glGetUniformLocation(ShaderProgramObject_pf, "u_Kd");

	LsUniform_pf_zero = glGetUniformLocation(ShaderProgramObject_pf, "u_Ls_zero");
	LsUniform_pf_one = glGetUniformLocation(ShaderProgramObject_pf, "u_Ls_one");
	LsUniform_pf_two = glGetUniformLocation(ShaderProgramObject_pf, "u_Ls_two");
	KsUniform_pf = glGetUniformLocation(ShaderProgramObject_pf, "u_Ks");

	LightPositionUniform_pf_zero = glGetUniformLocation(ShaderProgramObject_pf, "u_light_position_zero");
	LightPositionUniform_pf_one = glGetUniformLocation(ShaderProgramObject_pf, "u_light_position_one");
	LightPositionUniform_pf_two = glGetUniformLocation(ShaderProgramObject_pf, "u_light_position_two");


	//declare vertex3f coordinates here

	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_element);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();


	//--------------------------------------------------------------------------------------------------------------

	//For sphere

	//vao = VertexArrayObject
	glGenVertexArrays(1, &vao_sphere);

	//bind array with vao
	glBindVertexArray(vao_sphere);

	//-------------------------------------------------------------------------------------------------------

	//For position

	//create vbo
	glGenBuffers(1, &vbo_sphere_position);

	//target vbo <-> GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_position);

	//fill the buffer data statically
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

	//give strides to vPosition
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);  //GL_FALSE = we are not sending normalized co-or

																					//enable array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp

														//unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	//-------------------------------------------------------------------------------------------------------


	//For Normals

	//create vbo
	glGenBuffers(1, &vbo_sphere_normal);

	//target vbo <-> GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_normal);

	//fill the buffer data statically
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);

	//give strides to vPosition
	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);  //GL_FALSE = we are not sending normalized co-or

																				  //enable array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);	//AMC = AstroMediComp

														//unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//---------------------------------------------------------------------------------------------------------

	//For elements

	//create vbo
	glGenBuffers(1, &vbo_sphere_element);

	//target vbo <-> GL_ARRAY_BUFFER
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

	//fill the buffer data statically
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_element), sphere_element, GL_STATIC_DRAW);

	//unbind buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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
	gbVertex = false;
	gbFragment = false;


	//warmup call
	resize(WIN_WIDTH, WIN_HEIGHT);

	return 0;
}

void display(void)
{
	GLfloat radius = 100.0f;

	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (gbVertex == true && gbFragment == false)
	{
		//binding
		glUseProgram(ShaderProgramObject_pv);

		//for lights
		if (gbLight == true)
		{
			glUniform1i(LKeyPressedUniform_pv, 1);
			glUniform1f(MaterialShininessUniform_pv, 1);

			glUniform3fv(LaUniform_pv_zero, 1, LightAmbientZero);
			glUniform3fv(LaUniform_pv_one, 1, LightAmbientOne);
			glUniform3fv(LaUniform_pv_two, 1, LightAmbientTwo);
			glUniform3fv(KaUniform_pv, 1, MaterialAmbient);

			glUniform3fv(LdUniform_pv_zero, 1, LightDiffuseZero);
			glUniform3fv(LdUniform_pv_one, 1, LightDiffuseOne);
			glUniform3fv(LdUniform_pv_two, 1, LightDiffuseTwo);
			glUniform3fv(KdUniform_pv, 1, MaterialDiffuse);

			glUniform3fv(LsUniform_pv_zero, 1, LightSpecularZero);
			glUniform3fv(LsUniform_pv_one, 1, LightSpecularOne);
			glUniform3fv(LsUniform_pv_two, 1, LightSpecularTwo);
			glUniform3fv(KsUniform_pv, 1, MaterialSpecular);

			/*LightPositionZero[1] = LightAngleZero;
			LightPositionOne[1] = LightAngleOne;
			LightPositionTwo[1] = LightAngleTwo;*/

			LightPositionZero[0] = 0.0f;
			LightPositionZero[1] = cos(LightAngleZero) * radius;
			LightPositionZero[2] = sin(LightAngleZero) * radius;
			LightPositionZero[3] = 0.0f;

			LightPositionOne[0] = cos(LightAngleOne) * radius;
			LightPositionOne[1] = 0.0f;
			LightPositionOne[2] = sin(LightAngleOne) * radius;
			LightPositionOne[3] = 0.0f;

			LightPositionTwo[0] = 0.0f;
			LightPositionTwo[1] = sin(LightAngleTwo) * radius;
			LightPositionTwo[2] = cos(LightAngleTwo) * radius;
			LightPositionTwo[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pv_zero, 1, (GLfloat *)LightPositionZero);
			glUniform4fv(LightPositionUniform_pv_one, 1, (GLfloat *)LightPositionOne);
			glUniform4fv(LightPositionUniform_pv_two, 1, (GLfloat *)LightPositionTwo);
		}
		else
		{
			glUniform1i(LKeyPressedUniform_pv, 0);
		}

		// OpenGL Drawing
		//declaration of matrices
		mat4 modelMatrix_pv;
		mat4 viewMatrix_pv;
		mat4 projectionMatrix_pv;

		//for rotation
		mat4 ratationMatrix_pv;
		mat4 ratationMatrix_pv_zero;
		mat4 ratationMatrix_pv_one;
		mat4 ratationMatrix_pv_two;
		mat4 translationMatrix_pv;


		//For rectangle
		//initialize above matrices to identity
		modelMatrix_pv = mat4::identity();
		viewMatrix_pv = mat4::identity();
		projectionMatrix_pv = mat4::identity();

		ratationMatrix_pv = mat4::identity();
		ratationMatrix_pv_zero = mat4::identity();
		ratationMatrix_pv_one = mat4::identity();
		ratationMatrix_pv_two = mat4::identity();
		translationMatrix_pv = mat4::identity();

		//do necessary transformation if ant required
		translationMatrix_pv = translate(0.0f, 0.0f, -4.0f);

		ratationMatrix_pv_zero = rotate(LightAngleZero, 1.0f, 0.0f, 0.0f);
		ratationMatrix_pv_one = rotate(LightAngleOne, 0.0f, 1.0f, 0.0f);
		ratationMatrix_pv_two = rotate(LightAngleTwo, 0.0f, 0.0f, 1.0f);

		//do necessary matrix multiplication

		ratationMatrix_pv = ratationMatrix_pv_zero * ratationMatrix_pv_one * ratationMatrix_pv_two;

		modelMatrix_pv = translationMatrix_pv * ratationMatrix_pv;

		projectionMatrix_pv = projectionMatrix_pv * perspectiveProjectionMatrix;

		//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
		//this is internally done  by gluOrho2D/ glOrtho

		//send necessary matrices to shader in respective uniforms
		glUniformMatrix4fv(ModelMatrixUniform_pv, 1, GL_FALSE, modelMatrix_pv);

		glUniformMatrix4fv(ProjectionMatrixUniform_pv, 1, GL_FALSE, projectionMatrix_pv);
		//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

		glUniformMatrix4fv(ViewMatrixUniform_pv, 1, GL_FALSE, viewMatrix_pv);
		//GL_FALSE = internally we are not transposing = row -> col / col -> row
		//OpenGL is col major while DirectX is row major

		//bind with vao(this will avoid many repetitive binding with vbo)
		glBindVertexArray(vao_sphere);

		//similarly bind the textures if any

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

		//draw necessary scene
		glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
		// glDrawArrays(GLenum, GLint , GLsizei)
		// (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

		//unbind va0
		glBindVertexArray(0);

		//unbinding - unused program
		glUseProgram(0);

	}

	else if (gbFragment == true && gbVertex == false)
	{
		//for pf

		//binding
		glUseProgram(ShaderProgramObject_pf);

		//for lights
		if (gbLight == true)
		{
			glUniform1i(LKeyPressedUniform_pf, 1);
			glUniform1f(MaterialShininessUniform_pf, 1);

			glUniform3fv(LaUniform_pf_zero, 1, LightAmbientZero);
			glUniform3fv(LaUniform_pf_one, 1, LightAmbientOne);
			glUniform3fv(LaUniform_pf_two, 1, LightAmbientTwo);
			glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

			glUniform3fv(LdUniform_pf_zero, 1, LightDiffuseZero);
			glUniform3fv(LdUniform_pf_one, 1, LightDiffuseOne);
			glUniform3fv(LdUniform_pf_two, 1, LightDiffuseTwo);
			glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

			glUniform3fv(LsUniform_pf_zero, 1, LightSpecularZero);
			glUniform3fv(LsUniform_pf_one, 1, LightSpecularOne);
			glUniform3fv(LsUniform_pf_two, 1, LightSpecularTwo);
			glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

			/*LightPositionZero[1] = LightAngleZero;
			LightPositionOne[1] = LightAngleOne;
			LightPositionTwo[1] = LightAngleTwo;*/

			LightPositionZero[0] = 0.0f;
			LightPositionZero[1] = cos(LightAngleZero) * radius;
			LightPositionZero[2] = sin(LightAngleZero) * radius;
			LightPositionZero[3] = 0.0f;

			LightPositionOne[0] = cos(LightAngleOne) * radius;
			LightPositionOne[1] = 0.0f;
			LightPositionOne[2] = sin(LightAngleOne) * radius;
			LightPositionOne[3] = 0.0f;

			LightPositionTwo[0] = 0.0f;
			LightPositionTwo[1] = sin(LightAngleTwo) * radius;
			LightPositionTwo[2] = cos(LightAngleTwo) * radius;
			LightPositionTwo[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf_zero, 1, (GLfloat *)LightPositionZero);
			glUniform4fv(LightPositionUniform_pf_one, 1, (GLfloat *)LightPositionOne);
			glUniform4fv(LightPositionUniform_pf_two, 1, (GLfloat *)LightPositionTwo);
		}
		else
		{
			glUniform1i(LKeyPressedUniform_pf, 0);
		}

		// OpenGL Drawing
		//declaration of matrices
		mat4 modelMatrix_pf;
		mat4 viewMatrix_pf;
		mat4 projectionMatrix_pf;

		//for rotation
		mat4 ratationMatrix_pf;
		mat4 ratationMatrix_pf_one;
		mat4 ratationMatrix_pf_two;
		mat4 ratationMatrix_pf_zero;
		mat4 translationMatrix_pf;


		//For rectangle
		//initialize above matrices to identity
		modelMatrix_pf = mat4::identity();
		viewMatrix_pf = mat4::identity();
		projectionMatrix_pf = mat4::identity();

		ratationMatrix_pf = mat4::identity();
		ratationMatrix_pf_zero = mat4::identity();
		ratationMatrix_pf_one = mat4::identity();
		ratationMatrix_pf_two = mat4::identity();
		translationMatrix_pf = mat4::identity();

		//do necessary transformation if ant required
		translationMatrix_pf = translate(0.0f, 0.0f, -4.0f);

		ratationMatrix_pf_zero = rotate(LightAngleZero, 1.0f, 0.0f, 0.0f);
		ratationMatrix_pf_one = rotate(LightAngleOne, 0.0f, 1.0f, 0.0f);
		ratationMatrix_pf_two = rotate(LightAngleTwo, 0.0f, 0.0f, 1.0f);

		//do necessary matrix multiplication
		ratationMatrix_pf = ratationMatrix_pf_zero * ratationMatrix_pf_one * ratationMatrix_pf_two;

		modelMatrix_pf = translationMatrix_pf * ratationMatrix_pf;

		projectionMatrix_pf = projectionMatrix_pf * perspectiveProjectionMatrix;

		//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
		//this is internally done  by gluOrho2D/ glOrtho

		//send necessary matrices to shader in respective uniforms
		glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix_pf);

		glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix_pf);
		//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

		glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix_pf);
		//GL_FALSE = internally we are not transposing = row -> col / col -> row
		//OpenGL is col major while DirectX is row major

		//bind with vao(this will avoid many repetitive binding with vbo)
		glBindVertexArray(vao_sphere);

		//similarly bind the textures if any

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

		//draw necessary scene
		glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
		// glDrawArrays(GLenum, GLint , GLsizei)
		// (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

		//unbind va0
		glBindVertexArray(0);

		//unbinding - unused program
		glUseProgram(0);

	}

	else
	{
		//for pf by default

		//binding
		glUseProgram(ShaderProgramObject_pf);

		//for lights
		if (gbLight == true)
		{
			glUniform1i(LKeyPressedUniform_pf, 1);
			glUniform1f(MaterialShininessUniform_pf, 1);

			glUniform3fv(LaUniform_pf_zero, 1, LightAmbientZero);
			glUniform3fv(LaUniform_pf_one, 1, LightAmbientOne);
			glUniform3fv(LaUniform_pf_two, 1, LightAmbientTwo);
			glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

			glUniform3fv(LdUniform_pf_zero, 1, LightDiffuseZero);
			glUniform3fv(LdUniform_pf_one, 1, LightDiffuseOne);
			glUniform3fv(LdUniform_pf_two, 1, LightDiffuseTwo);
			glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

			glUniform3fv(LsUniform_pf_zero, 1, LightSpecularZero);
			glUniform3fv(LsUniform_pf_one, 1, LightSpecularOne);
			glUniform3fv(LsUniform_pf_two, 1, LightSpecularTwo);
			glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

			/*LightPositionZero[1] = LightAngleZero;
			LightPositionOne[1] = LightAngleOne;
			LightPositionTwo[1] = LightAngleTwo;*/

			LightPositionZero[0] = 0.0f;
			LightPositionZero[1] = cos(LightAngleZero) * radius;
			LightPositionZero[2] = sin(LightAngleZero) * radius;
			LightPositionZero[3] = 0.0f;

			LightPositionOne[0] = cos(LightAngleOne) * radius;
			LightPositionOne[1] = 0.0f;
			LightPositionOne[2] = sin(LightAngleOne) * radius;
			LightPositionOne[3] = 0.0f;

			LightPositionTwo[0] = 0.0f;
			LightPositionTwo[1] = sin(LightAngleTwo) * radius;
			LightPositionTwo[2] = cos(LightAngleTwo) * radius;
			LightPositionTwo[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf_zero, 1, (GLfloat *)LightPositionZero);
			glUniform4fv(LightPositionUniform_pf_one, 1, (GLfloat *)LightPositionOne);
			glUniform4fv(LightPositionUniform_pf_two, 1, (GLfloat *)LightPositionTwo);
		}
		else
		{
			glUniform1i(LKeyPressedUniform_pf, 0);
		}

		// OpenGL Drawing
		//declaration of matrices
		mat4 modelMatrix_pf;
		mat4 viewMatrix_pf;
		mat4 projectionMatrix_pf;

		//for rotation
		mat4 ratationMatrix_pf;
		mat4 ratationMatrix_pf_one;
		mat4 ratationMatrix_pf_two;
		mat4 ratationMatrix_pf_zero;
		mat4 translationMatrix_pf;


		//For rectangle
		//initialize above matrices to identity
		modelMatrix_pf = mat4::identity();
		viewMatrix_pf = mat4::identity();
		projectionMatrix_pf = mat4::identity();

		ratationMatrix_pf = mat4::identity();
		ratationMatrix_pf_zero = mat4::identity();
		ratationMatrix_pf_one = mat4::identity();
		ratationMatrix_pf_two = mat4::identity();
		translationMatrix_pf = mat4::identity();

		//do necessary transformation if ant required
		translationMatrix_pf = translate(0.0f, 0.0f, -4.0f);

		ratationMatrix_pf_zero = rotate(LightAngleZero, 1.0f, 0.0f, 0.0f);
		ratationMatrix_pf_one = rotate(LightAngleOne, 0.0f, 1.0f, 0.0f);
		ratationMatrix_pf_two = rotate(LightAngleTwo, 0.0f, 0.0f, 1.0f);

		//do necessary matrix multiplication
		ratationMatrix_pf = ratationMatrix_pf_zero * ratationMatrix_pf_one * ratationMatrix_pf_two;

		modelMatrix_pf = translationMatrix_pf * ratationMatrix_pf;

		projectionMatrix_pf = projectionMatrix_pf * perspectiveProjectionMatrix;

		//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
		//this is internally done  by gluOrho2D/ glOrtho

		//send necessary matrices to shader in respective uniforms
		glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix_pf);

		glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix_pf);
		//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

		glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix_pf);
		//GL_FALSE = internally we are not transposing = row -> col / col -> row
		//OpenGL is col major while DirectX is row major

		//bind with vao(this will avoid many repetitive binding with vbo)
		glBindVertexArray(vao_sphere);

		//similarly bind the textures if any

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

		//draw necessary scene
		glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
		// glDrawArrays(GLenum, GLint , GLsizei)
		// (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

		//unbind va0
		glBindVertexArray(0);

		//unbinding - unused program
		glUseProgram(0);
	
	}

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

	if (vao_sphere)
	{
		glDeleteBuffers(1, &vao_sphere);
		vao_sphere = 0;
	}

	if (vbo_sphere_position)
	{
		glDeleteBuffers(1, &vbo_sphere_position);
		vbo_sphere_position = 0;
	}

	if (vbo_sphere_normal)
	{
		glDeleteBuffers(1, &vbo_sphere_normal);
		vbo_sphere_normal = 0;
	}

	if (vbo_sphere_element)
	{
		glDeleteBuffers(1, &vbo_sphere_element);
		vbo_sphere_element = 0;
	}

	//safe release

	if (ShaderProgramObject_pv)
	{
		glUseProgram(ShaderProgramObject_pv);

		//ask program how many shaders are attached to you

		glGetProgramiv(ShaderProgramObject_pv, GL_ATTACHED_SHADERS, &shaderCount);

		GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);

		if (pShaders)
		{
			glGetAttachedShaders(ShaderProgramObject_pv, shaderCount, &shaderCount, pShaders);

			for (shaderNumber = 0;shaderNumber < shaderCount;shaderNumber++)
			{
				glDetachShader(ShaderProgramObject_pv, pShaders[shaderNumber]);

				//delete detach shaders
				glDeleteShader(pShaders[shaderNumber]);

				pShaders[shaderNumber] = 0;
			}

			free(pShaders);
		}

		glDeleteProgram(ShaderProgramObject_pv);
		ShaderProgramObject_pv = 0;

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
