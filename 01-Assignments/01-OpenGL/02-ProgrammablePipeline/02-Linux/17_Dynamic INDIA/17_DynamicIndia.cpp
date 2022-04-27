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


GLuint vao_first_I;
GLuint vbo_first_I_pos;
GLuint vbo_first_I_col;

GLuint vao_N;
GLuint vbo_N_pos;
GLuint vbo_N_col;

GLuint vao_D;
GLuint vbo_D_pos;
GLuint vbo_D_col;

GLuint vao_last_I;
GLuint vbo_last_I_pos;
GLuint vbo_last_I_col;

GLuint vao_A;
GLuint vbo_A_pos;
GLuint vbo_A_col;

//-----------------------------------------------------

GLuint vao_flag_A;
GLuint vbo_flag_A_pos;
GLuint vbo_flag_A_col;


//saffron color
static float As1 = -4.8f;			//for letter A
static float As2 = -4.8f;
static float As3 = -4.8f;

//white color
static float Aw1 = -4.8f;
static float Aw2 = -4.8f;
static float Aw3 = -4.8f;

//green color
static float Ag1 = -4.8f;
static float Ag2 = -4.8f;
static float Ag3 = -4.8f;

//----------------------------------------------------
//translation of plane
static float p1x = -3.5f;        
static float p2y = -3.5f;
static float p2x = -3.5f;
static float p3x = -3.5f;
static float p3y = -3.5f;

GLuint vao_top_plane;
GLuint vbo_top_plane_pos;
GLuint vbo_top_plane_col;

GLuint vao_bottom_plane;
GLuint vbo_bottom_plane_pos;
GLuint vbo_bottom_plane_col;

GLuint vao_middle_plane;
GLuint vbo_middle_plane_pos;
GLuint vbo_middle_plane_col;
//---------------------------------------------------------------
GLuint vao_top_flag_plane;
GLuint vbo_top_flag_plane_pos;
GLuint vbo_top_flag_plane_col;

GLuint vao_bottom_flag_plane;
GLuint vbo_bottom_flag_plane_pos;
GLuint vbo_bottom_flag_plane_col;

GLuint vao_middle_flag_plane;
GLuint vbo_middle_flag_plane_pos;
GLuint vbo_middle_flag_plane_col;


//-----------------------------------------------------------------
GLuint vao_topli_plane;
GLuint vbo_topli_plane_pos;
GLuint vbo_topli_plane_col;

GLuint vao_topla_plane;
GLuint vbo_topla_plane_pos;
GLuint vbo_topla_plane_col;

GLuint vao_toplf_plane;
GLuint vbo_toplf_plane_pos;
GLuint vbo_toplf_plane_col;

GLuint vao_middelli_plane;
GLuint vbo_middleli_plane_pos;
GLuint vbo_middleli_plane_col;

GLuint vao_middlela_plane;
GLuint vbo_middlela_plane_pos;
GLuint vbo_middlela_plane_col;

GLuint vao_middlelf_plane;
GLuint vbo_middlelf_plane_pos;
GLuint vbo_middlelf_plane_col;

GLuint vao_bottomli_plane;
GLuint vbo_bottomli_plane_pos;
GLuint vbo_bottomli_plane_col;

GLuint vao_bottomla_plane;
GLuint vbo_bottomla_plane_pos;
GLuint vbo_bottomla_plane_col;

GLuint vao_bottomlf_plane;
GLuint vbo_bottomlf_plane_pos;
GLuint vbo_bottomlf_plane_col;

//-----------------------------------------------------------------
static float i1x = -1.0f;
static float i1y = 0.0f;        //for letter I1

static float nx = -0.28f;
static float ny = 2.4f;         //for letter N

//saffron color     
static float s1 = -3.1f;			//for letter D
static float s2 = -3.1f;
static float s3 = -3.1f;

//green color
static float g1 = -3.1f;
static float g2 = -3.1f;
static float g3 = -3.1f;


static float i2x = 0.28f;		//for letter I2
static float i2y = -3.1f;

static float ax = 1.7f;
static float ay = 0.0f;         //for letter A



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
		//update();
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
	

}

void initialize(void)
{
	// function declarations
	void uninitialize(void);
	void resize(int, int);

	void initializeFirstI();
	void initializeN();
	void initializeD();
	void initializeSecondI();
	void initializeA();

	void FlagToA();

	void DrawTopPlane();
	void DrawBottomPlane();
	void DrawMiddelPlane();

	void DrawOnTopPlaneLetterI();
	void DrawOnTopPlaneLetterA();
	void DrawOnTopPlaneLetterF();

	void DrawOnMiddlePlaneLetterI();
	void DrawOnMiddlePlaneLetterA();
	void DrawOnMiddlePlaneLetterF();

	void DrawOnBottomPlaneLetterI();
	void DrawOnBottomPlaneLetterA();
	void DrawOnBottomPlaneLetterF();

	void TriFlagTopPlane();
	void TriFlagMiddlePlane();
	void TriFlagBottomPlane();


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

//--------------------------------------------------------------------------------------------------------------
	initializeD();
	initializeFirstI();
	initializeN();
	initializeSecondI();
	initializeA();

	FlagToA();

	DrawTopPlane();
	DrawBottomPlane();
	DrawMiddelPlane();

	DrawOnTopPlaneLetterI();
	DrawOnTopPlaneLetterA();
	DrawOnTopPlaneLetterF();

	DrawOnMiddlePlaneLetterI();
	DrawOnMiddlePlaneLetterA();
	DrawOnMiddlePlaneLetterF();

	DrawOnBottomPlaneLetterI();
	DrawOnBottomPlaneLetterA();
	DrawOnBottomPlaneLetterF();

	TriFlagTopPlane();
	TriFlagMiddlePlane();
	TriFlagBottomPlane();
	
	//-------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------

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
	//declarations
	void DrawAllLetter(void);
	void PositionLetter(void);
	void PositionFlagToA(void);
	void PositionPlane(void);
	void DrawAllPlanes(void);
	void DrawLettersOnPlane(void);
	void DrawFlagToA(void);
	void DrawAllTriFlag(void);

	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(ShaderProgramObject);

	// OpenGL Drawing
//------------------------------------------------------------------------------------------------------------
	DrawAllLetter();
	DrawAllPlanes();
	DrawLettersOnPlane();
	DrawFlagToA();
	DrawAllTriFlag();
//--------------------------------------------------------------------------------------------------------------

	/*=============================================================================== */

	// stop using OpenGL program object
	glUseProgram(0);

	//to process buffered OpenGL Routines
	glXSwapBuffers(gpDisplay, gWindow);

	//--------------------------------------------------------------------------------------------------------------
	//Move letters
	PositionLetter();

	//Fade In flag to A
	PositionFlagToA();

	//Move Planes
	PositionPlane();
}

void DrawLettersOnPlane(void)
{
	//declaration of matrices
	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;

	//for Top Plane

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(p2x, -p2y, -1.4f);
	//modelViewMatrix = translate(0.0f, 0.0f, -1.4f);
	
	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glLineWidth(3.0f);
	//bind with vao
	glBindVertexArray(vao_topli_plane);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);

	//bind with vao
	glBindVertexArray(vao_topla_plane);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//bind with vao
	glBindVertexArray(vao_toplf_plane);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//---------------------------------------------------------------------------------------------------------------

	//for Middle Plane

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(p1x, 0.0f, -1.4f);
	//modelViewMatrix = translate(0.0f, 0.0f, -1.4f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glLineWidth(3.0f);
	//bind with vao
	glBindVertexArray(vao_middelli_plane);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);

	//bind with vao
	glBindVertexArray(vao_middlela_plane);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//bind with vao
	glBindVertexArray(vao_middlelf_plane);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//---------------------------------------------------------------------------------------------------------------

	//for Bottom Plane

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	//modelViewMatrix = translate(0.0f, 0.0f, -1.4f);
	modelViewMatrix = translate(p3x, p3y, -1.4f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glLineWidth(3.0f);
	//bind with vao
	glBindVertexArray(vao_bottomli_plane);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);

	//bind with vao
	glBindVertexArray(vao_bottomla_plane);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//bind with vao
	glBindVertexArray(vao_bottomlf_plane);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//---------------------------------------------------------------------------------------------------------------

}

void PositionLetter(void)
{
	//for letter I (1st time - India)

	i1x = i1x + 0.001f;

	if (i1x >= 0.0f)
		i1x = 0.0f;


	i1y = i1y + 0.001f;

	if (i1y >= 0.0f)
		i1y = 0.0f;

	//for letter N

	nx = nx + 0.001f;

	if (nx >= 0.0f)
		nx = 0.0f;

	ny = ny - 0.001f;

	if (ny <= 0.0f)
		ny = 0.0f;


	//for letter D

	//glColor3f(1.0f, 0.6f, 0.2f);
	//glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	s1 = s1 + 0.001f;

	if (s1 >= 1.0f)
		s1 = 1.0f;


	s2 = s2 + 0.001f;

	if (s2 >= 0.6f)
		s2 = 0.6f;


	s3 = s3 + 0.001f;

	if (s3 >= 0.2f)
		s3 = 0.2f;


	g1 = g1 + 0.001f;

	if (g1 >= 0.0705882353f)
		g1 = 0.0705882353f;


	g2 = g2 + 0.001f;

	if (g2 >= 0.53333f)
		g2 = 0.53333f;


	g3 = g3 + 0.001f;

	if (g3 >= 0.0274509804f)
		g3 = 0.0274509804f;



	//for letter I (2nd time - indIa)

	i2x = i2x - 0.001f;

	if (i2x <= 0.0f)
		i2x = 0.0f;

	i2y = i2y + 0.001f;

	if (i2y >= 0.0f)
		i2y = 0.0f;


	//for letter A

	ax = ax - 0.001f;

	if (ax <= 0.0f)
		ax = 0.0f;

	ay = ay + 0.001f;

	if (ay >= 0.0f)
		ay = 0.0f;
}

void initializeFirstI()
{

	const GLfloat first_I_pos[] =
	{
		-0.56f, 0.3f, 0.0f,
		-0.56f, -0.3f, 0.0f,
		-0.65f, 0.3f, 0.0f,
		-0.47f, 0.3f, 0.0f,
		-0.65f, -0.3f, 0.0f,
		-0.47f, -0.3f, 0.0f,
	};

	const GLfloat first_I_col[] =
	{
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		0.0705882353f, 0.53333f, 0.0274509804f,

	};


	//create vao and vbo
	glGenVertexArrays(1, &vao_first_I);
	glBindVertexArray(vao_first_I);

	//yellow
	glGenBuffers(1, &vbo_first_I_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_first_I_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(first_I_pos), first_I_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_first_I_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_first_I_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(first_I_col), first_I_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void initializeN()
{
	GLfloat lineNpos[] = {
		-0.37f, 0.3f, 0.0f,
		-0.37f, -0.3f, 0.0f,
		-0.19f, 0.3f, 0.0f,
		-0.19f, -0.3f, 0.0f,
		-0.37f, 0.3f, 0.0f,
		-0.19f, -0.3f, 0.0f,
	};

	GLfloat lineNCol[] = {
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
	};

	glGenVertexArrays(1, &vao_N);
	glBindVertexArray(vao_N);

	glGenBuffers(1, &vbo_N_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_N_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineNpos), lineNpos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_N_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_N_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineNCol), lineNCol, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

}

void initializeD()
{
	GLfloat Dpos[] = {
		-0.09f, 0.3f, 0.0f,
		-0.09f, -0.3f, 0.0f,
		0.09f, 0.2f, 0.0f,
		0.09f, -0.2f, 0.0f,
		-0.09f, 0.3f, 0.0f,
		0.04f, 0.3f, 0.0f,
		-0.09f, -0.3f, 0.0f,
		0.04f, -0.3f, 0.0f,
		0.04f, 0.3f, 0.0f,
		0.09f, 0.2f, 0.0f,
		0.04f, -0.3f, 0.0f,
		0.09f, -0.2f, 0.0f,
	};

	GLfloat DCol[] = {
		s1, s2, s3,
		g1, g2, g3,
		s1, s2, s3,
		g1, g2, g3,
		s1, s2, s3,
		s1, s2, s3,
		g1, g2, g3,
		g1, g2, g3,
		s1, s2, s3,
		s1, s2, s3,
		g1, g2, g3,
		g1, g2, g3,
		/*1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		0.0705882353f, 0.53333f, 0.0274509804f,*/
	};

	glGenVertexArrays(1, &vao_D);
	glBindVertexArray(vao_D);

	glGenBuffers(1, &vbo_D_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_D_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Dpos), Dpos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_D_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_D_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(DCol), DCol, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void initializeSecondI()
{

	const GLfloat second_I_pos[] =
	{
		0.28f, 0.3f, 0.0f,
		0.28f, -0.3f, 0.0f,
		0.19f, 0.3f, 0.0f,
		0.37f, 0.3f, 0.0f,
		0.19f, -0.3f, 0.0f,
		0.37f, -0.3f, 0.0f,
	};

	const GLfloat second_I_col[] =
	{
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		0.0705882353f, 0.53333f, 0.0274509804f,

	};


	//create vao and vbo
	glGenVertexArrays(1, &vao_last_I);
	glBindVertexArray(vao_last_I);

	//yellow
	glGenBuffers(1, &vbo_last_I_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_last_I_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(second_I_pos), second_I_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_last_I_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_last_I_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(second_I_col), second_I_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void initializeA()
{

	const GLfloat A_pos[] =
	{
		0.56f, 0.3f, 0.0f,
		0.47f, -0.3f, 0.0f,
		0.56f, 0.3f, 0.0f,
		0.65f, -0.3f, 0.0f,
	};

	const GLfloat A_col[] =
	{
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
	};


	//create vao and vbo
	glGenVertexArrays(1, &vao_A);
	glBindVertexArray(vao_A);

	//yellow
	glGenBuffers(1, &vbo_A_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_A_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(A_pos), A_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_A_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_A_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(A_col), A_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------------------------------------
void PositionFlagToA(void)
{
	//for letter A

	//glColor3f(1.0f, 0.6f, 0.2f);
	//glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);


	//saffron color
	As1 = As1 + 0.001f;

	if (As1 >= 1.0f)
		As1 = 1.0f;


	As2 = As2 + 0.001f;

	if (As2 >= 0.6f)
		As2 = 0.6f;


	As3 = As3 + 0.001f;

	if (As3 >= 0.2f)
		As3 = 0.2f;

	//white color
	Aw1 = Aw1 + 0.001f;

	if (Aw1 >= 1.0f)
		Aw1 = 1.0f;


	Aw2 = Aw2 + 0.001f;

	if (Aw2 >= 1.0f)
		Aw2 = 1.0f;


	Aw3 = Aw3 + 0.001f;

	if (Aw3 >= 1.0f)
		Aw3 = 1.0f;


	//green color
	Ag1 = Ag1 + 0.001f;

	if (Ag1 >= 0.0705882353f)
		Ag1 = 0.0705882353f;


	Ag2 = Ag2 + 0.001f;

	if (Ag2 >= 0.53333f)
		Ag2 = 0.53333f;


	Ag3 = Ag3 + 0.001f;

	if (Ag3 >= 0.0274509804f)
		Ag3 = 0.0274509804f;

}



void FlagToA(void)
{
	const GLfloat A_flag_pos[] =
	{
		0.542f, 0.01f, 0.0f,
		0.578f, 0.01f, 0.0f,
		0.54f, 0.0f, 0.0f,
		0.58f, 0.0f, 0.0f,
		0.538f, -0.01f, 0.0f,
		0.581f, -0.01f, 0.0f,
	};

	const GLfloat A_flag_col[] =
	{
		As1, As2, As3,
		As1, As2, As3,
		Aw1, Aw2, Aw3,
		Aw1, Aw2, Aw3,
		Ag1, Ag2, Ag3,
		Ag1, Ag2, Ag3,

	};


	//create vao and vbo
	glGenVertexArrays(1, &vao_flag_A);
	glBindVertexArray(vao_flag_A);

	//yellow
	glGenBuffers(1, &vbo_flag_A_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_flag_A_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(A_flag_pos), A_flag_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_flag_A_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_flag_A_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(A_flag_col), A_flag_col, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

//------------------------------------------------------------------------------------------------------------------------
void PositionPlane(void)
{
	//for movement of middle plane
	p1x = p1x + 0.001;


	//for movement of top plane
	p2x = p2x + 0.001;

	p2y = p2y + 0.001;
	if (p2y >= 0.42)
		p2y = 0.42f;

	if (p2x >= 1.6f)
	{
		p2y = p2y - 0.01f;
		if (p2y <= 0.0f)
		{
			p2y = 0.0f;
		}

	}

	//for movement of bottom plane
	p3x = p3x + 0.001;

	p3y = p3y + 0.001;
	if (p3y >= 0.42)
		p3y = 0.42f;

	if (p3x >= 1.6f)
	{
		p3y = p3y - 0.01f;
		if (p3y <= 0.0f)
		{
			p3y = 0.0f;
		}

	}

}

void TriFlagTopPlane(void)
{
	const GLfloat Top_plane_flag_pos[] =
	{
		-0.96f, 0.41f, 0.0f,
		-1.2f, 0.41f, 0.0f,
		-0.96f, 0.42f, 0.0f,
		-1.2f, 0.42f, 0.0f,
		-0.96f, 0.43f, 0.0f,
		-1.2f, 0.43f, 0.0f,
	};

	const GLfloat Top_plane_flag_col[] =
	{
		//saffron color
		1.0f, 0.6f, 0.2f,
		1.0f, 0.6f, 0.2f,
		//white color 
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		//green color
		0.0705882353f, 0.53333f, 0.0274509804f,
		0.0705882353f, 0.53333f, 0.0274509804f,
	};
	//create vao and vbo
	glGenVertexArrays(1, &vao_top_flag_plane);
	glBindVertexArray(vao_top_flag_plane);

	glGenBuffers(1, &vbo_top_flag_plane_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_top_flag_plane_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Top_plane_flag_pos), Top_plane_flag_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_top_flag_plane_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_top_flag_plane_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Top_plane_flag_col), Top_plane_flag_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void TriFlagBottomPlane(void)
{
	const GLfloat Bottom_plane_flag_pos[] =
	{
		-0.96f, -0.43f, 0.0f,
		-1.2f, -0.43f, 0.0f,
		-0.96f, -0.42f, 0.0f,
		-1.2f, -0.42f, 0.0f,
		-0.96f, -0.41f, 0.0f,
		-1.2f, -0.41f, 0.0f,
	};

	const GLfloat Bottom_plane_flag_col[] =
	{
		//saffron color
		1.0f, 0.6f, 0.2f,
		1.0f, 0.6f, 0.2f,
		//white color 
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		//green color
		0.0705882353f, 0.53333f, 0.0274509804f,
		0.0705882353f, 0.53333f, 0.0274509804f,
	};
	//create vao and vbo
	glGenVertexArrays(1, &vao_bottom_flag_plane);
	glBindVertexArray(vao_bottom_flag_plane);

	glGenBuffers(1, &vbo_bottom_flag_plane_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_bottom_flag_plane_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Bottom_plane_flag_pos), Bottom_plane_flag_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_bottom_flag_plane_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_bottom_flag_plane_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Bottom_plane_flag_col), Bottom_plane_flag_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void TriFlagMiddlePlane(void)
{
	const GLfloat Middel_plane_flag_pos[] =
	{
		-0.96f, 0.01f, 0.0f,
		-1.2f, 0.01f, 0.0f,
		-0.96f, 0.0f, 0.0f,
		-1.2f, 0.0f, 0.0f,
		-0.96f, -0.01f, 0.0f,
		-1.2f, -0.01f, 0.0f,
	};

	const GLfloat Middel_plane_flag_col[] =
	{
		//saffron color
		1.0f, 0.6f, 0.2f,
		1.0f, 0.6f, 0.2f,
		//white color 
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		//green color
		0.0705882353f, 0.53333f, 0.0274509804f,
		0.0705882353f, 0.53333f, 0.0274509804f,
	};
	//create vao and vbo
	glGenVertexArrays(1, &vao_middle_flag_plane);
	glBindVertexArray(vao_middle_flag_plane);

	glGenBuffers(1, &vbo_middle_flag_plane_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_middle_flag_plane_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Middel_plane_flag_pos), Middel_plane_flag_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_middle_flag_plane_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_middle_flag_plane_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Middel_plane_flag_col), Middel_plane_flag_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}



void DrawTopPlane(void)
{
	const GLfloat Top_plane_pos[] =
	{
		-0.96f, 0.44f, 0.0f,
		-0.96f, 0.40f, 0.0f,
		-0.80f, 0.385f, 0.0f,
		-0.80f, 0.455f, 0.0f,

		-0.80f, 0.385f, 0.0f,
		-0.80f, 0.455f, 0.0f,
		-0.75f, 0.430f, 0.0f,
		-0.75f, 0.410f, 0.0f,

		//upper wing
		-0.92f, 0.44f, 0.0f,
		-0.92f, 0.50f, 0.0f,
		-0.89f, 0.50f, 0.0f,
		-0.84f, 0.44f, 0.0f,

		//lower wing
		-0.92f, 0.40f, 0.0f,
		-0.92f, 0.34f, 0.0f,
		-0.89f, 0.34f, 0.0f,
		-0.84f, 0.40f, 0.0f,

		//back tail up
		-0.95f, 0.42f, 0.0f,
		-0.97f, 0.42f, 0.0f,
		-0.99f, 0.46f, 0.0f,
		-0.97f, 0.46f, 0.0f,

		//back tail down
		-0.95f, 0.42f, 0.0f,
		-0.97f, 0.42f, 0.0f,
		-0.99f, 0.38f, 0.0f,
		-0.97f, 0.38f, 0.0f,
	};

	const GLfloat Top_plane_col[] =
	{
		0.7294117647f, 0.8862745098f, 0.9333333f,	//186,226,238 -powder blue
		0.7294117647f, 0.8862745098f, 0.9333333f,
		0.7294117647f, 0.8862745098f, 0.9333333f,
		0.7294117647f, 0.8862745098f, 0.9333333f,

		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,

		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,

		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,

		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,

		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
	};
	//create vao and vbo
	glGenVertexArrays(1, &vao_top_plane);
	glBindVertexArray(vao_top_plane);

	glGenBuffers(1, &vbo_top_plane_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_top_plane_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Top_plane_pos), Top_plane_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_top_plane_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_top_plane_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Top_plane_col), Top_plane_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void DrawBottomPlane(void)
{
	const GLfloat Bottom_plane_pos[] =
	{
		-0.96f, -0.44f, 0.0f,
		-0.96f, -0.40f, 0.0f,
		-0.80f, -0.385f, 0.0f,
		-0.80f, -0.455f, 0.0f,

		-0.80f, -0.385f, 0.0f,
		-0.80f, -0.455f, 0.0f,
		-0.75f, -0.430f, 0.0f,
		-0.75f, -0.410f, 0.0f,

		//upper wing
		-0.92f, -0.44f, 0.0f,
		-0.92f, -0.50f, 0.0f,
		-0.89f, -0.50f, 0.0f,
		-0.84f, -0.44f, 0.0f,

		//lower wing
		-0.92f, -0.40f, 0.0f,
		-0.92f, -0.34f, 0.0f,
		-0.89f, -0.34f, 0.0f,
		-0.84f, -0.40f, 0.0f,

		//back tail up
		-0.95f, -0.42f, 0.0f,
		-0.97f, -0.42f, 0.0f,
		-0.99f, -0.46f, 0.0f,
		-0.97f, -0.46f, 0.0f,

		//back tail down
		-0.95f, -0.42f, 0.0f,
		-0.97f, -0.42f, 0.0f,
		-0.99f, -0.38f, 0.0f,
		-0.97f, -0.38f, 0.0f,
	};

	const GLfloat Bottom_plane_col[] =
	{
		0.7294117647f, 0.8862745098f, 0.9333333f,	//186,226,238 -powder blue
		0.7294117647f, 0.8862745098f, 0.9333333f,
		0.7294117647f, 0.8862745098f, 0.9333333f,
		0.7294117647f, 0.8862745098f, 0.9333333f,

		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,

		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,

		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,

		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,

		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
	};
	//create vao and vbo
	glGenVertexArrays(1, &vao_bottom_plane);
	glBindVertexArray(vao_bottom_plane);

	glGenBuffers(1, &vbo_bottom_plane_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_bottom_plane_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Bottom_plane_pos), Bottom_plane_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_bottom_plane_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_bottom_plane_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Bottom_plane_col), Bottom_plane_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void DrawMiddelPlane(void)
{
	const GLfloat Middle_plane_pos[] =
	{
		-0.96f, 0.02f, 0.0f,
		-0.96f, -0.02f, 0.0f,
		-0.80f, -0.035f, 0.0f,
		-0.80f, 0.035f, 0.0f,

		-0.80f, -0.035f, 0.0f,
		-0.80f, 0.035f, 0.0f,
		-0.75f, 0.01f, 0.0f,
		-0.75f, -0.01f, 0.0f,

		//upper wing
		-0.92f, 0.02f, 0.0f,
		-0.92f, 0.09f, 0.0f,
		-0.89f, 0.09f, 0.0f,
		-0.84f, 0.02f, 0.0f,

		//lower wing
		-0.92f, -0.02f, 0.0f,
		-0.92f, -0.09f, 0.0f,
		-0.89f, -0.09f, 0.0f,
		-0.84f, -0.02f, 0.0f,

		//back tail up
		-0.95f, 0.00f, 0.0f,
		-0.97f, 0.00f, 0.0f,
		-0.99f, 0.04f, 0.0f,
		-0.97f, 0.04f, 0.0f,

		//back tail down
		-0.95f, 0.00f, 0.0f,
		-0.97f, 0.00f, 0.0f,
		-0.99f, -0.04f, 0.0f,
		-0.97f, -0.04f, 0.0f,
	};

	const GLfloat Middle_plane_col[] =
	{
		0.7294117647f, 0.8862745098f, 0.9333333f,	//186,226,238 -powder blue
		0.7294117647f, 0.8862745098f, 0.9333333f,
		0.7294117647f, 0.8862745098f, 0.9333333f,
		0.7294117647f, 0.8862745098f, 0.9333333f,

		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,

		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,

		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,

		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,

		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
		0.7294117647f, 0.8862745098, 0.9333333f,
	};
	//create vao and vbo
	glGenVertexArrays(1, &vao_middle_plane);
	glBindVertexArray(vao_middle_plane);

	glGenBuffers(1, &vbo_middle_plane_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_middle_plane_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Middle_plane_pos), Middle_plane_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_middle_plane_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_middle_plane_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Middle_plane_col), Middle_plane_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void DrawOnTopPlaneLetterI(void)
{
	const GLfloat topl_plane_pos[] =
	{
		-0.93f, 0.43f, 0.0f,
		-0.93f, 0.412f, 0.0f,
		-0.92f, 0.43f, 0.0f,
		-0.94f, 0.43f, 0.0f,
		-0.92f, 0.412f, 0.0f,
		-0.94f, 0.412f, 0.0f,
	};

	const GLfloat topl_plane_col[] =
	{
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};
	//create vao and vbo
	glGenVertexArrays(1, &vao_topli_plane);
	glBindVertexArray(vao_topli_plane);

	glGenBuffers(1, &vbo_topli_plane_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_topli_plane_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(topl_plane_pos), topl_plane_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_topli_plane_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_topli_plane_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(topl_plane_col), topl_plane_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void DrawOnTopPlaneLetterA(void)
{
	const GLfloat topl_plane_pos[] =
	{
		-0.90f, 0.43f, 0.0f,
		-0.91f, 0.412f, 0.0f,
		-0.90f, 0.43f, 0.0f,
		-0.89f, 0.412f, 0.0f,
		-0.908f, 0.42f, 0.0f,
		-0.895f, 0.42f, 0.0f,
	};

	const GLfloat topl_plane_col[] =
	{
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};
	//create vao and vbo
	glGenVertexArrays(1, &vao_topla_plane);
	glBindVertexArray(vao_topla_plane);

	glGenBuffers(1, &vbo_topla_plane_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_topla_plane_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(topl_plane_pos), topl_plane_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_topla_plane_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_topla_plane_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(topl_plane_col), topl_plane_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void DrawOnTopPlaneLetterF(void)
{
	const GLfloat topl_plane_pos[] =
	{
		-0.88f, 0.43f, 0.0f,
		-0.88f, 0.412f, 0.0f,
		-0.88f, 0.43f, 0.0f,
		-0.86f, 0.43f, 0.0f,
		-0.88f, 0.42f, 0.0f,
		-0.86f, 0.42f, 0.0f,
	};

	const GLfloat topl_plane_col[] =
	{
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};
	//create vao and vbo
	glGenVertexArrays(1, &vao_toplf_plane);
	glBindVertexArray(vao_toplf_plane);

	glGenBuffers(1, &vbo_toplf_plane_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_toplf_plane_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(topl_plane_pos), topl_plane_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_toplf_plane_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_toplf_plane_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(topl_plane_col), topl_plane_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void DrawOnMiddlePlaneLetterI()
{
	const GLfloat middleli_plane_pos[] =
	{
		-0.93f, 0.008f, 0.0f,
		-0.93f, -0.008f, 0.0f,
		-0.92f, 0.008f, 0.0f,
		-0.94f, 0.008f, 0.0f,
		-0.92f, -0.008f, 0.0f,
		-0.94f, -0.008f, 0.0f,
	};

	const GLfloat middleli_plane_col[] =
	{
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};
	
	//create vao and vbo
	glGenVertexArrays(1, &vao_middelli_plane);
	glBindVertexArray(vao_middelli_plane);

	glGenBuffers(1, &vbo_middleli_plane_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_middleli_plane_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(middleli_plane_pos), middleli_plane_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_middleli_plane_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_middleli_plane_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(middleli_plane_col), middleli_plane_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void DrawOnMiddlePlaneLetterA(void)
{
	const GLfloat middlel_plane_pos[] =
	{
		-0.90f, 0.0099f, 0.0f,
		-0.91f, -0.009f, 0.0f,
		-0.90f, 0.0099f, 0.0f,
		-0.89f, -0.009f, 0.0f,
		-0.908f, 0.001f, 0.0f,
		-0.895f, 0.001f, 0.0f,
	};

	const GLfloat middlel_plane_col[] =
	{
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};
	//create vao and vbo
	glGenVertexArrays(1, &vao_middlela_plane);
	glBindVertexArray(vao_middlela_plane);

	glGenBuffers(1, &vbo_middlela_plane_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_middlela_plane_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(middlel_plane_pos), middlel_plane_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_middlela_plane_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_middlela_plane_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(middlel_plane_col), middlel_plane_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void DrawOnMiddlePlaneLetterF(void)
{
	const GLfloat middlel_plane_pos[] =
	{
		-0.88f, 0.008f, 0.0f,
		-0.88f, -0.008f, 0.0f,
		-0.88f, 0.008f, 0.0f,
		-0.86f, 0.008f, 0.0f,
		-0.88f, -0.001f, 0.0f,
		-0.86f, -0.001f, 0.0f,
	};

	const GLfloat middlel_plane_col[] =
	{
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};
	//create vao and vbo
	glGenVertexArrays(1, &vao_middlelf_plane);
	glBindVertexArray(vao_middlelf_plane);

	glGenBuffers(1, &vbo_middlelf_plane_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_middlelf_plane_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(middlel_plane_pos), middlel_plane_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_middlelf_plane_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_middlelf_plane_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(middlel_plane_col), middlel_plane_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void DrawOnBottomPlaneLetterI(void)
{
	const GLfloat bottoml_plane_pos[] =
	{
		-0.93f, -0.43f, 0.0f,
		-0.93f, -0.412f, 0.0f,
		-0.92f, -0.43f, 0.0f,
		-0.94f, -0.43f, 0.0f,
		-0.92f, -0.412f, 0.0f,
		-0.94f, -0.412f, 0.0f,
	};

	const GLfloat bottoml_plane_col[] =
	{
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};
	//create vao and vbo
	glGenVertexArrays(1, &vao_bottomli_plane);
	glBindVertexArray(vao_bottomli_plane);

	glGenBuffers(1, &vbo_bottomli_plane_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_bottomli_plane_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bottoml_plane_pos), bottoml_plane_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_bottomli_plane_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_bottomli_plane_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bottoml_plane_col), bottoml_plane_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void DrawOnBottomPlaneLetterA(void)
{
	const GLfloat bottoml_plane_pos[] =
	{
		-0.90f, -0.412f, 0.0f,
		-0.91f, -0.43f, 0.0f,
		-0.90f, -0.412f, 0.0f,
		-0.89f, -0.43f, 0.0f,
		-0.908f, -0.42f, 0.0f,
		-0.895f, -0.42f, 0.0f,
	};

	const GLfloat bottoml_plane_col[] =
	{
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};
	//create vao and vbo
	glGenVertexArrays(1, &vao_bottomla_plane);
	glBindVertexArray(vao_bottomla_plane);

	glGenBuffers(1, &vbo_bottomla_plane_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_bottomla_plane_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bottoml_plane_pos), bottoml_plane_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_bottomla_plane_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_bottomla_plane_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bottoml_plane_col), bottoml_plane_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void DrawOnBottomPlaneLetterF(void)
{
	const GLfloat bottoml_plane_pos[] =
	{
		-0.88f, -0.43f, 0.0f,
		-0.88f, -0.412f, 0.0f,
		-0.88f, -0.412f, 0.0f,
		-0.86f, -0.412f, 0.0f,
		-0.88f, -0.42f, 0.0f,
		-0.86f, -0.42f, 0.0f,
	};

	const GLfloat bottoml_plane_col[] =
	{
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};
	//create vao and vbo
	glGenVertexArrays(1, &vao_bottomlf_plane);
	glBindVertexArray(vao_bottomlf_plane);

	glGenBuffers(1, &vbo_bottomlf_plane_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_bottomlf_plane_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bottoml_plane_pos), bottoml_plane_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_bottomlf_plane_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_bottomlf_plane_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bottoml_plane_col), bottoml_plane_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void DrawAllPlanes(void)
{
	//declaration of matrices
	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;

	//for Top Plane

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(p2x, -p2y, -1.4f);
	//modelViewMatrix = translate(0.0f, 0.0f, -1.4f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	//bind with vao
	glBindVertexArray(vao_top_plane);

	//draw scene
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

	//unbind vao
	glBindVertexArray(0);
	//---------------------------------------------------------------------------------------------------------------

	//for Bottom Plane

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	//modelViewMatrix = translate(0.0f, 0.0f, -1.4f);
	modelViewMatrix = translate(p3x, p3y, -1.4f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	//bind with vao
	glBindVertexArray(vao_bottom_plane);

	//draw scene
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

	//unbind vao
	glBindVertexArray(0);
	//--------------------------------------------------------------------------------------------------------------

	//for middle Plane

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	//modelViewMatrix = translate(0.0f, 0.0f, -1.4f);	
	modelViewMatrix = translate(p1x, 0.0f, -1.4f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	//bind with vao
	glBindVertexArray(vao_middle_plane);

	//draw scene
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

	//unbind vao
	glBindVertexArray(0);

}

void DrawAllTriFlag(void)
{
	//declaration of matrices
	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;

	//for Top Plane flag

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(p2x, -p2y, -1.4f);
	//modelViewMatrix = translate(0.0f, 0.0f, -1.4f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	//bind with vao
	glBindVertexArray(vao_top_flag_plane);

	glLineWidth(9.0f);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//---------------------------------------------------------------------------------------------------------------

	//for bottom plane flag

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(p3x, p3y, -1.4f);
	//modelViewMatrix = translate(0.0f, 0.0f, -1.4f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	//bind with vao
	glBindVertexArray(vao_bottom_flag_plane);

	glLineWidth(9.0f);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//---------------------------------------------------------------------------------------------------------------


	//for middel plane flag

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(p1x, 0.0f, -1.4f);
	//modelViewMatrix = translate(0.0f, 0.0f, -1.4f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	//bind with vao
	glBindVertexArray(vao_middle_flag_plane);

	glLineWidth(9.0f);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	
}


void DrawAllLetter(void)
{
	// OpenGL Drawing
	//declaration of matrices
	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;

	//-------------------------------------------------------------------------------------------------------------------

	//for first I

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(i1x, i1y, -1.5f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glLineWidth(60.0f);

	//bind with vao
	glBindVertexArray(vao_first_I);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//------------------------------------------------------------------------------------------------------------------

	//for line N

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(nx, ny, -1.5f);

	//do necessary matrix multiplication

	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glLineWidth(60.0f);

	//bind with vao
	glBindVertexArray(vao_N);

	//draw scene
	//glLineWidth(2.0f);

	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//------------------------------------------------------------------------------------------------------------------

	//for D letter

	GLfloat DCol[] = {
		s1, s2, s3,
		g1, g2, g3,
		s1, s2, s3,
		g1, g2, g3,
		s1, s2, s3,
		s1, s2, s3,
		g1, g2, g3,
		g1, g2, g3,
		s1, s2, s3,
		s1, s2, s3,
		g1, g2, g3,
		g1, g2, g3,
	};


	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(0.0f, 0.0f, -1.5f);

	//do necessary matrix multiplication

	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glBindVertexArray(vao_D);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_D_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(DCol), DCol, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glLineWidth(60.0f);

	//draw scene

	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);
	glDrawArrays(GL_LINES, 6, 2);
	glDrawArrays(GL_LINES, 8, 2);
	glDrawArrays(GL_LINES, 10, 2);

	//unbind vao
	glBindVertexArray(0);

	//--------------------------------------------------------------------------------------------------------------------


	//for last I

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(i2x, i2y, -1.5f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glLineWidth(60.0f);

	//bind with vao
	glBindVertexArray(vao_last_I);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//-------------------------------------------------------------------------------------------------------------------

	//for A

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(ax, ay, -1.5f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glLineWidth(60.0f);

	//bind with vao
	glBindVertexArray(vao_A);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);

	//unbind vao
	glBindVertexArray(0);
	//-------------------------------------------------------------------------------------------------------------------
}

void DrawFlagToA(void)
{
	// OpenGL Drawing
	//declaration of matrices
	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;

	//-------------------------------------------------------------------------------------------------------------------

	//for A flag

	const GLfloat A_flag_col[] =
	{
		As1, As2, As3,
		As1, As2, As3,
		Aw1, Aw2, Aw3,
		Aw1, Aw2, Aw3,
		Ag1, Ag2, Ag3,
		Ag1, Ag2, Ag3,
	};

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(ax, ay, -1.5f);
	//modelViewMatrix = translate(0.0f, 0.0f, -1.5f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	//bind with vao
	glBindVertexArray(vao_flag_A);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_flag_A_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(A_flag_col), A_flag_col, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glLineWidth(9.0f);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);

}

//-----------------------------------------------------------------------------------------------------------------------

void uninitialize(void)
{
	if (vao_first_I)
	{
		glDeleteVertexArrays(1, &vao_first_I);
		vao_first_I = 0;
	}

	if (vbo_first_I_pos)
	{
		glDeleteVertexArrays(1, &vbo_first_I_pos);
		vbo_first_I_pos = 0;
	}

	if (vbo_first_I_col)
	{
		glDeleteVertexArrays(1, &vbo_first_I_col);
		vbo_first_I_col = 0;
	}

	if (vao_D)
	{
		glDeleteVertexArrays(1, &vao_D);
		vao_D = 0;
	}

	if (vbo_D_pos)
	{
		glDeleteVertexArrays(1, &vbo_D_pos);
		vbo_D_pos = 0;
	}

	if (vbo_D_col)
	{
		glDeleteVertexArrays(1, &vbo_D_col);
		vbo_D_col = 0;
	}

	if (vao_N)
	{
		glDeleteVertexArrays(1, &vao_N);
		vao_N = 0;
	}

	if (vbo_N_pos)
	{
		glDeleteVertexArrays(1, &vbo_N_pos);
		vbo_N_pos = 0;
	}

	if (vbo_N_col)
	{
		glDeleteVertexArrays(1, &vbo_N_col);
		vbo_N_col = 0;
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
