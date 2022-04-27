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

#include <GL/glew.h> // for GLSL extensions IMPORTANT : This Line Should Be Before #include<gl\gl.h>

#include <GL/gl.h>
#include <GL/glx.h> //for 'glx' functions

#include "vmath.h"

using namespace vmath;

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXTCOORD0,
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

bool bFullscreen = false;

GLenum result;

GLuint VertexShaderObject;
GLuint FragmentShaderObject;
GLuint ShaderProgramObject;

GLuint vao_triangle;
GLuint vao_rectangle;

GLuint vbo_position_triangle;
GLuint vbo_position_rectangle;

GLuint vbo_color_triangle;
GLuint vbo_color_rectangle;

GLuint mvpUniform;					//mvp = ModelViewProjection
mat4 perspectiveProjectionMatrix;	//mat4 is from vmath

GLfloat angle_tri = 0.0f;
GLfloat angle_rec = 0.0f;

char keys[26];


//entry-point function
int main(int argc, char *argv[])
{
	//function prototype
	void CreateWindow(void);
	void ToggleFullscreen(void);
	void initialize(void);
	void resize(int,int);
	void display(void);
	void spin(void);
	void uninitialize(void);
	void update(void);
	
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
						
						default:
							break;
					}
					
					XLookupString(&event.xkey,keys,sizeof(keys),NULL,NULL);
				        switch(keys[0])
				        {
				        case 'F':
				        case 'f':
				                if(bFullscreen==false)
						        {
							        ToggleFullscreen();
							        bFullscreen=true;
						        }
						        else
						        {
							        ToggleFullscreen();
							        bFullscreen=false;
						        }
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
		update();
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

void update(void)
{
	angle_tri = angle_tri + 0.05f;
	if (angle_tri >= 360.0f)
		angle_tri = angle_tri - 360.0f;

	angle_rec = angle_rec + 0.05f;
	if (angle_rec >= 360.0f)
		angle_rec = angle_rec - 360.0f;

}

void ToggleFullscreen(void)
{
	Atom wm_state;
	Atom fullscreen;
	XEvent xev={0};
	
        //code
        
	wm_state=XInternAtom(gpDisplay,"_NET_WM_STATE",False);//normal window state
	memset(&xev,0,sizeof(xev));
	
	xev.type=ClientMessage;
	xev.xclient.window=gWindow;
	xev.xclient.message_type=wm_state;
	xev.xclient.format=32;//32-bit
	xev.xclient.data.l[0]=bFullscreen ? 0 : 1;
	
	fullscreen=XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN",False);
	xev.xclient.data.l[1]=fullscreen;
	
	//parallel to SendMessage()
	XSendEvent(gpDisplay,
	           RootWindow(gpDisplay,gpXVisualInfo->screen),
	           False,//do not send this message to Sibling windows
	           StructureNotifyMask, //resizing mask (event_mask)
	           &xev);
}

void initialize(void)
{
	// function declarations
	void uninitialize(void);
	void resize(int,int);
	
	//code
	// create a new GL context 4.5 for rendering
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((GLubyte *)"glXCreateContextAttribsARB");
	
	GLint attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB,4,
		GLX_CONTEXT_MINOR_VERSION_ARB,5,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		0 }; // array must be terminated by 0 or None
		
	gGLXContext = glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0,True,attribs);

	if(!gGLXContext) // fallback to safe old style 2.x context
	{
		// When a context version below 3.0 is requested, implementations will return 
		// the newest context version compatible with OpenGL versions less than version 3.0.
		GLint attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB,1,
			GLX_CONTEXT_MINOR_VERSION_ARB,0,
			0 }; // array must be terminated by 0 or None
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

	//VERTEX SHADER

	//define vertex shader object
	VertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	//write vertex shader code
	const GLchar *vertexShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec4 vColor;" \
		"out vec4 out_color;" \
		"uniform mat4 u_mvp_matrix;" \
		"void main(void)" \
		"{" \
		"gl_Position = u_mvp_matrix * vPosition;" \
		"out_color = vColor;" \
		"}";


	//specify above source code to vertex shader object
	glShaderSource(VertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

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
	glCompileShader(VertexShaderObject);

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
			szInfoLog = (GLchar*)malloc(iInfoLogLength);

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

	//define fragment shader object
	FragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//write fragment shader code
	const GLchar *fragmentShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"out vec4 FragColor;" \
		"in vec4 out_color;" \
		"void main(void)" \
		"{" \
		"FragColor = out_color;" \
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
			szInfoLog = (GLchar*)malloc(iInfoLogLength);

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

	//create obj
	ShaderProgramObject = glCreateProgram();

	//add vertex shader
	glAttachShader(ShaderProgramObject, VertexShaderObject);

	//add fragment shader
	glAttachShader(ShaderProgramObject, FragmentShaderObject);

	//Prelinking binding to vertex attributes
	glBindAttribLocation(ShaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");

	glLinkProgram(ShaderProgramObject);

	//Error checking for shader

	iInfoLogLength = 0;
	GLint iShaderProgramLinkStatus = 0;
	szInfoLog = NULL;

	//glGetProgramiv(ShaderProgramObject, GL_LINK_STATUS, &iProgramLinkStatus);
	glGetProgramiv(ShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);

	if (iShaderProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(ShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);

		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);

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

	//Postlinking retriving uniform locations
	mvpUniform = glGetUniformLocation(ShaderProgramObject, "u_mvp_matrix");

	//declare vertex3f coordinates here
	const GLfloat triangleVertices[] =
	{ 0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f
	};

	const GLfloat triangleColor[] =
	{ 1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};

	const GLfloat rectangleVertices[] =
	{ 1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f
	};

	const GLfloat rectangleColor[] =
	{ 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};


	//For triangle


	//vao = VertexArrayObject
	glGenVertexArrays(1, &vao_triangle);

	//bind array with vao
	glBindVertexArray(vao_triangle);


	//For position

	//create vbo
	glGenBuffers(1, &vbo_position_triangle);

	//target vbo <-> GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_triangle);

	//fill the buffer data statically
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

	//give strides to vPosition
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);  //GL_FALSE = we are not sending normalized co-or

																					//enable array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp

														//unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	//For Color

	//create vbo
	glGenBuffers(1, &vbo_color_triangle);

	//target vbo <-> GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color_triangle);

	//fill the buffer data statically
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColor), triangleColor, GL_STATIC_DRAW);

	//give strides to vPosition
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);  //GL_FALSE = we are not sending normalized co-or

																					//enable array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);	//AMC = AstroMediComp

														//unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);



	//unbind array
	glBindVertexArray(0);

	//--------------------------------------------------------------------------------------------------------------

	//For rectangle

	//vao = VertexArrayObject
	glGenVertexArrays(1, &vao_rectangle);

	//bind array with vao
	glBindVertexArray(vao_rectangle);

	//For position

	//create vbo
	glGenBuffers(1, &vbo_position_rectangle);

	//target vbo <-> GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_rectangle);

	//fill the buffer data statically
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), rectangleVertices, GL_STATIC_DRAW);

	//give strides to vPosition
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);  //GL_FALSE = we are not sending normalized co-or

																					//enable array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp

														//unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	//For color

	//create vbo
	glGenBuffers(1, &vbo_color_rectangle);

	//target vbo <-> GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color_rectangle);

	//fill the buffer data statically
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleColor), rectangleColor, GL_STATIC_DRAW);

	//give strides to vPosition
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);  //GL_FALSE = we are not sending normalized co-or

																					//enable array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);	//AMC = AstroMediComp

														//unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);



	//unbind array
	glBindVertexArray(0);
	
	//------------------------------------------------------------------------------------------------------------------
	
	//glShadeModel(GL_SMOOTH);
	
	glClearDepth(1.0f);	//give existance to depth buffer

	glEnable(GL_DEPTH_TEST);	//for 3D geometry enable depth test i.e z axis 

	glDepthFunc(GL_LEQUAL);		//LEQUAL = less than or equal to test compared to the max value which is set to 1.0f

								//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	
	// set background clearing color
	glClearColor(0.0f,0.0f,0.0f,0.0f); // blue
	
	//identity() is in mat4
	perspectiveProjectionMatrix = mat4::identity();
	
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

void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//binding
	glUseProgram(ShaderProgramObject);

	// OpenGL Drawing
	//declaration of matrices
	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;

	//for rotation
	mat4 ratationMatrix;
	mat4 translationMatrix;

	//For Triangle


	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	//do necessary transformation if ant required
	translationMatrix = translate(-1.5f, 0.0f, -6.0f);

	ratationMatrix = rotate(angle_tri, 0.0f, 1.0f, 0.0f);

	//do necessary matrix multiplication

	modelViewMatrix = translationMatrix * ratationMatrix;

	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	//bind with vao(this will avoid many repetitive binding with vbo)
	glBindVertexArray(vao_triangle);

	//similarly bind the textures if any

	//draw necessary scene
	glDrawArrays(GL_TRIANGLES, 0, 3);
	// glDrawArrays(GLenum, GLint , GLsizei)
	// (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

	//unbind va0
	glBindVertexArray(0);



	//For rectangle


	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	ratationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	//do necessary transformation if ant required
	translationMatrix = translate(1.5f, 0.0f, -6.0f);

	ratationMatrix = rotate(angle_rec, 0.0f, 1.0f, 0.0f);

	//do necessary matrix multiplication

	modelViewMatrix = translationMatrix * ratationMatrix;

	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	//bind with vao(this will avoid many repetitive binding with vbo)
	glBindVertexArray(vao_rectangle);

	//similarly bind the textures if any

	//draw necessary scene
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	// glDrawArrays(GLenum, GLint , GLsizei)
	// (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

	//unbind va0
	glBindVertexArray(0);



	//unbinding - unused program
	glUseProgram(0);

	
	//to process buffered OpenGL Routines
	glXSwapBuffers(gpDisplay,gWindow);
}


void uninitialize(void)
{
        //declare variables
	GLsizei shaderCount;
	GLsizei shaderNumber;
	
	//code
	if (vao_triangle)
	{
		glDeleteBuffers(1, &vao_triangle);
		vao_triangle = 0;
	}

	if (vao_rectangle)
	{
		glDeleteBuffers(1, &vao_rectangle);
		vao_rectangle = 0;
	}

	if (vbo_position_triangle)
	{
		glDeleteBuffers(1, &vbo_position_triangle);
		vbo_position_triangle = 0;
	}

	if (vbo_position_rectangle)
	{
		glDeleteBuffers(1, &vbo_position_rectangle);
		vbo_position_rectangle = 0;
	}
	
	if (vbo_color_triangle)
	{
		glDeleteBuffers(1, &vbo_color_triangle);
		vbo_color_triangle = 0;
	}

	if (vbo_color_rectangle)
	{
		glDeleteBuffers(1, &vbo_color_rectangle);
		vbo_color_rectangle = 0;
	}
	
	//safe release

	if (ShaderProgramObject)
	{
		glUseProgram(ShaderProgramObject);

		//ask program how many shaders are attached to you

		glGetProgramiv(ShaderProgramObject, GL_ATTACHED_SHADERS, &shaderCount);

		GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);

		if (pShaders)
		{
			glGetAttachedShaders(ShaderProgramObject, shaderCount, &shaderCount, pShaders);

			for (shaderNumber = 0;shaderNumber < shaderCount;shaderNumber++)
			{
				glDetachShader(ShaderProgramObject, pShaders[shaderNumber]);

				//delete detach shaders
				glDeleteShader(pShaders[shaderNumber]);

				pShaders[shaderNumber] = 0;
			}

			free(pShaders);
		}

		glDeleteProgram(ShaderProgramObject);
		ShaderProgramObject = 0;

		glUseProgram(0);
	}

	
	/*// detach vertex shader from shader program object
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
	glUseProgram(0);*/

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
