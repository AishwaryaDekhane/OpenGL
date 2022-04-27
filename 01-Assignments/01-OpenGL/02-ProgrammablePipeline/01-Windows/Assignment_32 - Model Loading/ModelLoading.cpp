#include <windows.h>
#include <C:\glew\include\GL\glew.h>
#include <gl/GL.h>
#include<stdio.h>
#include <math.h>
#include "Header.h"
#include "vmath.h"

#pragma comment(lib,"C:\\glew\\lib\\Release\\Win32\\glew32.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")


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

GLuint vao;
GLuint vbo_vertices;
GLuint vbo_normal;
GLuint vbo_texcoord;
GLuint element_buffer_vertices;
GLuint element_buffer_normal;
GLuint element_buffer_texture;

GLuint model_uniform;
GLuint view_uniform;
GLuint projection_uniform;

GLuint texture_marble;
GLuint samplerUniform;

GLuint La_uniform;
GLuint Ld_uniform;
GLuint Ls_uniform;
GLuint lightPosition_uniform;

GLuint Ka_uniform;
GLuint Kd_uniform;
GLuint Ks_uniform;
GLuint shininess_uniform;
GLuint LKeyPressed_Uniform;

vmath::mat4 perspectiveProjectionMatrix;

//light values
float LightAmbient[4] = { 0.25f, 0.25f, 0.25f, 0.25f };
float LightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float LightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float LightPosition[4] = { 100.0f, 100.0f, 100.0f, 1.0f };			//{ 1.0f, 1.0f, 1.0f, 1.0f };

//material values
float MaterialAmbient[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float MaterialDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float MaterialSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float MaterialShininess = 128.0f;							//{128.0f};

//model loading variables
struct vec_int
{
	int *p;
	int size;
};

struct vec_float
{
	float *pf;
	int size;
};

#define BUFFER_SIZE 1024
char buffer[BUFFER_SIZE];

FILE *gpMeshFile = NULL;

struct vec_float *gpVertices = NULL;
struct vec_float *gpTexture = NULL;
struct vec_float *gpNormal = NULL;

struct vec_float *gp_sorted_vertices = NULL;
struct vec_float *gp_sorted_texture = NULL;
struct vec_float *gp_sorted_normal = NULL;

struct vec_int *gp_indices_vertices = NULL;
struct vec_int *gp_indices_texture = NULL;
struct vec_int *gp_indices_normal = NULL;


//Rotation variables
float rotation_angle = 0.0f;

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
	void LoadMesh(void);

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
		"in vec2 vTexcoord;" \
		
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform int u_LKeyPressed;" \
		"uniform vec4 u_light_position;" \

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
		"phong_ads_light = (ambient + diffuse + specular) * vec3(texture(u_sampler, out_texcoord));" \
		"}" \
		"else" \
		"{" \
		"phong_ads_light = vec3(texture(u_sampler, out_texcoord));" \
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
	glBindAttribLocation(gShaderProgramObject, AMC_ATTRIBUTE_TEXCOORD0, "vTexcoord");
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

	//load mesh call
	LoadMesh();

	//For cube

		//create vao and vbo
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	//position
	glGenBuffers(1, &vbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);

	glBufferData(	GL_ARRAY_BUFFER, 
					(gp_sorted_vertices->size) * sizeof(float), 
					gp_sorted_vertices->pf, 
					GL_STATIC_DRAW);

	glVertexAttribPointer(	AMC_ATTRIBUTE_POSITION, 
							3, 
							GL_FLOAT, 
							GL_FALSE, 
							0, 
							NULL);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//normal
	glGenBuffers(1, &vbo_normal);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);

	glBufferData(	GL_ARRAY_BUFFER, 
					(gp_sorted_normal->size) * sizeof(float), 
					gp_sorted_normal->pf, 
					GL_STATIC_DRAW);

	glVertexAttribPointer(	AMC_ATTRIBUTE_NORMAL, 
							3, 
							GL_FLOAT, 
							GL_FALSE, 
							0, 
							NULL);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//texcoord
	glGenBuffers(1, &vbo_texcoord);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord);

	glBufferData(	GL_ARRAY_BUFFER, 
					(gp_sorted_texture->size) * sizeof(float), 
					gp_sorted_texture->pf, 
					GL_STATIC_DRAW);

	glVertexAttribPointer(	AMC_ATTRIBUTE_TEXCOORD0, 
							2, 
							GL_FLOAT, 
							GL_FALSE, 
							0, 
							NULL);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//element buffer
	
	//vertices
	glGenBuffers(1, &element_buffer_vertices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_vertices);
	glBufferData(	GL_ELEMENT_ARRAY_BUFFER,
					gp_indices_vertices->size * sizeof(int),
					gp_indices_vertices->p,
					GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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
	load_Textures(&texture_marble, MAKEINTRESOURCE(IDBITMAP_STONE));

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
	rotation_angle = rotation_angle + 0.05f;
	if (rotation_angle >= 360.0f)
		rotation_angle = rotation_angle - 360.0f;

}

void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//binding
	glUseProgram(gShaderProgramObject);

	vmath::mat4 modelMatrix;
	vmath::mat4 viewMatrix;
	vmath::mat4 projectionMatrix;
	vmath::mat4 translationMatrix;
	vmath::mat4 rotationMatrix;

	//init above metrices to identity
	modelMatrix = vmath::mat4::identity();
	viewMatrix = vmath::mat4::identity();
	projectionMatrix = vmath::mat4::identity();
	translationMatrix = vmath::mat4::identity();
	rotationMatrix = vmath::mat4::identity();

	//do necessary transformations here
	translationMatrix = vmath::translate(0.0f, -1.5f, -10.0f);
	rotationMatrix = vmath::rotate(rotation_angle, 0.0f, 1.0f, 0.0f);

	//do necessary matrix multiplication
	modelMatrix = modelMatrix * translationMatrix;
	modelMatrix = modelMatrix * rotationMatrix;
	projectionMatrix *= perspectiveProjectionMatrix;

	//send necessary matrics to shaders in respective uniforms
	glUniformMatrix4fv(model_uniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(view_uniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, projectionMatrix);

	//active texture
	glActiveTexture(GL_TEXTURE0);

	//bind with texture 
	glBindTexture(GL_TEXTURE_2D, texture_marble);

	//push in fragment shader
	glUniform1i(samplerUniform, 0);

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
		glUniform1f(shininess_uniform, MaterialShininess);
		//send light position
		glUniform4fv(lightPosition_uniform, 1, LightPosition);
	}
	else
	{
		//notify shader that we aren't pressed the "L" key
		glUniform1i(LKeyPressed_Uniform, 0);
	}
	
	glBindVertexArray(vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_vertices);
	glDrawElements(	GL_TRIANGLES,
					(gp_indices_vertices->size),
					GL_UNSIGNED_INT,
					NULL);
	
	glBindVertexArray(0);

	//unuse program
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

	int destroy_vec_int(struct vec_int *p_vec_int); 
	int destroy_vec_float(struct vec_float *p_vec_float);


	if (gbFullscreen == true)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);

	}
	if (texture_marble)
	{
		glDeleteTextures(1, &texture_marble);
		texture_marble = 0;
	}

	if (vbo_vertices)
	{
		glDeleteBuffers(1, &vbo_vertices);
		vbo_vertices = 0;
	}
	if (vbo_texcoord)
	{
		glDeleteBuffers(1, &vbo_texcoord);
		vbo_texcoord = 0;
	}
	if (vbo_normal)
	{
		glDeleteBuffers(1, &vbo_normal);
		vbo_normal = 0;
	}
	if (vao)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}

	if(element_buffer_vertices)
	{
		glDeleteBuffers(1, &element_buffer_vertices); 
		element_buffer_vertices = 0; 
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

	if(gpVertices)
	{
		destroy_vec_float(gpVertices);
		gpVertices = NULL;
	}

	if(gpTexture)
	{
		destroy_vec_float(gpTexture);
		gpTexture = NULL;
	}

	if(gpNormal)
	{
		destroy_vec_float(gpNormal);
		gpNormal = NULL;
	}

	if(gp_sorted_vertices)
	{
		destroy_vec_float(gp_sorted_vertices);
		gp_sorted_vertices = NULL;
	}

	if(gp_sorted_texture)
	{
		destroy_vec_float(gp_sorted_texture);
		gp_sorted_texture = NULL;
	}

	if(gp_sorted_normal)
	{
		destroy_vec_float(gp_sorted_normal);
		gp_sorted_normal = NULL;
	}

	if(gp_indices_vertices)
	{
		destroy_vec_int(gp_indices_vertices);
		gp_indices_vertices = NULL;
	}

	if(gp_indices_texture)
	{
		destroy_vec_int(gp_indices_texture);
		gp_indices_texture = NULL;
	}

	if(gp_indices_normal)
	{
		destroy_vec_int(gp_indices_normal);
		gp_indices_normal = NULL;
	}

	if (gpFile)
	{
		fprintf(gpFile, "Log file is closed successfully \n\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}

void LoadMesh(void)
{
	//function declarations
	struct vec_int *create_vec_int(void);
	struct vec_float *create_vec_float(void);
	int push_back_vec_int(struct vec_int *p_vec_int, int data);
	int push_back_vec_float(struct vec_float *p_vec_float, float data);
	void show_vec_float(struct vec_float *p_vec_float);
	void show_vec_int(struct vec_int *p_vec_int);
	int destroy_vec_float(struct vec_float *p_vec_float);

	//variable declaration
	char *space = " ";
	char *slash = "/";
	char *first_token = NULL;
	char *token = NULL;
	char *f_enteries[3] = { NULL, NULL, NULL };
	
	int nr_vert_cords = 0;
	int nr_tex_cords = 0;
	int nr_norm_cords = 0;
	int nr_faces = 0;

	int i, vi;

	gpMeshFile = fopen("teapot.obj", "r");
	if(gpMeshFile == NULL)
	{
		fprintf(stderr, "error : unable to open obj file\n");
		exit(EXIT_FAILURE);
	}

	gpVertices 	= 	create_vec_float();
	gpTexture 	= 	create_vec_float();
	gpNormal 	=	create_vec_float();

	gp_indices_vertices 	=	create_vec_int();
	gp_indices_texture		=	create_vec_int();
	gp_indices_normal	 	=	create_vec_int();

	while(fgets(buffer, BUFFER_SIZE, gpMeshFile) != NULL)
	{
		first_token = strtok(buffer, space);
		if(strcmp(first_token, "v") == 0)
		{
			nr_vert_cords++;
			while((token = strtok(NULL, space)) != NULL)
			{
				push_back_vec_float(gpVertices, atof(token));
			}

		}

		else if(strcmp(first_token, "vt") == 0)
		{
			nr_tex_cords++;
			while((token = strtok(NULL, space)) != NULL)
			{
				push_back_vec_float(gpTexture, atof(token));
			}
			
		}

		else if(strcmp(first_token, "vn") == 0)
		{
			nr_norm_cords++;
			while((token = strtok(NULL, space)) != NULL)
			{
				push_back_vec_float(gpNormal, atof(token));
			}
			
		}

		else if(strcmp(first_token, "f") == 0)
		{
			nr_faces++;
			for(i = 0; i < 3; i++)
			{
				f_enteries[i] = strtok(NULL, space);
			}

			for(i = 0; i < 3; i++)
			{
				token = strtok(f_enteries[i], slash);
				push_back_vec_int(gp_indices_vertices, atoi(token) - 1);

				token = strtok(NULL, slash);
				push_back_vec_int(gp_indices_texture, atoi(token) - 1);

				token = strtok(NULL, slash);
				push_back_vec_int(gp_indices_normal, atoi(token) - 1);
			}			
		}
	}

	gp_sorted_vertices = create_vec_float();
	for(int i = 0; i < gp_indices_vertices->size; i++)
	{
		push_back_vec_float(gp_sorted_vertices, gpVertices->pf[i]);
	}

	gp_sorted_texture = create_vec_float();
	for(int i = 0; i < gp_indices_texture->size; i++)
	{
		push_back_vec_float(gp_sorted_texture, gpTexture->pf[i]);
	}

	gp_sorted_normal = create_vec_float();
	for(int i = 0; i < gp_indices_normal->size; i++)
	{
		push_back_vec_float(gp_sorted_normal, gpNormal->pf[i]);
	}

	fclose(gpMeshFile);
	gpMeshFile = NULL;
}

struct vec_int *create_vec_int(void)
{
	//code
	struct vec_int *p = (struct vec_int *)malloc(sizeof(struct vec_int));
	if(p != NULL)
	{
		memset(p, 0, sizeof(struct vec_int));
		return (p);
	}
	return(NULL);
}

struct vec_float *create_vec_float(void)
{
	//code
	struct vec_float *p = (struct vec_float *)malloc(sizeof(struct vec_float));
	if(p != NULL)
	{
		memset(p, 0, sizeof(struct vec_float));
		return (p);
	}
	return(NULL);
}
int push_back_vec_int(struct vec_int *p_vec_int, int data)
{
	//code
	p_vec_int->p = (int *)realloc(p_vec_int->p, (p_vec_int->size + 1) * sizeof(int));
	p_vec_int->size = p_vec_int->size + 1;
	p_vec_int->p[p_vec_int->size-1] = data;
	return (0);
}
int push_back_vec_float(struct vec_float *p_vec_float, float data)
{
	//code
	p_vec_float->pf = (float *)realloc(p_vec_float->pf, (p_vec_float->size + 1) * sizeof(float));
	p_vec_float->size = p_vec_float->size + 1;
	p_vec_float->pf[p_vec_float->size-1] = data;
	return (0);
}
void show_vec_float(struct vec_float *p_vec_float)
{
	//code
	int i = 0;
	for(i = 0; i < p_vec_float->size; i++)
	{
		fprintf(gpFile, "%f\n", p_vec_float->pf[i]);
	}
}
void show_vec_int(struct vec_int *p_vec_int)
{
	//code
	int i = 0;
	for(i = 0; i < p_vec_int->size; i++)
	{
		fprintf(gpFile, "%d\n", p_vec_int->p[i]);
	}
}
int destroy_vec_float(struct vec_float *p_vec_float)
{
	//code
	free(p_vec_float->pf);
	free(p_vec_float);
	p_vec_float = NULL;
	return (0);
}

int destroy_vec_int(struct vec_int *p_vec_int)
{
	//code
	free(p_vec_int->p);
	free(p_vec_int);
	p_vec_int = NULL;
	return (0);
}


