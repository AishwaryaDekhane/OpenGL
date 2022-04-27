#include <windows.h>
#include <gl/glew.h>
#include <gl/GL.h>
#include<stdio.h>
#include <math.h>
#include "Header.h"
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
bool gbLight;
GLuint gShaderProgramObject;

GLuint vao_cube;
GLuint vbo_cube;

GLfloat angle_cube = 0.0f;
mat4 perspectiveProjectionMatrix;	//mat4 is from vmath

GLuint	Texture_Stone;

GLuint samplerUniform;

GLuint model_uniform;
GLuint view_uniform;
GLuint projection_uniform;

GLuint La_uniform;
GLuint Ld_uniform;
GLuint Ls_uniform;
GLuint lightPosition_uniform;

GLuint Ka_uniform;
GLuint Kd_uniform;
GLuint Ks_uniform;
GLuint shininess_uniform;
GLuint LKeyPressed_Uniform;

GLfloat LightAmbient[4] = { 0.0f,0.0f,0.0f,0.0f };
GLfloat LightDiffuse[4] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat LightSpecular[4] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat LightPosition[4] = { 100.0f,100.0f,100.0f,1.0f };

GLfloat MaterialAmbient[4] = { 0.0f,0.0f,0.0f,0.0f };
GLfloat MaterialDiffuse[4] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat MaterialSpecular[4] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat MaterialShininess[1] = { 128.0f };

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
				update();
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


int initialize(void)
{
	//function declaration
	void uninitialize(void);
	void resize(int, int);
	BOOL load_Textures(GLuint *, TCHAR[]);

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
		"in vec3 vNormal;" \
		"in vec2 vTexcoord;" \

		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform int u_LKeyPressed;" \
		"uniform vec4 u_light_position;" \

		"out vec4 out_color;" \
		"out vec2 out_texcoord;" \
		"out vec3 t_norm;" \
		"out vec3 light_direction;" \
		"out vec3 viewer_vector;" \
		"void main(void)" \
		"{" \
		"if (u_LKeyPressed == 1)" \
		"{" \
		"vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;" \
		"mat3 normal_matrix = mat3(transpose(inverse(u_view_matrix * u_model_matrix)));" \
		"t_norm = normal_matrix * vNormal;" \
		"light_direction = vec3(u_light_position - eye_coordinates);" \
		"viewer_vector = vec3(-eye_coordinates);" \
		"}" \
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
		"out_color = vColor;" \
		"out_texcoord = vTexcoord;" \
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
		"in vec4 out_color;" \
		"in vec2 out_texcoord;" \
		"in vec3 t_norm;" \
		"in vec3 light_direction;" \
		"in vec3 viewer_vector;" \

		"uniform int u_LKeyPressed;" \
		"uniform vec3 u_La;" \
		"uniform vec3 u_Ld;" \
		"uniform vec3 u_Ls;" \
		"uniform vec4 u_light_position;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform float shininess;" \

		"out vec3 phong_ads_light;" \
		"out vec4 FragColor;" \
		"uniform sampler2D u_sampler;" \
		"void main(void)" \
		"{" \
		"if(u_LKeyPressed == 1)" \
		"{" \
		"vec3 normalised_transformed_normal = normalize(t_norm);" \
		"vec3 normalised_light_direction = normalize(light_direction);" \
		"vec3 normalised_viewer_vector = normalize(viewer_vector);" \
		"vec3 reflection_vector = reflect(-normalised_light_direction, normalised_transformed_normal);" \
		"float tn_dot_LightDirection = max(dot(normalised_light_direction, normalised_transformed_normal), 0.0);" \
		"vec3 ambient = u_La * u_Ka;" \
		"vec3 diffuse = u_Ld * u_Kd * tn_dot_LightDirection;" \
		"vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalised_viewer_vector), 0.0), shininess);" \
		"phong_ads_light = (ambient + diffuse + specular) * vec3(out_color * texture(u_sampler, out_texcoord));" \
		"}" \
		"else" \
		"{" \
		"phong_ads_light = vec3(out_color * texture(u_sampler, out_texcoord));" \
		"}" \
		"FragColor =  vec4(phong_ads_light,1.0);" \
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
	gShaderProgramObject = glCreateProgram();

	//add vertex shader
	glAttachShader(gShaderProgramObject, gVertexShaderObject);

	//add fragment shader
	glAttachShader(gShaderProgramObject, gFragmentShaderObject);

	//Prelinking binding to vertex attributes
	glBindAttribLocation(gShaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(gShaderProgramObject, AMC_ATTRIBUTE_TEXCOORD0, "vTexture0_Coord");
	glBindAttribLocation(gShaderProgramObject, AMC_ATTRIBUTE_COLOR, "vColor");
	glBindAttribLocation(gShaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");

	glLinkProgram(gShaderProgramObject);

	//Error checking for shader

	iInfoLogLength = 0;
	szInfoLog = NULL;

	// link shader
	glLinkProgram(gShaderProgramObject);
	GLint iShaderProgramLinkStatus = 0;
	glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if (iShaderProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength>0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}

	//Postlinking retriving uniform locations
	//post linking retriving uniform location
	model_uniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	view_uniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	projection_uniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");

	LKeyPressed_Uniform = glGetUniformLocation(gShaderProgramObject, "u_LKeyPressed");
	La_uniform = glGetUniformLocation(gShaderProgramObject, "u_La");
	Ld_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ld");
	Ls_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ls");
	Ka_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");
	Kd_uniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
	Ks_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");
	shininess_uniform = glGetUniformLocation(gShaderProgramObject, "shininess");
	lightPosition_uniform = glGetUniformLocation(gShaderProgramObject, "u_light_position");

	samplerUniform = glGetUniformLocation(gShaderProgramObject, "u_sampler");

	//declare vertex3f coordinates here

	GLfloat cubeVCNT[] =
	{
		1.0f, 1.0f, -1.0f,    1.0f, 0.0f, 0.0f,    0.0f, 1.0f,    0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, -1.0f,    1.0f, 0.0f, 0.0f,    0.0f, 0.0f,    0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 0.0f,    1.0f, 0.0f,    0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f,    0.0f, 1.0f, 0.0f,

		1.0f, -1.0f, -1.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f,    0.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,0.0f, 1.0f, 0.0f,    0.0f, 1.0f,    0.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f,    0.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 1.0f,    0.0f, 1.0f, 0.0f,    1.0f, 0.0f,    0.0f, -1.0f, 0.0f,

		1.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,    0.0f, 0.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,    0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,    0.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,    0.0f, 0.0f, 1.0f,

		1.0f, 1.0f, -1.0f,    0.0f, 1.0f, 1.0f,    1.0f, 0.0f,    0.0f, 0.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f,    0.0f, 0.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,0.0f, 1.0f, 1.0f,    0.0f, 1.0f,    0.0f, 0.0f, -1.0f,
		1.0f, -1.0f, -1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f,    0.0f, 0.0f, -1.0f,

		1.0f, 1.0f, -1.0f,    1.0f, 0.0f, 1.0f,    1.0f, 0.0f,    1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f,    1.0f, 1.0f,    1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f,    1.0f, 0.0f, 1.0f,    0.0f, 1.0f,    1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 1.0f,    0.0f, 0.0f,    1.0f, 0.0f, 0.0f,

		-1.0f, 1.0f, -1.0f,    1.0f, 1.0f, 0.0f,    0.0f, 0.0f,    -1.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f,    -1.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f,    -1.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,1.0f, 1.0f, 0.0f,    0.0f, 1.0f,    -1.0f, 0.0f, 0.0f
	};

	//--------------------------------------------------------------------------------------------------------------


	//For cube

	glGenVertexArrays(1, &vao_cube);
	glBindVertexArray(vao_cube);

	glGenBuffers(1, &vbo_cube);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube);
	glBufferData(GL_ARRAY_BUFFER, 24 * 11 * sizeof(GLfloat), cubeVCNT, GL_STATIC_DRAW);

	//position
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	//color
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

	//texcoord
	glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD0, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD0);

	//normal
	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	//-----------------------------------------------------------------------------------------------------------------

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

	//for texture
	load_Textures(&Texture_Stone, MAKEINTRESOURCE(IDBITMAP_STONE));

	gbLight = false;

	//warmup call
	resize(WIN_WIDTH, WIN_HEIGHT);

	glEnable(GL_TEXTURE_2D);

	return 0;
}

BOOL load_Textures(GLuint *texture, TCHAR imageResourceId[])
{
	//converting image resource to image data
	HBITMAP hBitmap = NULL;
	BITMAP bmp;
	int bStatus = FALSE;

	//code

	//HANDLE LoadImage(HMODULE,LPCTSTR,UINT,INT,INT,UINT);
	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), imageResourceId, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);	//DIB=Device Independent Bitmap
	if (hBitmap)
	{
		bStatus = TRUE;

		GetObject(hBitmap, sizeof(bmp), &bmp);	//image 2D isin the form of rows and columns

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);	//4=RGBA

		glGenTextures(1, texture);

		glBindTexture(GL_TEXTURE_2D, *texture);

		//mipmap = to reduce load time and memory of image we create multiple images of given render - optimization
		//GL_LINEAR is used for good quality it calculates average and then map out texel center to the pixel center

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	//MAG= magnification of image

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	//MIN= minification of image

																						//instead of gluBuild2DMipmaps we use these two below fuctions
		glTexImage2D(GL_TEXTURE_2D,			//target
			0,								//mipmap level
			GL_RGB,							//opengl internal image format
			bmp.bmWidth,
			bmp.bmHeight,
			0,								//border width
			GL_BGR,							//external image data format/ pixel
			GL_UNSIGNED_BYTE,
			bmp.bmBits);

		glGenerateMipmap(GL_TEXTURE_2D);

		DeleteObject(hBitmap);
	}
	return(bStatus);
}

void update(void)
{
	angle_cube = angle_cube + 0.05f;
	if (angle_cube >= 360.0f)
		angle_cube = angle_cube - 360.0f;

}

void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//binding
	glUseProgram(gShaderProgramObject);

	// OpenGL Drawing
	//declaration of matrices
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 ProjectionMatrix;

	//for rotation
	mat4 ratationMatrix;
	mat4 translationMatrix;

	//if light is enabled
	if (gbLight == true)
	{
		//notify shader that we pressed the "L" key
		glUniform1i(LKeyPressed_Uniform, 1);
		//send light intensityx
		glUniform3fv(La_uniform, 1, LightAmbient);
		glUniform3fv(Ld_uniform, 1, LightDiffuse);
		glUniform3fv(Ls_uniform, 1, LightSpecular);
		//send coeff. of material's reflectivity
		glUniform3fv(Ka_uniform, 1, MaterialAmbient);
		glUniform3fv(Kd_uniform, 1, MaterialDiffuse);
		glUniform3fv(Ks_uniform, 1, MaterialSpecular);
		//shininess
		glUniform1f(shininess_uniform, 1);
		//send light position
		glUniform4fv(lightPosition_uniform, 1, (GLfloat *)LightPosition);
	}
	else
	{
		//notify shader that we aren't pressed the "L" key
		glUniform1i(LKeyPressed_Uniform, 0);
	}

	//For cube
	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	ProjectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	//do necessary transformation if ant required
	translationMatrix = translate(0.0f, 0.0f, -6.0f);

	ratationMatrix = rotate(angle_cube, angle_cube, angle_cube);

	//do necessary matrix multiplication
	modelMatrix = modelMatrix * translationMatrix;
	modelMatrix = modelMatrix * ratationMatrix;
	ProjectionMatrix = perspectiveProjectionMatrix;

	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(model_uniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(view_uniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, ProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture_Stone);
	glUniform1i(samplerUniform, 0);

	//bind with vao(this will avoid many repetitive binding with vbo)
	glBindVertexArray(vao_cube);

	//similarly bind the textures if any

	//draw necessary scene
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
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
	if (Texture_Stone)
	{
		glDeleteTextures(1, &Texture_Stone);
		Texture_Stone = 0;
	}


	if (vao_cube)
	{
		glDeleteBuffers(1, &vao_cube);
		vao_cube = 0;
	}

	if (vbo_cube)
	{
		glDeleteBuffers(1, &vbo_cube);
		vbo_cube = 0;
	}

	//safe release

	if (gShaderProgramObject)
	{
		glUseProgram(gShaderProgramObject);

		//ask program how many shaders are attached to you

		glGetProgramiv(gShaderProgramObject, GL_ATTACHED_SHADERS, &shaderCount);

		GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);

		if (pShaders)
		{
			glGetAttachedShaders(gShaderProgramObject, shaderCount, &shaderCount, pShaders);

			for (shaderNumber = 0;shaderNumber < shaderCount;shaderNumber++)
			{
				glDetachShader(gShaderProgramObject, pShaders[shaderNumber]);

				//delete detach shaders
				glDeleteShader(pShaders[shaderNumber]);

				pShaders[shaderNumber] = 0;
			}

			free(pShaders);
		}

		glDeleteProgram(gShaderProgramObject);
		gShaderProgramObject = 0;

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
