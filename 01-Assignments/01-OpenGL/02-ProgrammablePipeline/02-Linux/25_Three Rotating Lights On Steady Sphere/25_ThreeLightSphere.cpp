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

GLuint ShaderProgramObject_pv;
GLuint ShaderProgramObject_pf;
GLuint gVertexShaderObject_pv;
GLuint gFragmentShaderObject_pv;
GLuint gVertexShaderObject_pf;
GLuint gFragmentShaderObject_pf;

GLuint vao_sphere;
GLuint vbo_sphere_position;
GLuint vbo_sphere_normal;
GLuint vbo_sphere_element;

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

GLfloat angle_sphere = 0.0f;

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
	static bool bIsFKeyPressed = false;
	static bool bIsVKeyPressed = false;

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
				case XK_Escape:						//full screen
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

				case XK_Q: //for 'q' or 'Q' Quit application
				case XK_q:
					bDone = true;
					break;

				case XK_F:
				case XK_f:
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
					
				case XK_V:
				case XK_v:
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

void initialize(void)
{
	// function declarations
	void uninitialize(void);
	void resize(int, int);

	//for shaders - variable declration
	GLint iShaderCompileStatus_pv = 0;
	GLint iInfoLogLength_pv = 0;
	GLchar *szInfoLog_pv = NULL;
	GLint iProgramLinkStatus_pv = 0;

	GLint iShaderCompileStatus_pf = 0;
	GLint iInfoLogLength_pf = 0;
	GLchar *szInfoLog_pf = NULL;
	GLint iProgramLinkStatus_pf = 0;

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
	//declare vertex3f coordinates here

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
				fprintf(gpFile, "Vertex Shader Error : \n %s\n", szInfoLog_pv);
				free(szInfoLog_pv);
				uninitialize();
				
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
				fprintf(gpFile, "Vertex Shader Error : \n %s\n", szInfoLog_pf);
				free(szInfoLog_pf);
				uninitialize();
				
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
				fprintf(gpFile, "Fragment Shader Errors : \n %s \n", szInfoLog_pv);
				free(szInfoLog_pv);
				uninitialize();
				
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
				fprintf(gpFile, "Fragment Shader Errors : \n %s \n", szInfoLog_pf);
				free(szInfoLog_pf);
				uninitialize();
				
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
				fprintf(gpFile, "Program Link Error : \n %s\n", szInfoLog_pv);
				free(szInfoLog_pv);
				uninitialize();
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
				fprintf(gpFile, "Program Link Error : \n %s\n", szInfoLog_pf);
				free(szInfoLog_pf);
			
				uninitialize();
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
//-------------------------------------------------------------------------------------------------------------------

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

	glUseProgram(ShaderProgramObject_pv);

	// detach vertex shader from shader program object
	glDetachShader(ShaderProgramObject_pv, gVertexShaderObject_pv);
	// detach fragment  shader from shader program object
	glDetachShader(ShaderProgramObject_pv, gFragmentShaderObject_pv);

	// delete vertex shader object
	glDeleteShader(gVertexShaderObject_pv);
	gVertexShaderObject_pv = 0;
	// delete fragment shader object
	glDeleteShader(gFragmentShaderObject_pv);
	gFragmentShaderObject_pv = 0;

	// delete shader program object
	glDeleteProgram(ShaderProgramObject_pv);
	ShaderProgramObject_pv = 0;

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
