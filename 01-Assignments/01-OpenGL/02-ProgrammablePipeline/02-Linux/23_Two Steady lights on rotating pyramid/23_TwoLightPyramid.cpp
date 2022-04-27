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

GLuint vao_pyramid;
GLuint vbo_pyramid_position;
GLuint vbo_pyramid_normal;
GLuint vbo_pyramid_element;

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

bool gbAnimate;
bool gbLight;

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
		angle_pyramid = angle_pyramid + 0.05f;
		if (angle_pyramid >= 360.0f)
			angle_pyramid = angle_pyramid - 360.0f;
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

//-------------------------------------------------------------------------------------------------------------------
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
	/*=============================================================================== */

	// stop using OpenGL program object
	glUseProgram(0);

	//to process buffered OpenGL Routines
	glXSwapBuffers(gpDisplay, gWindow);
}


void uninitialize(void)
{
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
