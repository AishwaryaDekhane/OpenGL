//headers
#include <iostream>
#include <stdio.h>  //for printf()
#include <stdlib.h> //for exit()
#include <memory.h> //for memset()

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

GLuint mvpUniform;
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

GLuint vao_red;
GLuint vao_green;
GLuint vao_blue;
GLuint vbo_red_line_position;
GLuint vbo_red_line_color;
GLuint vbo_green_line_position;
GLuint vbo_green_line_color;
GLuint vbo_blue_line_position;
GLuint vbo_blue_line_color;

GLuint vao_triangle;
GLuint vbo_triangle_pos;
GLuint vbo_triangle_col;

GLuint vao_rectangle;
GLuint vbo_rectangle_pos;
GLuint vbo_rectangle_col;

GLuint vao_outer_circle;
GLuint vbo_outer_circle_pos;
GLuint vbo_outer_circle_col;

GLuint vao_inner_circle;
GLuint vbo_inner_circle_pos;
GLuint vbo_inner_circle_col;

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

void initialize(void)
{
	// function declarations
	void uninitialize(void);
	void resize(int, int);
	void initializeTriangle();
	void initializeReactangle();
	void initializeOuterCircle();
	void initializeInnerCircle();

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
		"#version 140 "
		"\n"
		"in vec4 vPosition;"
		"in vec4 vColor;"
		"uniform mat4 u_mvp_matrix;"
		"out vec4 out_color;"
		"void main(void)"
		"{"
		"gl_Position = u_mvp_matrix*vPosition;"
		"out_color = vColor;"
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
		"#version 140 \n"
		"in vec4 out_color;"
		"out vec4 FragColor;"
		"void main(void)"
		"{"
		"FragColor = out_color;"
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
						 AMC_ATTRIBUTE_COLOR,
						 "vColor");

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
	mvpUniform = glGetUniformLocation(ShaderProgramObject, "u_mvp_matrix");

//-------------------------------------------------------------------------------------------------------------------
//declare vertex3f coordinates here
	//line vertices declaration
	const GLfloat blueLines[] =
	{
		-0.95f, 1.0f, 0.0f,
		-0.95f, -1.0f, 0.0f,

		-0.90f, 1.0f, 0.0f,
		-0.90f, -1.0f, 0.0f,

		-0.85f, 1.0f, 0.0f,
		-0.85f, -1.0f, 0.0f,

		-0.80f, 1.0f, 0.0f,
		-0.80f, -1.0f, 0.0f,

		-0.75f, 1.0f, 0.0f,
		-0.75f, -1.0f, 0.0f,

		-0.70f, 1.0f, 0.0f,
		-0.70f, -1.0f, 0.0f,

		-0.65f, 1.0f, 0.0f,
		-0.65f, -1.0f, 0.0f,

		-0.60f, 1.0f, 0.0f,
		-0.60f, -1.0f, 0.0f,

		-0.55f, 1.0f, 0.0f,
		-0.55f, -1.0f, 0.0f,

		-0.50f, 1.0f, 0.0f,
		-0.50f, -1.0f, 0.0f,

		-0.45f, 1.0f, 0.0f,
		-0.45f, -1.0f, 0.0f,

		-0.40f, 1.0f, 0.0f,
		-0.40f, -1.0f, 0.0f,

		-0.35f, 1.0f, 0.0f,
		-0.35f, -1.0f, 0.0f,

		-0.30f, 1.0f, 0.0f,
		-0.30f, -1.0f, 0.0f,

		-0.25f, 1.0f, 0.0f,
		-0.25f, -1.0f, 0.0f,

		-0.20f, 1.0f, 0.0f,
		-0.20f, -1.0f, 0.0f,

		-0.15f, 1.0f, 0.0f,
		-0.15f, -1.0f, 0.0f,

		-0.10f, 1.0f, 0.0f,
		-0.10f, -1.0f, 0.0f,

		-0.05f, 1.0f, 0.0f,
		-0.05f, -1.0f, 0.0f,

		0.95f, 1.0f, 0.0f,
		0.95f, -1.0f, 0.0f,

		0.90f, 1.0f, 0.0f,
		0.90f, -1.0f, 0.0f,

		0.85f, 1.0f, 0.0f,
		0.85f, -1.0f, 0.0f,

		0.80f, 1.0f, 0.0f,
		0.80f, -1.0f, 0.0f,

		0.75f, 1.0f, 0.0f,
		0.75f, -1.0f, 0.0f,

		0.70f, 1.0f, 0.0f,
		0.70f, -1.0f, 0.0f,

		0.65f, 1.0f, 0.0f,
		0.65f, -1.0f, 0.0f,

		0.60f, 1.0f, 0.0f,
		0.60f, -1.0f, 0.0f,

		0.55f, 1.0f, 0.0f,
		0.55f, -1.0f, 0.0f,

		0.50f, 1.0f, 0.0f,
		0.50f, -1.0f, 0.0f,

		0.45f, 1.0f, 0.0f,
		0.45f, -1.0f, 0.0f,

		0.40f, 1.0f, 0.0f,
		0.40f, -1.0f, 0.0f,

		0.35f, 1.0f, 0.0f,
		0.35f, -1.0f, 0.0f,

		0.30f, 1.0f, 0.0f,
		0.30f, -1.0f, 0.0f,

		0.25f, 1.0f, 0.0f,
		0.25f, -1.0f, 0.0f,

		0.20f, 1.0f, 0.0f,
		0.20f, -1.0f, 0.0f,

		0.15f, 1.0f, 0.0f,
		0.15f, -1.0f, 0.0f,

		0.10f, 1.0f, 0.0f,
		0.10f, -1.0f, 0.0f,

		0.05f, 1.0f, 0.0f,
		0.05f, -1.0f, 0.0f,

		1.0f, -0.95f, 0.0f,
		-1.0f, -0.95, 0.0f,

		1.0f, -0.90f, 0.0f,
		-1.0f, -0.90f, 0.0f,

		1.0f, -0.85f, 0.0f,
		-1.0f, -0.85f, 0.0f,

		1.0f, -0.80f, 0.0f,
		-1.0f, -0.80f, 0.0f,

		1.0f, -0.75f, 0.0f,
		-1.0f, -0.75f, 0.0f,

		1.0f, -0.70f, 0.0f,
		-1.0f, -0.70f, 0.0f,

		1.0f, -0.65f, 0.0f,
		-1.0f, -0.65f, 0.0f,

		1.0f, -0.60f, 0.0f,
		-1.0f, -0.60f, 0.0f,

		1.0f, -0.55f, 0.0f,
		-1.0f, -0.55f, 0.0f,

		1.0f, -0.50f, 0.0f,
		-1.0f, -0.50f, 0.0f,

		1.0f, -0.45f, 0.0f,
		-1.0f, -0.45f, 0.0f,

		1.0f, -0.40f, 0.0f,
		-1.0f, -0.40f, 0.0f,

		1.0f, -0.35f, 0.0f,
		-1.0f, -0.35f, 0.0f,

		1.0f, -0.30f, 0.0f,
		-1.0f, -0.30f, 0.0f,

		1.0f, -0.25f, 0.0f,
		-1.0f, -0.25f, 0.0f,

		1.0f, -0.20f, 0.0f,
		-1.0f, -0.20f, 0.0f,

		1.0f, -0.15f, 0.0f,
		-1.0f, -0.15f, 0.0f,

		1.0f, -0.10f, 0.0f,
		-1.0f, -0.10f, 0.0f,

		1.0f, -0.05f, 0.0f,
		-1.0f, -0.05f, 0.0f,

		1.0f, 0.95f, 0.0f,
		-1.0f, 0.95f, 0.0f,

		1.0f, 0.90f, 0.0f,
		-1.0f, 0.90f, 0.0f,

		1.0f, 0.85f, 0.0f,
		-1.0f, 0.85f, 0.0f,

		1.0f, 0.80f, 0.0f,
		-1.0f, 0.80f, 0.0f,

		1.0f, 0.75f, 0.0f,
		-1.0f, 0.75f, 0.0f,

		1.0f, 0.70f, 0.0f,
		-1.0f, 0.70f, 0.0f,

		1.0f, 0.65f, 0.0f,
		-1.0f, 0.65f, 0.0f,

		1.0f, 0.60f, 0.0f,
		-1.0f, 0.60f, 0.0f,

		1.0f, 0.55f, 0.0f,
		-1.0f, 0.55f, 0.0f,

		1.0f, 0.50f, 0.0f,
		-1.0f, 0.50f, 0.0f,

		1.0f, 0.45f, 0.0f,
		-1.0f, 0.45f, 0.0f,

		1.0f, 0.40f, 0.0f,
		-1.0f, 0.40f, 0.0f,

		1.0f, 0.35f, 0.0f,
		-1.0f, 0.35f, 0.0f,

		1.0f, 0.30f, 0.0f,
		-1.0f, 0.30f, 0.0f,

		1.0f, 0.25f, 0.0f,
		-1.0f, 0.25f, 0.0f,

		1.0f, 0.20f, 0.0f,
		-1.0f, 0.20f, 0.0f,

		1.0f, 0.15f, 0.0f,
		-1.0f, 0.15f, 0.0f,

		1.0f, 0.10f, 0.0f,
		-1.0f, 0.10f, 0.0f,

		1.0f, 0.05f, 0.0f,
		-1.0f, 0.05f, 0.0f
	};

	const GLfloat redLine[] =
	{
		1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
	};

	const GLfloat greenLine[] =
	{
		0.0f, 1.0f, 0.0f,
		0.0f, -1.0f, 0.0f
	};

	//color buffers
	const GLfloat redColor[] =
	{
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f
	};
	const GLfloat greenColor[] =
	{
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	//--------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------
	//create vao and vbo
	glGenVertexArrays(1, &vao_green);
	glBindVertexArray(vao_green);

	//green
	glGenBuffers(1, &vbo_green_line_position);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_green_line_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(greenLine), greenLine, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_green_line_color);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_green_line_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(greenColor), greenColor, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	//red
	glGenVertexArrays(1, &vao_red);
	glBindVertexArray(vao_red);

	glGenBuffers(1, &vbo_red_line_position);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_red_line_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(redLine), redLine, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_red_line_color);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_red_line_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(redColor), redColor, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//blue
	glGenVertexArrays(1, &vao_blue);
	glBindVertexArray(vao_blue);

	glGenBuffers(1, &vbo_blue_line_position);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_blue_line_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(blueLines), blueLines, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glVertexAttrib3f(AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);

	glBindVertexArray(0);

//--------------------------------------------------------------------------------------------------------------
	initializeOuterCircle();
	initializeInnerCircle();
	initializeTriangle();
	initializeReactangle();
	
	
//-------------------------------------------------------------------------------------------------------------------
	glShadeModel(GL_SMOOTH);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	perspectiveProjectionMatrix = mat4::identity();

	// ==============================================================================================================

	// set background clearing color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // black

	// resize
	resize(WIN_WIDTH, WIN_HEIGHT);
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

	const GLfloat triangle_col[]=
	{
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
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

void initializeReactangle()
{
	GLfloat reactanglepos[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
	};

	GLfloat rectangleCol[] = {
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
	};

	glGenVertexArrays(1, &vao_rectangle);
	glBindVertexArray(vao_rectangle);

	glGenBuffers(1, &vbo_rectangle_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_rectangle_pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(reactanglepos),reactanglepos,GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_rectangle_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_rectangle_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleCol), rectangleCol, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

}

void initializeOuterCircle()
{
	GLfloat outerCircleVertex[10];
	GLfloat radius_outer = 0.71;

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

	GLfloat outerCircleCol[] = {
		1.0f,1.0f,0.0f,
		1.0f,1.0f,0.0f,

		1.0f,1.0f,0.0f,
		1.0f,1.0f,0.0f,

		1.0f,1.0f,0.0f,
		1.0f,1.0f,0.0f,
	};

	glGenVertexArrays(1, &vao_outer_circle);
	glBindVertexArray(vao_outer_circle);

	for (GLfloat angle = 0.0f; angle < (2.0f * PI); angle = angle + 0.01f)
	{
		outerCircleVertex[0] = ((cos(angle) * radius_outer));
		outerCircleVertex[1] = ((sin(angle) * radius_outer));
		outerCircleVertex[2] = 0.0f;
	}

	glGenBuffers(1, &vbo_outer_circle_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_outer_circle_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(outerCircleVertex), outerCircleVertex, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_outer_circle_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_outer_circle_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(outerCircleCol), outerCircleCol, GL_STATIC_DRAW);
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
		1.0f,1.0f,0.0f,
		1.0f,1.0f,0.0f,

		1.0f,1.0f,0.0f,
		1.0f,1.0f,0.0f,

		1.0f,1.0f,0.0f,
		1.0f,1.0f,0.0f,
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
	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;
	mat4 translationMatrix;
	mat4 rotationMatrix;

	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(ShaderProgramObject);

	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();

	//Do necessary transformation
	modelViewMatrix = translate(0.0f, 0.0f, -1.2f);

	//Do necessary matrix multiplication
	//modelViewMatrix = translationMatrix * rotationMatrix;
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform,
					   1,
					   GL_FALSE,
					   modelViewProjectionMatrix);

	//Bind with vao (This will avoid many repeatation bindings with vbo)
		//bind with vao
	glBindVertexArray(vao_red);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);

	//unbind vao
	glBindVertexArray(0);

	//bind with vao
	glBindVertexArray(vao_green);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);

	//unbind vao
	glBindVertexArray(0);

	//bind with vao
	glBindVertexArray(vao_blue);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);
	glDrawArrays(GL_LINES, 6, 2);
	glDrawArrays(GL_LINES, 8, 2);
	glDrawArrays(GL_LINES, 10, 2);
	glDrawArrays(GL_LINES, 12, 2);
	glDrawArrays(GL_LINES, 14, 2);
	glDrawArrays(GL_LINES, 16, 2);
	glDrawArrays(GL_LINES, 18, 2);
	glDrawArrays(GL_LINES, 20, 2);

	glDrawArrays(GL_LINES, 22, 2);
	glDrawArrays(GL_LINES, 24, 2);
	glDrawArrays(GL_LINES, 26, 2);
	glDrawArrays(GL_LINES, 28, 2);
	glDrawArrays(GL_LINES, 30, 2);
	glDrawArrays(GL_LINES, 32, 2);
	glDrawArrays(GL_LINES, 34, 2);
	glDrawArrays(GL_LINES, 36, 2);
	glDrawArrays(GL_LINES, 38, 2);
	glDrawArrays(GL_LINES, 40, 2);
	glDrawArrays(GL_LINES, 42, 2);

	glDrawArrays(GL_LINES, 44, 2);
	glDrawArrays(GL_LINES, 46, 2);
	glDrawArrays(GL_LINES, 48, 2);
	glDrawArrays(GL_LINES, 50, 2);
	glDrawArrays(GL_LINES, 52, 2);
	glDrawArrays(GL_LINES, 54, 2);
	glDrawArrays(GL_LINES, 56, 2);
	glDrawArrays(GL_LINES, 58, 2);
	glDrawArrays(GL_LINES, 60, 2);
	glDrawArrays(GL_LINES, 62, 2);
	glDrawArrays(GL_LINES, 64, 2);

	glDrawArrays(GL_LINES, 66, 2);
	glDrawArrays(GL_LINES, 68, 2);
	glDrawArrays(GL_LINES, 70, 2);
	glDrawArrays(GL_LINES, 72, 2);
	glDrawArrays(GL_LINES, 74, 2);
	glDrawArrays(GL_LINES, 76, 2);
	glDrawArrays(GL_LINES, 78, 2);
	glDrawArrays(GL_LINES, 80, 2);
	glDrawArrays(GL_LINES, 82, 2);
	glDrawArrays(GL_LINES, 84, 2);
	glDrawArrays(GL_LINES, 86, 2);

	glDrawArrays(GL_LINES, 88, 2);
	glDrawArrays(GL_LINES, 90, 2);
	glDrawArrays(GL_LINES, 92, 2);
	glDrawArrays(GL_LINES, 94, 2);
	glDrawArrays(GL_LINES, 96, 2);
	glDrawArrays(GL_LINES, 98, 2);
	glDrawArrays(GL_LINES, 100, 2);
	glDrawArrays(GL_LINES, 102, 2);
	glDrawArrays(GL_LINES, 104, 2);
	glDrawArrays(GL_LINES, 106, 2);
	glDrawArrays(GL_LINES, 108, 2);

	glDrawArrays(GL_LINES, 110, 2);
	glDrawArrays(GL_LINES, 112, 2);
	glDrawArrays(GL_LINES, 114, 2);
	glDrawArrays(GL_LINES, 116, 2);
	glDrawArrays(GL_LINES, 118, 2);
	glDrawArrays(GL_LINES, 120, 2);
	glDrawArrays(GL_LINES, 122, 2);
	glDrawArrays(GL_LINES, 124, 2);
	glDrawArrays(GL_LINES, 126, 2);
	glDrawArrays(GL_LINES, 128, 2);
	glDrawArrays(GL_LINES, 130, 2);

	glDrawArrays(GL_LINES, 132, 2);
	glDrawArrays(GL_LINES, 134, 2);
	glDrawArrays(GL_LINES, 136, 2);
	glDrawArrays(GL_LINES, 138, 2);
	glDrawArrays(GL_LINES, 140, 2);
	glDrawArrays(GL_LINES, 142, 2);
	glDrawArrays(GL_LINES, 144, 2);
	glDrawArrays(GL_LINES, 146, 2);
	glDrawArrays(GL_LINES, 148, 2);
	glDrawArrays(GL_LINES, 150, 2);
	glDrawArrays(GL_LINES, 152, 2);

	glDrawArrays(GL_LINES, 154, 2);
	glDrawArrays(GL_LINES, 156, 2);
	glDrawArrays(GL_LINES, 158, 2);
	glDrawArrays(GL_LINES, 160, 2);
	glDrawArrays(GL_LINES, 162, 2);
	glDrawArrays(GL_LINES, 164, 2);

	//unbind vao
	glBindVertexArray(0);
	/*=============================================================================== */
		//-------------------------------------------------------------------------------------------------------------------
	
	//for triangle
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();

	//Do necessary transformation
	modelViewMatrix = translate(0.0f, 0.001f, -2.0f);

	//Do necessary matrix multiplication
	//modelViewMatrix = translationMatrix * rotationMatrix;
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform,
					   1,
					   GL_FALSE,
					   modelViewProjectionMatrix);

	//bind with vao
	glBindVertexArray(vao_triangle);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//------------------------------------------------------------------------------------------------------------------

	//for rectangle

	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();

	//Do necessary transformation
	modelViewMatrix = translate(0.003f, 0.003f, -2.0f);

	//Do necessary matrix multiplication
	//modelViewMatrix = translationMatrix * rotationMatrix;
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform,
					   1,
					   GL_FALSE,
					   modelViewProjectionMatrix);

	//bind with vao
	glBindVertexArray(vao_rectangle);

	//draw scene
	//glLineWidth(2.0f);

	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);
	glDrawArrays(GL_LINES, 6, 2);

	//unbind vao
	glBindVertexArray(0);
	//------------------------------------------------------------------------------------------------------------------

	//for outer circle

	GLfloat outerCircleVertex[10];
	GLfloat radius_outer = 0.71;

	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();

	//Do necessary transformation
	modelViewMatrix = translate(0.0f, 0.0f, -2.0f);

	//Do necessary matrix multiplication
	//modelViewMatrix = translationMatrix * rotationMatrix;
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform,
					   1,
					   GL_FALSE,
					   modelViewProjectionMatrix);
	
	glBindVertexArray(vao_outer_circle);

	for (GLfloat angle = 0.0f; angle < (2.0f * PI); angle = angle + 0.01f)
	{
		outerCircleVertex[0] = ((cos(angle) * radius_outer));
		outerCircleVertex[1] = ((sin(angle) * radius_outer));
		outerCircleVertex[2] = 0.0f;

		glBindBuffer(GL_ARRAY_BUFFER, vbo_outer_circle_pos);
		glBufferData(GL_ARRAY_BUFFER, sizeof(outerCircleVertex), outerCircleVertex, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//draw scene
		glPointSize(2.0f);
		glDrawArrays(GL_POINTS, 0, 1);
	}

	//unbind vao
	glBindVertexArray(0);
	//------------------------------------------------------------------------------------------------------------------
	//for inner circle

	GLfloat innerCircleVertex[10];
	GLfloat radius_inner = 0.308;

	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();

	//Do necessary transformation
	modelViewMatrix = translate(0.0f, -0.19f, -2.0f);

	//Do necessary matrix multiplication
	//modelViewMatrix = translationMatrix * rotationMatrix;
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform,
					   1,
					   GL_FALSE,
					   modelViewProjectionMatrix);

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


	// stop using OpenGL program object
	glUseProgram(0);

	//to process buffered OpenGL Routines
	glXSwapBuffers(gpDisplay, gWindow);
}


void uninitialize(void)
{
		if (vbo_red_line_position)
	{
		glDeleteBuffers(1, &vbo_red_line_position);
		vbo_red_line_position = 0;
	}
	if (vbo_red_line_color)
	{
		glDeleteBuffers(1, &vbo_red_line_color);
		vbo_red_line_color = 0;
	}

	if (vbo_green_line_position)
	{
		glDeleteBuffers(1, &vbo_green_line_position);
		vbo_green_line_position = 0;
	}
	if (vbo_green_line_color)
	{
		glDeleteBuffers(1, &vbo_green_line_color);
		vbo_green_line_color = 0;
	}

	if (vbo_blue_line_position)
	{
		glDeleteBuffers(1, &vbo_blue_line_position);
		vbo_blue_line_position = 0;
	}
	if (vbo_blue_line_color)
	{
		glDeleteBuffers(1, &vbo_blue_line_color);
		vbo_blue_line_color = 0;
	}

	if (vao_red)
	{
		glDeleteVertexArrays(1, &vao_red);
		vao_red = 0;
	}
	if (vao_green)
	{
		glDeleteVertexArrays(1, &vao_green);
		vao_green = 0;
	}
	if (vao_blue)
	{
		glDeleteVertexArrays(1, &vao_blue);
		vao_blue = 0;
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
