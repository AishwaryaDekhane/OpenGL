//headers
#include <iostream>
#include <stdio.h> //for printf()
#include <stdlib.h> //for exit()
#include <memory.h> //for memset()

//headers for XServer
#include <X11/Xlib.h> //analogous to windows.h
#include <X11/Xutil.h> //for visuals
#include <X11/XKBlib.h> //XkbKeycodeToKeysym()
#include <X11/keysym.h> //for 'Keysym'

#include <GL/glew.h> 
// for GLSL extensions IMPORTANT : This Line Should Be Before #include<gl\gl.h>

#include <GL/gl.h>
#include <GL/glx.h> //for 'glx' functions

#include <SOIL/SOIL.h>

#include "vmath.h"

using namespace vmath;

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXCOORD0,
};

//global variable declarations
FILE *gpFile = NULL;

Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;
Colormap gColormap;
Window gWindow;
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB=NULL;
GLXFBConfig gGLXFBConfig;
GLXContext gGLXContext; //parallel to HGLRC

bool gbFullscreen = false;

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint vao_quad;

GLuint vbo_position_quad;

GLuint vbo_texture_quad;

GLuint mvpUniform;			//mvp = ModelViewProjection
mat4 perspectiveProjectionMatrix;	//mat4 is from vmath

GLuint Texture_Smiley;

GLuint samplerUniform;

int PressedDigit = 0;

//entry-point function
int main(int argc, char *argv[])
{
	//function prototype
	void CreateWindow(void);
	void ToggleFullscreen(void);
	void initialize(void);
	void resize(int,int);
	void display(void);
	void uninitialize(void);
	
	//code
	// create log file
	gpFile=fopen("Log.txt", "w");
	if (gpFile==NULL)
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
	bool bDone=false;
	
	while(bDone==false)
	{
		while(XPending(gpDisplay))
		{
			XNextEvent(gpDisplay,&event); //parallel to GetMessage()
			switch(event.type) //parallel to 'iMsg'
			{
				case MapNotify: //parallel to WM_CREATE
					break;
				case KeyPress: //parallel to WM_KEYDOWN
					keySym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
					switch(keySym)
					{
						case XK_Escape:
							bDone=true;
							break;
							
						case XK_F:
						case XK_f:
							if(gbFullscreen==false)
							{
								ToggleFullscreen();
								gbFullscreen=true;
							}
							else
							{
								ToggleFullscreen();
								gbFullscreen=false;
							}
							break;
							
						case 49:		                            
			                                PressedDigit = 1;
			                                break;

		                                case 50:		                              
			                                PressedDigit = 2;
			                                break;

		                                case 51:	                              
			                                PressedDigit = 3;
			                                break;

		                                case 52:
			                                PressedDigit = 4;
			                                break;
						   
							
						default:
							break;
					}
					break;
				case ButtonPress:
					switch(event.xbutton.button)
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
					winWidth=event.xconfigure.width;
					winHeight=event.xconfigure.height;
					resize(winWidth,winHeight);
					break;
				case Expose: //parallel to WM_PAINT
					break;
				case DestroyNotify:
					break;
				case 33: //close button, system menu -> close
					bDone=true;
					break;
				default:
					break;
			}
		}
		
		display();
		
	}
	
	uninitialize();
	return(0);
}

void CreateWindow(void)
{
	//function prototype
	void uninitialize(void);
	
	//variable declarations
	XSetWindowAttributes winAttribs;
	GLXFBConfig *pGLXFBConfigs=NULL;
	GLXFBConfig bestGLXFBConfig;
	XVisualInfo *pTempXVisualInfo=NULL;
	int iNumFBConfigs=0;
	int styleMask;
	int i;
	
	static int frameBufferAttributes[]={
		GLX_X_RENDERABLE,True,
		GLX_DRAWABLE_TYPE,GLX_WINDOW_BIT,
		GLX_RENDER_TYPE,GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE,GLX_TRUE_COLOR,
		GLX_RED_SIZE,8,
		GLX_GREEN_SIZE,8,
		GLX_BLUE_SIZE,8,
		GLX_ALPHA_SIZE,8,
		GLX_DEPTH_SIZE,24,
		GLX_STENCIL_SIZE,8,
		GLX_DOUBLEBUFFER,True,
		//GLX_SAMPLE_BUFFERS,1,
		//GLX_SAMPLES,4,
		None}; // array must be terminated by 0
	
	//code
	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL)
	{
		printf("ERROR : Unable To Obtain X Display.\n");
		uninitialize();
		exit(1);
	}
	
	// get a new framebuffer config that meets our attrib requirements
	pGLXFBConfigs=glXChooseFBConfig(gpDisplay,DefaultScreen(gpDisplay),frameBufferAttributes,&iNumFBConfigs);
	if(pGLXFBConfigs==NULL)
	{
		printf( "Failed To Get Valid Framebuffer Config. Exitting Now ...\n");
		uninitialize();
		exit(1);
	}
	printf("%d Matching FB Configs Found.\n",iNumFBConfigs);
	
	// pick that FB config/visual with the most samples per pixel
	int bestFramebufferconfig=-1,worstFramebufferConfig=-1,bestNumberOfSamples=-1,worstNumberOfSamples=999;
	for(i=0;i<iNumFBConfigs;i++)
	{
		pTempXVisualInfo=glXGetVisualFromFBConfig(gpDisplay,pGLXFBConfigs[i]);
		if(pTempXVisualInfo)
		{
			int sampleBuffer,samples;
			glXGetFBConfigAttrib(gpDisplay,pGLXFBConfigs[i],GLX_SAMPLE_BUFFERS,&sampleBuffer);
			glXGetFBConfigAttrib(gpDisplay,pGLXFBConfigs[i],GLX_SAMPLES,&samples);
			printf("Matching Framebuffer Config=%d : Visual ID=0x%lu : SAMPLE_BUFFERS=%d : SAMPLES=%d\n",i,pTempXVisualInfo->visualid,sampleBuffer,samples);
			if(bestFramebufferconfig < 0 || sampleBuffer && samples > bestNumberOfSamples)
			{
				bestFramebufferconfig=i;
				bestNumberOfSamples=samples;
			}
			if( worstFramebufferConfig < 0 || !sampleBuffer || samples < worstNumberOfSamples)
			{
				worstFramebufferConfig=i;
			    worstNumberOfSamples=samples;
			}
		}
		XFree(pTempXVisualInfo);
	}
	bestGLXFBConfig = pGLXFBConfigs[bestFramebufferconfig];
	// set global GLXFBConfig
	gGLXFBConfig=bestGLXFBConfig;
	
	// be sure to free FBConfig list allocated by glXChooseFBConfig()
	XFree(pGLXFBConfigs);
	
	gpXVisualInfo=glXGetVisualFromFBConfig(gpDisplay,bestGLXFBConfig);
	printf("Chosen Visual ID=0x%lu\n",gpXVisualInfo->visualid );
	
	//setting window's attributes
	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(gpDisplay,
										RootWindow(gpDisplay,gpXVisualInfo->screen), //you can give defaultScreen as well
										gpXVisualInfo->visual,
										AllocNone); //for 'movable' memory allocation
										
	winAttribs.event_mask=StructureNotifyMask | KeyPressMask | ButtonPressMask |
						  ExposureMask | VisibilityChangeMask | PointerMotionMask;
	
	styleMask=CWBorderPixel | CWEventMask | CWColormap;
	gColormap=winAttribs.colormap;										           
	
	gWindow=XCreateWindow(gpDisplay,
						  RootWindow(gpDisplay,gpXVisualInfo->screen),
						  0,
						  0,
						  WIN_WIDTH,
						  WIN_HEIGHT,
						  0, //border width
						  gpXVisualInfo->depth, //depth of visual (depth for Colormap)          
						  InputOutput, //class(type) of your window
						  gpXVisualInfo->visual,
						  styleMask,
						  &winAttribs);
	if(!gWindow)
	{
		printf("Failure In Window Creation.\n");
		uninitialize();
		exit(1);
	}
	
	XStoreName(gpDisplay,gWindow,"OpenGL Programmable Pipeline Window");
	
	Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_WINDOW_DELETE",True);
	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);
	
	XMapWindow(gpDisplay,gWindow);
}

void ToggleFullscreen(void)
{
	//code
	Atom wm_state=XInternAtom(gpDisplay,"_NET_WM_STATE",False); //normal window state
	
	XEvent event;
	memset(&event,0,sizeof(XEvent));
	
	event.type=ClientMessage;
	event.xclient.window=gWindow;
	event.xclient.message_type=wm_state;
	event.xclient.format=32; //32-bit
	event.xclient.data.l[0]=gbFullscreen ? 0 : 1;

	Atom fullscreen=XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN",False);	
	event.xclient.data.l[1]=fullscreen;
	
	//parallel to SendMessage()
	XSendEvent(gpDisplay,
			   RootWindow(gpDisplay,gpXVisualInfo->screen),
			   False, //do not send this message to Sibling windows
			   StructureNotifyMask, //resizing mask (event_mask)
			   &event);	
}

void initialize(void)
{
	// function declarations
	void uninitialize(void);
	void resize(int,int);
	bool LoadTexture(GLuint * texture, const char *path);
	
	//code
	// create a new GL context 4.5 for rendering
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((GLubyte *)"glXCreateContextAttribsARB");
	
	GLint attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB,4,
		GLX_CONTEXT_MINOR_VERSION_ARB,5,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		0 }; // array must be terminated by 0
		
	gGLXContext = glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0,True,attribs);

	if(!gGLXContext) // fallback to safe old style 2.x context
	{
		// When a context version below 3.0 is requested, implementations will return 
		// the newest context version compatible with OpenGL versions less than version 3.0.
		GLint attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB,1,
			GLX_CONTEXT_MINOR_VERSION_ARB,0,
			0 }; // array must be terminated by 0
		printf("Failed To Create GLX 4.5 context. Hence Using Old-Style GLX Context\n");
		gGLXContext = glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0,True,attribs);
	}
	else // successfully created 4.1 context
	{
		printf("OpenGL Context 4.5 Is Created.\n");
	}
	
	// verifying that context is a direct context
	if(!glXIsDirect(gpDisplay,gGLXContext))
	{
		printf("Indirect GLX Rendering Context Obtained\n");
	}
	else
	{
		printf("Direct GLX Rendering Context Obtained\n" );
	}
	
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	
	//code
	// GLEW Initialization Code For GLSL ( IMPORTANT : It Must Be Here. Means After Creating OpenGL Context But Before Using Any OpenGL Function )
	GLenum glew_error = glewInit();
	if (glew_error != GLEW_OK)
	{
		printf("Failure To Initialize GLEW. Exitting Now ...\n");
		uninitialize();
		exit(1);
	}

	// *** VERTEX SHADER ***
	// create shader
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	// provide source code to shader
	const GLchar *vertexShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec2 vTexture0_Coord;" \
		"out vec2 out_texture0_coord;" \
		"uniform mat4 u_mvp_matrix;" \
		"void main(void)" \
		"{" \
		"gl_Position = u_mvp_matrix * vPosition;" \
		"out_texture0_coord = vTexture0_Coord;" \
		"}";
		
	glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

	// compile shader
	glCompileShader(gVertexShaderObject);
	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	char *szInfoLog = NULL;
	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}

	// *** FRAGMENT SHADER ***
	// create shader
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	// provide source code to shader
	const GLchar *fragmentShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"in vec2 out_texture0_coord;" \
		"out vec4 FragColor;" \
		"uniform sampler2D u_texture0_sampler;" \
		"void main(void)" \
		"{" \
		"FragColor = texture(u_texture0_sampler, out_texture0_coord);" \
		"}";
		
	glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

	// compile shader
	glCompileShader(gFragmentShaderObject);
	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
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
	mvpUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");
	samplerUniform = glGetUniformLocation(gShaderProgramObject, "u_texture0_sampler");

	//declare vertex3f coordinates here

	const GLfloat quadVertices[] =
	{       1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f
	};



	//--------------------------------------------------------------------------------------------------------------

	//For rectangle

	//vao = VertexArrayObject
	glGenVertexArrays(1, &vao_quad);

	//bind array with vao
	glBindVertexArray(vao_quad);

	//For position

	//create vbo
	glGenBuffers(1, &vbo_position_quad);

	//target vbo <-> GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_quad);

	//fill the buffer data statically
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	//give strides to vPosition
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);  //GL_FALSE = we are not sending normalized co-or

	//enable array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp

	//unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	//For texture

	//create vbo
	glGenBuffers(1, &vbo_texture_quad);

	//target vbo <-> GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vbo_texture_quad);

	//fill the buffer data statically
	glBufferData(GL_ARRAY_BUFFER,
		4 * 2 * sizeof(GLfloat),		//quad- 4 coord, texccord - 2 coord, float
		NULL,							//no array
		GL_DYNAMIC_DRAW);				//draw dynamically
	
	//for tweaked smiley it is in display

	//give strides to vPosition
	glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD0, 2, GL_FLOAT, GL_FALSE, 0, NULL);  //GL_FALSE = we are not sending normalized co-or

																					 //enable array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD0);	//AMC = AstroMediComp

														//unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);



	//unbind array
	glBindVertexArray(0);

	//-----------------------------------------------------------------------------------------------------------------

	
	// set-up depth buffer
	glClearDepth(1.0f);
	
	// enable depth testing
	glEnable(GL_DEPTH_TEST);
	
	// depth test to do
	glDepthFunc(GL_LEQUAL);
	
	// set really nice percpective calculations 
	// glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	
	// We will always cull back faces for better performance
	glEnable(GL_CULL_FACE);

	// set background clearing color
	glClearColor(0.0f,0.0f,0.0f,0.0f); // black
	
	//identity() is in mat4
	perspectiveProjectionMatrix = mat4::identity();
	
	//for texture
	LoadTexture(&Texture_Smiley, "Smiley.bmp");
	
	glEnable(GL_TEXTURE_2D);
	
	// resize
	resize(WIN_WIDTH, WIN_HEIGHT);
}

void resize(int width,int height)
{
    //code
	if (height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	perspectiveProjectionMatrix = perspective(45.0f, ((GLfloat)width / (GLfloat)height), 0.1f, 100.0f);
}

bool LoadTexture(GLuint *texture, const char *path)
{
   bool bResult;
    int imageWidth, imageHeight;
    unsigned char *imageData = NULL;
    
    imageData = SOIL_load_image(path, &imageWidth, &imageHeight, 0, SOIL_LOAD_RGB);

    if (imageData == NULL)
    {
        printf("ImageData = NULL\n");

        bResult = false;
        return bResult;
    }
    else
    {
        bResult = true;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
   
  // instead of gluBuild2DMipmaps we use these two below fuctions
		glTexImage2D(GL_TEXTURE_2D,			//target
			0,								//mipmap level
			GL_RGB,							//opengl internal image format
			imageWidth,
			imageHeight,
			0,								//border width
			GL_RGB,							//external image data format/ pixel
			GL_UNSIGNED_BYTE,
			imageData);

		glGenerateMipmap(GL_TEXTURE_2D);

    SOIL_free_image_data(imageData);
    return bResult;
}

void display(void)
{
        //declaration
	GLfloat quadTexture[8];
	
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//binding
	glUseProgram(gShaderProgramObject);

	// OpenGL Drawing
	//declaration of matrices
	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;

        // OpenGL Drawing
	
	// set modelview matrix to identity matrix

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//abu
	//do necessary transformation if ant required
	modelViewMatrix = translate(0.0f, 0.0f, -3.0f);


	//do necessary matrix multiplication

	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture_Smiley);
	glUniform1i(samplerUniform, 0);

	//bind with vao(this will avoid many repetitive binding with vbo)
	glBindVertexArray(vao_quad);

	//similarly bind the textures if any

	//draw necessary scene

	if (PressedDigit == 1)
	{
		quadTexture[0] = 0.5f;
		quadTexture[1] = 0.5f;
		quadTexture[2] = 0.0f;
		quadTexture[3] = 0.5f;
		quadTexture[4] = 0.0f;
		quadTexture[5] = 0.0f;
		quadTexture[6] = 0.5f;
		quadTexture[7] = 0.0f;

	}
	else if (PressedDigit == 2)
	{
		quadTexture[0] = 1.0f;
		quadTexture[1] = 1.0f;
		quadTexture[2] = 0.0f;
		quadTexture[3] = 1.0f;
		quadTexture[4] = 0.0f;
		quadTexture[5] = 0.0f;
		quadTexture[6] = 1.0f;
		quadTexture[7] = 0.0f;

	}
	else if (PressedDigit == 3)
	{
		quadTexture[0] = 2.0f;
		quadTexture[1] = 2.0f;
		quadTexture[2] = 0.0f;
		quadTexture[3] = 2.0f;
		quadTexture[4] = 0.0f;
		quadTexture[5] = 0.0f;
		quadTexture[6] = 2.0f;
		quadTexture[7] = 0.0f;
	}
	else if (PressedDigit == 4)
	{
		quadTexture[0] = 0.5f;
		quadTexture[1] = 0.5f;
		quadTexture[2] = 0.5f;
		quadTexture[3] = 0.5f;
		quadTexture[4] = 0.5f;
		quadTexture[5] = 0.5f;
		quadTexture[6] = 0.5f;
		quadTexture[7] = 0.5f;
	}
	else
	{
		quadTexture[0] = 1.0f;
		quadTexture[1] = 1.0f;
		quadTexture[2] = 0.0f;
		quadTexture[3] = 1.0f;
		quadTexture[4] = 0.0f;
		quadTexture[5] = 0.0f;
		quadTexture[6] = 1.0f;
		quadTexture[7] = 0.0f;
		
	}
	


	//target vbo <-> GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vbo_texture_quad);

	//fill the buffer data statically
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadTexture), quadTexture, GL_DYNAMIC_DRAW);

	//give strides to vPosition
	//glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD0, 2, GL_FLOAT, GL_FALSE, 0, NULL);  //GL_FALSE = we are not sending normalized co-or

	//enable array
	//glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD0);	//AMC = AstroMediComp

	
	//unbind buffer vbo
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//draw necessary scene
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	// glDrawArrays(GLenum, GLint , GLsizei)
	// (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

	//unbind vao
	glBindVertexArray(0);

	//unbinding - unused program
	glUseProgram(0);

	
	//to process buffered OpenGL Routines
	glXSwapBuffers(gpDisplay,gWindow);
}

void uninitialize(void)
{
	//code
	if (vao_quad)
	{
		glDeleteBuffers(1, &vao_quad);
		vao_quad = 0;
	}

	if (vbo_position_quad)
	{
		glDeleteBuffers(1, &vbo_position_quad);
		vbo_position_quad = 0;
	}

	if (vbo_texture_quad)
	{
		glDeleteBuffers(1, &vbo_texture_quad);
		vbo_texture_quad = 0;
	}

	if (Texture_Smiley)
	{
		glDeleteTextures(1, &Texture_Smiley);
		Texture_Smiley = 0;
	}
	
	// detach vertex shader from shader program object
	glDetachShader(gShaderProgramObject, gVertexShaderObject);
	// detach fragment  shader from shader program object
	glDetachShader(gShaderProgramObject, gFragmentShaderObject);

	// delete vertex shader object
	glDeleteShader(gVertexShaderObject);
	gVertexShaderObject = 0;
	// delete fragment shader object
	glDeleteShader(gFragmentShaderObject);
	gFragmentShaderObject = 0;

	// delete shader program object
	glDeleteProgram(gShaderProgramObject);
	gShaderProgramObject = 0;

	// unlink shader program
	glUseProgram(0);

	// Releasing OpenGL related and XWindow related objects 	
	GLXContext currentContext=glXGetCurrentContext();
	if(currentContext!=NULL && currentContext==gGLXContext)
	{
		glXMakeCurrent(gpDisplay,0,0);
	}
	
	if(gGLXContext)
	{
		glXDestroyContext(gpDisplay,gGLXContext);
	}
	
	if(gWindow)
	{
		XDestroyWindow(gpDisplay,gWindow);
	}
	
	if(gColormap)
	{
		XFreeColormap(gpDisplay,gColormap);
	}
	
	if(gpXVisualInfo)
	{
		free(gpXVisualInfo);
		gpXVisualInfo=NULL;
	}
	
	if(gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;
	}

	if (gpFile)
	{
		fprintf(gpFile, "Log File Is Successfully Closed.\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}
