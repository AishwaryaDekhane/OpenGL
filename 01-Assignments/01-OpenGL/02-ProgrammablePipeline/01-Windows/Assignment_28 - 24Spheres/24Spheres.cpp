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
GLint PressButton = 0;

GLuint ShaderProgramObject_pf;

GLuint vao_sphere;
GLuint vbo_sphere_position;
GLuint vbo_sphere_normal;
GLuint vbo_sphere_element;

mat4 perspectiveProjectionMatrix;	//mat4 is from vmath

GLuint ModelMatrixUniform_pf;
GLuint ViewMatrixUniform_pf;
GLuint ProjectionMatrixUniform_pf;
GLuint LaUniform_pf;
GLuint KaUniform_pf;
GLuint LdUniform_pf;
GLuint KdUniform_pf;
GLuint LsUniform_pf;
GLuint KsUniform_pf;
GLuint LightPositionUniform_pf;
GLuint MaterialShininessUniform_pf;
GLuint LKeyPressedUniform_pf;

GLfloat AngleOfXRotation = 0.0f;
GLfloat AngleOfYRotation = 0.0f;
GLfloat AngleOfZRotation = 0.0f;

bool gbAnimate;
bool gbLight;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_element[2280];

GLfloat LightAmbient[4] = { 0.0f,0.0f,0.0f,0.0f };
GLfloat LightDiffuse[4] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat LightSpecular[4] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat LightPosition[4] = { 100.0f,100.0f,100.0f,1.0f };

GLfloat MaterialAmbient[4] = { 0.0f,0.0f,0.0f,0.0f };
GLfloat MaterialDiffuse[4] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat MaterialSpecular[4] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat MaterialShininess[1] = { 128.0f };

int gNumVertices;
int gNumElements;

int width;
int height;

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

				//if (gbAnimate == true)
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

	case WM_CHAR:
		switch (wParam)
		{
		case 'X':
		case 'x':
			PressButton = 1;
			AngleOfXRotation = 0.0f;
			break;

		case 'y':
		case 'Y':
			PressButton = 2;
			AngleOfYRotation = 0.0f;
			break;

		case 'z':
		case 'Z':
			PressButton = 3;
			AngleOfZRotation = 0.0f;
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
	AngleOfXRotation = AngleOfXRotation + 0.005f;
	if (AngleOfXRotation >= 360.0f)
		AngleOfXRotation = AngleOfXRotation - 360.0f;

	AngleOfYRotation = AngleOfYRotation + 0.005f;
	if (AngleOfYRotation >= 360.0f)
		AngleOfYRotation = AngleOfYRotation - 360.0f;

	AngleOfZRotation = AngleOfZRotation + 0.005f;
	if (AngleOfZRotation >= 360.0f)
		AngleOfZRotation = AngleOfZRotation - 360.0f;

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
		"uniform vec4 u_light_position;" \
		"out vec3 phong_ADS_light;" \
		"out vec3 tNorm;" \
		"out vec3 light_direction;" \
		"out vec3 view_vector;" \

		"void main(void)" \
		"{" \
		"if (u_LKeyPressed == 1)" \
		"{" \
		"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;" \
		"tNorm = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
		"light_direction = vec3(u_light_position - eyeCoordinates);" \
		"vec3 viewer_vector = vec3(-eyeCoordinates.xyz);" \
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
		"uniform vec3 u_La;" \
		"uniform vec3 u_Ld;" \
		"uniform vec3 u_Ls;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform float u_material_shininess;" \
		"in vec3 phong_ADS_light;" \
		"in vec3 tNorm;" \
		"in vec3 light_direction;" \
		"in vec3 view_vector;" \
		"out vec4 FragColor;" \
		"uniform int u_LKeyPressed;" \

		"void main(void)" \
		"{" \
		"vec4 color;" \
		"if (u_LKeyPressed == 1)" \
		"{" \
		"vec3 normalized_tNorm = normalize(tNorm);"
		"vec3 normalized_light_direction = normalize(light_direction);" \
		"vec3 normalized_view_vector = normalize(view_vector);" \
		"vec3 reflection_vector = reflect(-normalized_light_direction,normalized_tNorm);" \
		"float tn_dot_ld = max(dot(normalized_light_direction.xyz,normalized_tNorm),0.0f);" \
		"vec3 ambient = u_La * u_Ka;" \
		"vec3 diffuse = u_Ld  * u_Kd * tn_dot_ld;" \
		"vec3 specular = u_Ls  * u_Ks * pow(max(dot(reflection_vector,normalized_view_vector),0.0f),u_material_shininess);" \
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
	ShaderProgramObject_pf = glCreateProgram();

	//add vertex shader
	glAttachShader(ShaderProgramObject_pf, gVertexShaderObject);

	//add fragment shader
	glAttachShader(ShaderProgramObject_pf, gFragmentShaderObject);

	//Prelinking binding to vertex attributes
	glBindAttribLocation(ShaderProgramObject_pf, AMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(ShaderProgramObject_pf, AMC_ATTRIBUTE_NORMAL, "vNormal");

	glLinkProgram(ShaderProgramObject_pf);

	//Error checking for shader

	iInfoLogLength = 0;
	szInfoLog = NULL;

	glGetProgramiv(ShaderProgramObject_pf, GL_LINK_STATUS, &iProgramLinkStatus);

	if (iProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(ShaderProgramObject_pf, GL_INFO_LOG_LENGTH, &iInfoLogLength);

		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);

			if (szInfoLog != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(ShaderProgramObject_pf, iInfoLogLength, &written, szInfoLog);
				fprintf_s(gpFile, "Program Link Error : \n %s\n", szInfoLog);
				free(szInfoLog);
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

	LaUniform_pf = glGetUniformLocation(ShaderProgramObject_pf, "u_La");
	KaUniform_pf = glGetUniformLocation(ShaderProgramObject_pf, "u_Ka");

	LdUniform_pf = glGetUniformLocation(ShaderProgramObject_pf, "u_Ld");
	KdUniform_pf = glGetUniformLocation(ShaderProgramObject_pf, "u_Kd");

	LsUniform_pf = glGetUniformLocation(ShaderProgramObject_pf, "u_Ls");
	KsUniform_pf = glGetUniformLocation(ShaderProgramObject_pf, "u_Ks");
	LightPositionUniform_pf = glGetUniformLocation(ShaderProgramObject_pf, "u_light_position");;

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
	glClearColor(0.25f, 0.25f, 0.25f, 0.0f);


	//For Depth

	//glShadeModel(GL_SMOOTH);

	glClearDepth(1.0f);	//give existance to depth buffer

	glEnable(GL_DEPTH_TEST);	//for 3D geometry enable depth test i.e z axis 

	glDepthFunc(GL_LEQUAL);		//LEQUAL = less than or equal to test compared to the max value which is set to 1.0f

								//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

								//for animation - rotation
	glEnable(GL_CULL_FACE);

	//identity() is in mat4
	perspectiveProjectionMatrix = mat4::identity();

	gbAnimate = false;
	gbLight = false;

	//warmup call
	resize(WIN_WIDTH, WIN_HEIGHT);

	return 0;
}

void Draw24Spheres(void)
{
	void resize(int, int);


	GLfloat radius = 100.0f;
	// OpenGL Drawing
	//declaration of matrices
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;

	//for rotation
	mat4 ratationMatrix;
	mat4 translationMatrix;

	//-------------------- 1st sphere on 1st column, emerald ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.0215f;	//r
	MaterialAmbient[1] = 0.1745f;	//g
	MaterialAmbient[2] = 0.0215f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.07568f;	//r
	MaterialDiffuse[1] = 0.61424f;	//g
	MaterialDiffuse[2] = 0.07568f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.633f;	//r
	MaterialSpecular[1] = 0.727811f;	//g
	MaterialSpecular[2] = 0.633f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.6f * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);


		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(-3.5f, 2.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 2nd sphere on 1st column, jade ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.135f;	//r
	MaterialAmbient[1] = 0.2225f;	//g
	MaterialAmbient[2] = 0.1575f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.54f;	//r
	MaterialDiffuse[1] = 0.89f;	//g
	MaterialDiffuse[2] = 0.63f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.316228f;	//r
	MaterialSpecular[1] = 0.316228f;	//g
	MaterialSpecular[2] = 0.316228f; //b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.1f * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);


		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(-3.5f, 1.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 3rd sphere on 1st column, obsidian ------------------------------
	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.05375f;	//r
	MaterialAmbient[1] = 0.05f;	//g
	MaterialAmbient[2] = 0.06625f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.18275f;	//r
	MaterialDiffuse[1] = 0.17f;	//g
	MaterialDiffuse[2] = 0.22525f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.332741f;	//r
	MaterialSpecular[1] = 0.328634f;	//g
	MaterialSpecular[2] = 0.346435f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.3f * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(-3.5f, 0.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 4th sphere on 1st column, pearl ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.25f;	//r
	MaterialAmbient[1] = 0.20725f;	//g
	MaterialAmbient[2] = 0.20725f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 1.0f;	//r
	MaterialDiffuse[1] = 0.829f;	//g
	MaterialDiffuse[2] = 0.829f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.296648f;	//r
	MaterialSpecular[1] = 0.296648f;	//g
	MaterialSpecular[2] = 0.296648f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.088f * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(-3.5f, -0.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 5th sphere on 1st column, ruby ------------------------------
	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.1745f;	//r
	MaterialAmbient[1] = 0.01175f;	//g
	MaterialAmbient[2] = 0.01175f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.61424f;	//r
	MaterialDiffuse[1] = 0.04136f;	//g
	MaterialDiffuse[2] = 0.04136f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.727811f;	//r
	MaterialSpecular[1] = 0.626959f;	//g
	MaterialSpecular[2] = 0.626959f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.6f * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(-3.5f, -1.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 6th sphere on 1st column, turquoise ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.1f;	//r
	MaterialAmbient[1] = 0.18725f;	//g
	MaterialAmbient[2] = 0.1745f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.396f;	//r
	MaterialDiffuse[1] = 0.74151f;	//g
	MaterialDiffuse[2] = 0.69102f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.297254f;	//r
	MaterialSpecular[1] = 0.30829f;	//g
	MaterialSpecular[2] = 0.306678f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.1f * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(-3.5f, -2.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 7- 1st sphere on 2nd column, brass ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.329412f;	//r
	MaterialAmbient[1] = 0.223529f;	//g
	MaterialAmbient[2] = 0.027451f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.780392f;	//r
	MaterialDiffuse[1] = 0.568627f;	//g
	MaterialDiffuse[2] = 0.113725f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.992157f;	//r
	MaterialSpecular[1] = 0.941176f;	//g
	MaterialSpecular[2] = 0.807843f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.21794872f * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(-1.3f, 2.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 8- 2nd sphere on 2nd column, bronze ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.2125f;	//r
	MaterialAmbient[1] = 0.1275f;	//g
	MaterialAmbient[2] = 0.054f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.714f;	//r
	MaterialDiffuse[1] = 0.4284f;	//g
	MaterialDiffuse[2] = 0.18144f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.393548f;	//r
	MaterialSpecular[1] = 0.271906f;	//g
	MaterialSpecular[2] = 0.166721f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.2f * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(-1.3f, 1.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 9- 3rd sphere on 2nd column, chrome -----------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.25f;	//r
	MaterialAmbient[1] = 0.25f;	//g
	MaterialAmbient[2] = 0.25f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.4f;	//r
	MaterialDiffuse[1] = 0.4f;	//g
	MaterialDiffuse[2] = 0.4f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.774597f;	//r
	MaterialSpecular[1] = 0.774597f;	//g
	MaterialSpecular[2] = 0.774597f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.6f * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(-1.3f, 0.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 10- 4th sphere on 2nd column, copper -----------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.19125f;	//r
	MaterialAmbient[1] = 0.0735f;	//g
	MaterialAmbient[2] = 0.0225f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.7038f;	//r
	MaterialDiffuse[1] = 0.27048f;	//g
	MaterialDiffuse[2] = 0.0828f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.256777f;	//r
	MaterialSpecular[1] = 0.137622f;	//g
	MaterialSpecular[2] = 0.086014f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.1f * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(-1.3f, -0.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 11- 5th sphere on 2nd column, gold ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.24725f;	//r
	MaterialAmbient[1] = 0.1995f;	//g
	MaterialAmbient[2] = 0.0745f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.75164f;	//r
	MaterialDiffuse[1] = 0.60648f;	//g
	MaterialDiffuse[2] = 0.22648f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.628281f;	//r
	MaterialSpecular[1] = 0.555802f;	//g
	MaterialSpecular[2] = 0.366065f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.4f * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(-1.3f, -1.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 12- 6th sphere on 2nd column, silver ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.19225f;	//r
	MaterialAmbient[1] = 0.19225f;	//g
	MaterialAmbient[2] = 0.19225f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.50754f;	//r
	MaterialDiffuse[1] = 0.50754f;	//g
	MaterialDiffuse[2] = 0.50754f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.508273f;	//r
	MaterialSpecular[1] = 0.508273f;	//g
	MaterialSpecular[2] = 0.508273f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.4f * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(-1.3f, -2.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 13- 1st sphere on 3rd column, black plastic ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.01f;	//r
	MaterialDiffuse[1] = 0.01f;	//g
	MaterialDiffuse[2] = 0.01f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.50f;	//r
	MaterialSpecular[1] = 0.50f;	//g
	MaterialSpecular[2] = 0.50f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.25 * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(1.3f, 2.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 14- 2nd sphere on 3rd column, cyan plastic ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.1f;	//g
	MaterialAmbient[2] = 0.06f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.0f;	//r
	MaterialDiffuse[1] = 0.50980392f;	//g
	MaterialDiffuse[2] = 0.50980392f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.50196078f;	//r
	MaterialSpecular[1] = 0.50196078f;	//g
	MaterialSpecular[2] = 0.50196078f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.25 * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(1.3f, 1.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 15- 3rd sphere on 2nd column, green plastic------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.1f;	//r
	MaterialDiffuse[1] = 0.35f;	//g
	MaterialDiffuse[2] = 0.1f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.45f;	//r
	MaterialSpecular[1] = 0.55f;	//g
	MaterialSpecular[2] = 0.45f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.25 * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(1.3f, 0.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 16- 4th sphere on 3rd column, red plastic------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.5f;	//r
	MaterialDiffuse[1] = 0.0f;	//g
	MaterialDiffuse[2] = 0.0f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.7f;	//r
	MaterialSpecular[1] = 0.6f;	//g
	MaterialSpecular[2] = 0.6f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.25 * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(1.3f, -0.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 17- 5th sphere on 3rd column, white plastic------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.02f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.55f;	//r
	MaterialDiffuse[1] = 0.55f;	//g
	MaterialDiffuse[2] = 0.55f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.70f;	//r
	MaterialSpecular[1] = 0.70f;	//g
	MaterialSpecular[2] = 0.70f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.25 * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(1.3f, -1.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 18- 6th sphere on 3rd column, yellow plastic ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.5f;	//r
	MaterialDiffuse[1] = 0.5f;	//g
	MaterialDiffuse[2] = 0.0f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.60f;	//r
	MaterialSpecular[1] = 0.60f;	//g
	MaterialSpecular[2] = 0.50f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.25 * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(1.3f, -2.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 19- 1st sphere on 4th column, black rubber ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.02f;	//r
	MaterialAmbient[1] = 0.02f;	//g
	MaterialAmbient[2] = 0.02f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.01f;	//r
	MaterialDiffuse[1] = 0.01f;	//g
	MaterialDiffuse[2] = 0.01f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.4f;	//r
	MaterialSpecular[1] = 0.4f;	//g
	MaterialSpecular[2] = 0.4f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.078125 * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(3.5f, 2.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 20- 2nd sphere on 4th column, cyan rubber ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.05f;	//g
	MaterialAmbient[2] = 0.05f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.4f;	//r
	MaterialDiffuse[1] = 0.5f;	//g
	MaterialDiffuse[2] = 0.5f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.04f;	//r
	MaterialSpecular[1] = 0.7f;	//g
	MaterialSpecular[2] = 0.7f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.078125 * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(3.5f, 1.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 21- 3rd sphere on 4th column, green rubber ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.05f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.4f;	//r
	MaterialDiffuse[1] = 0.5f;	//g
	MaterialDiffuse[2] = 0.4f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.04f;	//r
	MaterialSpecular[1] = 0.7f;	//g
	MaterialSpecular[2] = 0.04f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.078125 * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(3.5f, 0.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 22- 4th sphere on 4th column, red rubber------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.05f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.5f;	//r
	MaterialDiffuse[1] = 0.4f;	//g
	MaterialDiffuse[2] = 0.4f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.7f;	//r
	MaterialSpecular[1] = 0.04f;	//g
	MaterialSpecular[2] = 0.04f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.078125 * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(3.5f, -0.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 23- 5th sphere on 4th column, white ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.05f;	//r
	MaterialAmbient[1] = 0.05f;	//g
	MaterialAmbient[2] = 0.05f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.5f;	//r
	MaterialDiffuse[1] = 0.5f;	//g
	MaterialDiffuse[2] = 0.5f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.7f;	//r
	MaterialSpecular[1] = 0.7f;	//g
	MaterialSpecular[2] = 0.7f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.078125 * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(3.5f, -1.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

	//------------------------------------------------------------------------------------
	//-------------------- 24- 6th sphere on 4th column, yellow rubber ------------------------------

	//initialize above matrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	MaterialAmbient[0] = 0.05f;	//r
	MaterialAmbient[1] = 0.05f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	MaterialDiffuse[0] = 0.5f;	//r
	MaterialDiffuse[1] = 0.5f;	//g
	MaterialDiffuse[2] = 0.4f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	MaterialSpecular[0] = 0.7f;	//r
	MaterialSpecular[1] = 0.7f;	//g
	MaterialSpecular[2] = 0.04f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	MaterialShininess[0] = 0.078125 * 128;

	//for lights
	if (gbLight == true)
	{
		glUniform1i(LKeyPressedUniform_pf, 1);
		glUniform1f(MaterialShininessUniform_pf, 1);

		glUniform3fv(LaUniform_pf, 1, LightAmbient);
		glUniform3fv(KaUniform_pf, 1, MaterialAmbient);

		glUniform3fv(LdUniform_pf, 1, LightDiffuse);
		glUniform3fv(KdUniform_pf, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_pf, 1, LightSpecular);
		glUniform3fv(KsUniform_pf, 1, MaterialSpecular);

		if (PressButton == 1)
		{
			ratationMatrix = rotate(AngleOfXRotation, 1.0f, 0.0f, 0.0f);

			LightPosition[0] = 0.0f;
			LightPosition[1] = (GLfloat)cos(AngleOfXRotation) * radius;
			LightPosition[2] = (GLfloat)sin(AngleOfXRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 2)
		{
			ratationMatrix = rotate(AngleOfYRotation, 0.0f, 1.0f, 0.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfYRotation) * radius;
			LightPosition[1] = 0.0f;
			LightPosition[2] = (GLfloat)sin(AngleOfYRotation) * radius;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
		else if (PressButton == 3)
		{
			ratationMatrix = rotate(AngleOfZRotation, 0.0f, 0.0f, 1.0f);

			LightPosition[0] = (GLfloat)cos(AngleOfZRotation) * radius;
			LightPosition[1] = (GLfloat)sin(AngleOfZRotation) * radius;
			LightPosition[2] = 0.0f;
			LightPosition[3] = 0.0f;

			glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)LightPosition);
		}
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//do necessary transformation if ant required
	translationMatrix = translate(3.5f, -2.5f, -11.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform_pf, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform_pf, 1, GL_FALSE, viewMatrix);
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

}

void display(void)
{
	void Draw24Spheres(void);

	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//binding
	glUseProgram(ShaderProgramObject_pf);

	Draw24Spheres();

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

	if (ShaderProgramObject_pf)
	{
		glUseProgram(ShaderProgramObject_pf);

		//ask program how many shaders are attached to you

		glGetProgramiv(ShaderProgramObject_pf, GL_ATTACHED_SHADERS, &shaderCount);

		GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);

		if (pShaders)
		{
			glGetAttachedShaders(ShaderProgramObject_pf, shaderCount, &shaderCount, pShaders);

			for (shaderNumber = 0;shaderNumber < shaderCount;shaderNumber++)
			{
				glDetachShader(ShaderProgramObject_pf, pShaders[shaderNumber]);

				//delete detach shaders
				glDeleteShader(pShaders[shaderNumber]);

				pShaders[shaderNumber] = 0;
			}

			free(pShaders);
		}

		glDeleteProgram(ShaderProgramObject_pf);
		ShaderProgramObject_pf = 0;

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
