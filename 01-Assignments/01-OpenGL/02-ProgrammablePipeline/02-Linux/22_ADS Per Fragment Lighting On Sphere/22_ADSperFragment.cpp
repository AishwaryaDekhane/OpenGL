//headers
#include <iostream>
#include <stdio.h>  //for printf()
#include <stdlib.h> //for exit()
#include <memory.h> //for memset()
#include "Sphere.h"

//headers for XServer
#include <X11/Xlib.h>   //analogous to windows.h
#include <X11/Xutil.h>  //for visuals
#include <X11/XKBlib.h> //XkbKeycodeToKeysym()
#include <X11/keysym.h> //for 'Keysym'

#include <GL/glew.h> // for GLSL extensions IMPORTANT : This Line Should Be Before #include<gl\gl.h>

#include <GL/gl.h>
#include <GL/glx.h> //for 'glx' functions
#include <math.h>
#include "vmath.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define PI 3.141592653589793238463

using namespace vmath;

//global variable declarations
FILE *gpFile = NULL;

enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXTCOORD
};

mat4 perspectiveProjectionMatrix;

Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColormap;
Window gWindow;
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display *, GLXFBConfig, GLXContext, Bool, const int *);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;
GLXFBConfig gGLXFBConfig;
GLXContext gGLXContext; //parallel to HGLRC
int giWindowWidth = 800;
int giWindowHeight = 600;

bool gbFullscreen = false;

GLenum result;

GLuint VertexShaderObject;
GLuint FragmentShaderObject;
GLuint ShaderProgramObject;

GLuint vao_sphere;
GLuint vbo_sphere_position;
GLuint vbo_sphere_normal;
GLuint vbo_sphere_element;

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

GLfloat angle_sphere = 0.0f;

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

//entry-point function
int main(int argc, char *argv[])
{
	//function prototype
	void CreateWindow(void);
	void ToggleFullscreen(void);
	void initialize(void);
	void resize(int, int);
	void display(void);
	void uninitialize(void);
	void update(void);

	static bool bIsAKeyPressed = false;
	static bool bIsLKeyPressed = false;

	//code
	// create log file
	gpFile = fopen("Log.txt", "w");
	if (gpFile == NULL)
	{
		printf("Log File Can Not Be Created. EXitting Now ...\n");
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log File Is Successfully Opened.\n");
	}

	// create the window
	CreateWindow();

	//initialize()
	initialize();

	//Message Loop

	//variable declarations
	XEvent event; //parallel to 'MSG' structure
	KeySym keySym;
	int winWidth;
	int winHeight;
	bool bDone = false;

	while (bDone == false)
	{
		while (XPending(gpDisplay))
		{
			XNextEvent(gpDisplay, &event); //parallel to GetMessage()
			switch (event.type)			   //parallel to 'iMsg'
			{
			case MapNotify: //parallel to WM_CREATE
				break;
			case KeyPress: //parallel to WM_KEYDOWN
				keySym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
				switch (keySym)
				{
				case XK_Escape:
					bDone = true;
					break;

				case XK_F:
				case XK_f:
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

				case XK_A:
				case XK_a:
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

				case XK_L:
				case XK_l:
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

			case ButtonPress:
				switch (event.xbutton.button)
				{
				case 1: //Left Button
					break;
				case 2: //Middle Button
					break;
				case 3: //Right Button
					break;
				default:
					break;
				}
				break;

			case MotionNotify: //parallel to WM_MOUSEMOVE
				break;

			case ConfigureNotify: //parallel to WM_SIZE
				winWidth = event.xconfigure.width;
				winHeight = event.xconfigure.height;
				resize(winWidth, winHeight);
				break;

			case Expose: //parallel to WM_PAINT
				break;

			case DestroyNotify:
				break;

			case 33: //close button, system menu -> close
				bDone = true;
				break;

			default:
				break;
			}
		}

		display();
		if (gbAnimate == true)
		{
			update();
		}
					
	}

	uninitialize();
	return (0);
}

void CreateWindow(void)
{
	//function prototype
	void uninitialize(void);

	//variable declarations
	XSetWindowAttributes winAttribs;
	GLXFBConfig *pGLXFBConfigs = NULL;
	GLXFBConfig bestGLXFBConfig;
	XVisualInfo *pTempXVisualInfo = NULL;
	int iNumFBConfigs = 0;
	int styleMask;
	int i;

	static int frameBufferAttributes[] = {
		GLX_X_RENDERABLE, True,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		GLX_STENCIL_SIZE, 8,
		GLX_DOUBLEBUFFER, True,
		//GLX_SAMPLE_BUFFERS,1,
		//GLX_SAMPLES,4,
		None}; // array must be terminated by 0

	//code
	gpDisplay = XOpenDisplay(NULL);
	if (gpDisplay == NULL)
	{
		printf("ERROR : Unable To Obtain X Display.\n");
		uninitialize();
		exit(1);
	}

	// get a new framebuffer config that meets our attrib requirements
	pGLXFBConfigs = glXChooseFBConfig(gpDisplay,
									  DefaultScreen(gpDisplay), frameBufferAttributes,
									  &iNumFBConfigs);
	if (pGLXFBConfigs == NULL)
	{
		printf("Failed To Get Valid Framebuffer Config. Exitting Now ...\n");
		uninitialize();
		exit(1);
	}
	//printf("%d Matching FB Configs Found.\n",iNumFBConfigs);

	// pick that FB config/visual with the most samples per pixel
	int bestFramebufferconfig = -1, worstFramebufferConfig = -1, bestNumberOfSamples = -1, worstNumberOfSamples = 999;
	for (i = 0; i < iNumFBConfigs; i++)
	{
		pTempXVisualInfo = glXGetVisualFromFBConfig(gpDisplay, pGLXFBConfigs[i]);
		if (pTempXVisualInfo)
		{
			int sampleBuffer, samples;
			glXGetFBConfigAttrib(gpDisplay,
								 pGLXFBConfigs[i], GLX_SAMPLE_BUFFERS, &sampleBuffer);
			glXGetFBConfigAttrib(gpDisplay,
								 pGLXFBConfigs[i], GLX_SAMPLES, &samples);
			//	printf("Matching Framebuffer Config=%d : Visual ID=0x%lu : SAMPLE_BUFFERS=%d : SAMPLES=%d\n",i,pTempXVisualInfo->visualid,sampleBuffer,samples);
			if (bestFramebufferconfig < 0 || sampleBuffer && samples > bestNumberOfSamples)
			{
				bestFramebufferconfig = i;
				bestNumberOfSamples = samples;
			}
			if (worstFramebufferConfig < 0 || !sampleBuffer || samples < worstNumberOfSamples)
			{
				worstFramebufferConfig = i;
				worstNumberOfSamples = samples;
			}
		}
		XFree(pTempXVisualInfo);
	}
	bestGLXFBConfig = pGLXFBConfigs[bestFramebufferconfig];
	// set global GLXFBConfig
	gGLXFBConfig = bestGLXFBConfig;

	// be sure to free FBConfig list allocated by glXChooseFBConfig()
	XFree(pGLXFBConfigs);

	gpXVisualInfo = glXGetVisualFromFBConfig(gpDisplay,
											 bestGLXFBConfig);
	//	printf("Chosen Visual ID=0x%lu\n",gpXVisualInfo->visualid );

	//setting window's attributes
	winAttribs.border_pixel = 0;
	winAttribs.background_pixmap = 0;
	winAttribs.colormap = XCreateColormap(gpDisplay,
										  RootWindow(gpDisplay,
													 gpXVisualInfo->screen), //you can give defaultScreen as well
										  gpXVisualInfo->visual,
										  AllocNone); //for 'movable' memory allocation

	winAttribs.event_mask = StructureNotifyMask | KeyPressMask | ButtonPressMask |
							ExposureMask | VisibilityChangeMask | PointerMotionMask;

	styleMask = CWBorderPixel | CWEventMask | CWColormap;
	gColormap = winAttribs.colormap;

	gWindow = XCreateWindow(gpDisplay,
							RootWindow(gpDisplay, gpXVisualInfo->screen),
							0,
							0,
							WIN_WIDTH,
							WIN_HEIGHT,
							0,					  //border width
							gpXVisualInfo->depth, //depth of visual (depth for Colormap)
							InputOutput,		  //class(type) of your window
							gpXVisualInfo->visual,
							styleMask,
							&winAttribs);
	if (!gWindow)
	{
		printf("Failure In Window Creation.\n");
		uninitialize();
		exit(1);
	}

	XStoreName(gpDisplay, gWindow, "OpenGL Programmable Pipeline Window");

	Atom windowManagerDelete = XInternAtom(gpDisplay, "WM_WINDOW_DELETE", True);
	XSetWMProtocols(gpDisplay, gWindow, &windowManagerDelete, 1);

	XMapWindow(gpDisplay, gWindow);
}

void ToggleFullscreen(void)
{
	//code
	Atom wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False); //normal window state

	XEvent event;
	memset(&event, 0, sizeof(XEvent));

	event.type = ClientMessage;
	event.xclient.window = gWindow;
	event.xclient.message_type = wm_state;
	event.xclient.format = 32; //32-bit
	event.xclient.data.l[0] = gbFullscreen ? 0 : 1;

	Atom fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	event.xclient.data.l[1] = fullscreen;

	//parallel to SendMessage()
	XSendEvent(gpDisplay,
			   RootWindow(gpDisplay, gpXVisualInfo->screen),
			   False,				//do not send this message to Sibling windows
			   StructureNotifyMask, //resizing mask (event_mask)
			   &event);
}

void update(void)
{
	angle_sphere = angle_sphere + 0.05f;
	if (angle_sphere >= 360.0f)
		angle_sphere = angle_sphere - 360.0f;
}

void initialize(void)
{
	// function declarations
	void uninitialize(void);
	void resize(int, int);

	//code
	// create a new GL context 4.5 for rendering
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((GLubyte *)"glXCreateContextAttribsARB");

	GLint attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 5,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		0}; // array must be terminated by 0

	gGLXContext = glXCreateContextAttribsARB(gpDisplay, gGLXFBConfig, 0, True, attribs);

	if (!gGLXContext) // fallback to safe old style 2.x context
	{
		// When a context version below 3.0 is requested, implementations will return
		// the newest context version compatible with OpenGL versions less than version 3.0.
		GLint attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
			GLX_CONTEXT_MINOR_VERSION_ARB, 0,
			0}; // array must be terminated by 0
		printf("Failed To Create GLX 4.5 context. Hence Using Old-Style GLX Context\n");
		gGLXContext = glXCreateContextAttribsARB(gpDisplay, gGLXFBConfig, 0, True, attribs);
	}
	else // successfully created 4.1 context
	{
		printf("OpenGL Context 4.5 Is Created.\n");
	}

	// verifying that context is a direct context
	if (!glXIsDirect(gpDisplay, gGLXContext))
	{
		printf("Indirect GLX Rendering Context Obtained\n");
	}
	else
	{
		printf("Direct GLX Rendering Context Obtained\n");
		printf("helooooo Direct GLX Rendering Context Obtained\n");
	}
	//printf("Before make current\n");
	glXMakeCurrent(gpDisplay, gWindow, gGLXContext);
	//	printf("After make current");
	//code
	// GLEW Initialization Code For GLSL ( IMPORTANT : It Must Be Here.
	//Means After Creating OpenGL Context But Before Using Any OpenGL Function )
	GLenum glew_error = glewInit();
	if (glew_error != GLEW_OK)
	{
		printf("Failure To Initialize GLEW. Exitting Now ...\n");
		uninitialize();
		exit(1);
	}

	//VERTEX SHADER
	//printf("VERTEX SHADER");
	//define vertex shader object
	VertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

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
	glShaderSource(VertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

	//compile the vertex shader
	glCompileShader(VertexShaderObject);

	//Error checking for VS

	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	char *szInfoLog = NULL;

	//step 1
	glGetShaderiv(VertexShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);

	//step 2
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(VertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);

		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);

			if (szInfoLog != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(VertexShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Vertex Shader Error : \n %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();

				exit(0);
			}
		}
	}

	//FRAGMENT SHADER
	//FRAGMENT SHADER
	//	printf("FRAGMENT SHADER");
	//define fragment shader object
	FragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

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

	//color of triangle is mentioned here in above code

	//specify above sourec code to fragment shader object
	glShaderSource(FragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

	//compile the shader
	glCompileShader(FragmentShaderObject);

	//Error checking for FS

	iShaderCompiledStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	glGetShaderiv(FragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);

	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(FragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);

		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);

			if (szInfoLog != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(FragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Fragment Shader Errors : \n %s \n", szInfoLog);
				free(szInfoLog);
				uninitialize();

				exit(0);
			}
		}
	}

	//SHADER PROGRAM
	printf("Creating shader program object\n");

	//create obj
	ShaderProgramObject = glCreateProgram();

	//add vertex shader
	glAttachShader(ShaderProgramObject, VertexShaderObject);

	//add fragment shader
	glAttachShader(ShaderProgramObject, FragmentShaderObject);

	//Prelinking binding to vertex
	glBindAttribLocation(ShaderProgramObject,
						 AMC_ATTRIBUTE_POSITION,
						 "vPosition");
	glBindAttribLocation(ShaderProgramObject,
						 AMC_ATTRIBUTE_NORMAL,
						 "vNormal");

	glLinkProgram(ShaderProgramObject);

	//Error checking for shader

	iInfoLogLength = 0;
	GLint iShaderProgramLinkStatus = 0;
	szInfoLog = NULL;

	glGetProgramiv(ShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);

	if (iShaderProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(ShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);

		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);

			if (szInfoLog != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(ShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Program Link Error : \n %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();

				exit(0);
			}
		}
	}
	//post links
	ModelMatrixUniform_pf = glGetUniformLocation(ShaderProgramObject, "u_model_matrix");
	ProjectionMatrixUniform_pf = glGetUniformLocation(ShaderProgramObject, "u_projection_matrix");
	ViewMatrixUniform_pf = glGetUniformLocation(ShaderProgramObject, "u_view_matrix");

	LKeyPressedUniform_pf = glGetUniformLocation(ShaderProgramObject, "u_LKeyPressed");
	MaterialShininessUniform_pf = glGetUniformLocation(ShaderProgramObject, "u_material_shininess");

	LaUniform_pf = glGetUniformLocation(ShaderProgramObject, "u_La");
	KaUniform_pf = glGetUniformLocation(ShaderProgramObject, "u_Ka");

	LdUniform_pf = glGetUniformLocation(ShaderProgramObject, "u_Ld");
	KdUniform_pf = glGetUniformLocation(ShaderProgramObject, "u_Kd");

	LsUniform_pf = glGetUniformLocation(ShaderProgramObject, "u_Ls");
	KsUniform_pf = glGetUniformLocation(ShaderProgramObject, "u_Ks");
	LightPositionUniform_pf = glGetUniformLocation(ShaderProgramObject, "u_light_position");

	//declare vertex3f coordinates here

	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_element);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();

//-------------------------------------------------------------------------------------------------------------------
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
	//---------------------------------------------------------------------------------------------------------
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	perspectiveProjectionMatrix = mat4::identity();

	// ==============================================================================================================

	// set background clearing color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // black

	gbAnimate = false;
	gbLight = false;

	// resize
	resize(WIN_WIDTH, WIN_HEIGHT);
}

void resize(int width, int height)
{
	if (height == 0)
	{
		height = 1;
	}
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	perspectiveProjectionMatrix = perspective(45.0f, ((GLfloat)width / (GLfloat)height), 0.1f, 100.0f);
}

void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(ShaderProgramObject);

	//---------------------------------------------------------------------------------------------------------------
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

		float lightPosition[] = { 100.0f, 100.0f, 100.0f, 1.0f };
		glUniform4fv(LightPositionUniform_pf, 1, (GLfloat *)lightPosition);
	}
	else
	{
		glUniform1i(LKeyPressedUniform_pf, 0);
	}

	//-------------------------------------------------------------------------------------------------------------------

	
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
	translationMatrix = translate(0.0f, 0.0f, -4.0f);

	ratationMatrix = rotate(angle_sphere, angle_sphere, angle_sphere);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

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
	/*=============================================================================== */

	// stop using OpenGL program object
	glUseProgram(0);

	//to process buffered OpenGL Routines
	glXSwapBuffers(gpDisplay, gWindow);
}


void uninitialize(void)
{
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

	glUseProgram(ShaderProgramObject);

	// detach vertex shader from shader program object
	glDetachShader(ShaderProgramObject, VertexShaderObject);
	// detach fragment  shader from shader program object
	glDetachShader(ShaderProgramObject, FragmentShaderObject);

	// delete vertex shader object
	glDeleteShader(VertexShaderObject);
	VertexShaderObject = 0;
	// delete fragment shader object
	glDeleteShader(FragmentShaderObject);
	FragmentShaderObject = 0;

	// delete shader program object
	glDeleteProgram(ShaderProgramObject);
	ShaderProgramObject = 0;

	// unlink shader program
	glUseProgram(0);

	// Releasing OpenGL related and XWindow related objects
	GLXContext currentContext = glXGetCurrentContext();
	if (currentContext != NULL && currentContext == gGLXContext)
	{
		glXMakeCurrent(gpDisplay, 0, 0);
	}

	if (gGLXContext)
	{
		glXDestroyContext(gpDisplay, gGLXContext);
	}

	if (gWindow)
	{
		XDestroyWindow(gpDisplay, gWindow);
	}

	if (gColormap)
	{
		XFreeColormap(gpDisplay, gColormap);
	}

	if (gpXVisualInfo)
	{
		free(gpXVisualInfo);
		gpXVisualInfo = NULL;
	}

	if (gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay = NULL;
	}

	if (gpFile)
	{
		fprintf(gpFile, "Log File Is Successfully Closed.\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}
