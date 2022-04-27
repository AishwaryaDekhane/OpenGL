#include <iostream>
#include <stdio.h>
#include <stdlib.h> 
#include <memory.h> 

#include <X11/Xlib.h>
#include <X11/Xutil.h> 
#include <X11/XKBlib.h>
#include <X11/keysym.h> 

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

using namespace std;

bool bFullscreen=false;
Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;
Colormap gColormap;
Window gWindow;
int giWindowWidth=800;
int giWindowHeight=600;

GLfloat angle_pyramid = 0.0f;

static GLXContext gGLXContext;
char keys[26];

bool bLight = false;

//LIGHT0
GLfloat LightAmbientZero[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat LightDiffuseZero[] = { 1.0f,0.0f,0.0f,1.0f };
GLfloat LightSpecularZero[] = { 1.0f,0.0f,0.0f,1.0f };
GLfloat LightPositionZero[] = { 0.0f,0.0f,0.0f,1.0f };

//LIGHT1
GLfloat LightAmbientOne[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat LightDiffuseOne[] = { 0.0f,1.0f,0.0f,1.0f };
GLfloat LightSpecularOne[] = { 0.0f,1.0f,0.0f,1.0f };
GLfloat LightPositionOne[] = { 0.0f,0.0f,0.0f,1.0f };

//LIGHT2
GLfloat LightAmbientTwo[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat LightDiffuseTwo[] = { 0.0f,0.0f,1.0f,1.0f };
GLfloat LightSpecularTwo[] = { 0.0f,0.0f,1.0f,1.0f };
GLfloat LightPositionTwo[] = { 0.0f,0.0f,0.0f,1.0f };

GLfloat MaterialAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat MaterialDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat MaterialSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat MaterialShininess[] = { 50.0f };

GLfloat LightAngleZero = 0.0f;
GLfloat LightAngleOne = 0.0f;
GLfloat LightAngleTwo = 0.0f;

GLUquadric *quadric = NULL;

int main(void)
{

	void CreateWindow(void);
	void ToggleFullscreen(void);
	void initialize(void);
	void display(void);
	void resize(int,int);
	void uninitialize(void);
	void update(void);
	
	int winWidth=giWindowWidth;
	int winHeight=giWindowHeight;
	
	bool bDone=false;
	
	CreateWindow();

	initialize();
	
	XEvent event;
	KeySym keysym;
	
	while(bDone==false)
	{
		while(XPending(gpDisplay))
		{
			XNextEvent(gpDisplay,&event);
			switch(event.type)
			{
				case MapNotify:
					break;
				case KeyPress:
					keysym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
				switch(keysym)
					{
						case XK_Escape:
						        bDone = true;
							//uninitialize();
							//exit(0);
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
						
				case 'L':
				case 'l':
					if (bLight == false)
					{
						bLight = true;
						glEnable(GL_LIGHTING);
					}
					else
					{
					        bLight = false;
						glDisable(GL_LIGHTING);
					}
					break;
						
				default:
					break;
				}
				
				break;
					
					
				case ButtonPress: 
					switch(event.xbutton.button)
					{
						case 1: 
						    break;
						case 2: 
						    break;
						case 3: 
						    break;
						default:
						    break;
					}
					break;
					
				case MotionNotify: 
					break;
					
				case ConfigureNotify: 
					winWidth=event.xconfigure.width;
					winHeight=event.xconfigure.height;
					resize(winWidth,winHeight);
					break;
					
				case Expose: 
					break;
					
				case DestroyNotify:
					break;
					
				case 33:
					bDone=true;
					break;
					
				default:
					break;
			}
		
		}
                update();
		display();
	}
	return(0);
}

void CreateWindow(void)
{

	void uninitialize(void);

	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int defaultDepth;
	int styleMask;

	static int frameBufferAttributes[]=
	{
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_WINDOW_BIT,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_ALPHA_SIZE, 1,
		None 
	}; 
	
	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL)
	{
		printf("ERROR : Unable To Open X Display.\nExitting Now...\n");
		uninitialize();
		exit(1);
	}
	
	defaultScreen=XDefaultScreen(gpDisplay);
	
	gpXVisualInfo=glXChooseVisual(gpDisplay,defaultScreen,frameBufferAttributes);
	
	if(gpXVisualInfo==NULL)
	{
		printf("ERROR : Unable To Allocate Memory For Visual Info.\nExitting Now...\n");
		uninitialize();
		exit(1);	
	}
		
	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(gpDisplay, 
					    RootWindow(gpDisplay, gpXVisualInfo->screen), 
					    gpXVisualInfo->visual,
 					    AllocNone);
	gColormap=winAttribs.colormap;

	winAttribs.background_pixel=BlackPixel(gpDisplay,defaultScreen);

	winAttribs.event_mask= ExposureMask | VisibilityChangeMask | ButtonPressMask | KeyPressMask | PointerMotionMask |
			       StructureNotifyMask;
	
	styleMask=CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;
	
	gWindow=XCreateWindow(gpDisplay,
			      RootWindow(gpDisplay,gpXVisualInfo->screen),
			      0,
			      0,
			      giWindowWidth,
			      giWindowHeight,
			      0,
			      gpXVisualInfo->depth,
			      InputOutput,
			      gpXVisualInfo->visual,
			      styleMask,
			      &winAttribs);
	if(!gWindow)
	{
		printf("ERROR : Failed To Create Main Window.\nExitting Now...\n");
		uninitialize();
		exit(1);
	}
	
	XStoreName(gpDisplay,gWindow,"OpenGL Window");
	
	Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_DELETE_WINDOW",True);
	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);
	
	XMapWindow(gpDisplay,gWindow);
}

void ToggleFullscreen(void)
{
	Atom wm_state;
	Atom fullscreen;
	XEvent xev={0};
	

	wm_state=XInternAtom(gpDisplay,"_NET_WM_STATE",False);
	memset(&xev,0,sizeof(xev));
	
	xev.type=ClientMessage;
	xev.xclient.window=gWindow;
	xev.xclient.message_type=wm_state;
	xev.xclient.format=32;
	xev.xclient.data.l[0]=bFullscreen ? 0 : 1;
	
	fullscreen=XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN",False);
	xev.xclient.data.l[1]=fullscreen;
	
	XSendEvent(gpDisplay,
	           RootWindow(gpDisplay,gpXVisualInfo->screen),
	           False,
	           StructureNotifyMask,
	           &xev);
}

void initialize(void)
{
        //fuction prototype	
        void uninitialize(void);
	void resize(int, int);
	
	gGLXContext=glXCreateContext(gpDisplay,gpXVisualInfo,NULL,GL_TRUE);
	
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	
	resize(giWindowWidth,giWindowHeight);
	
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);

	glDepthFunc(GL_LEQUAL);

	//for lights

	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbientZero);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuseZero);
	glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecularZero);

	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbientOne);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuseOne);
	glLightfv(GL_LIGHT1, GL_SPECULAR, LightSpecularOne);

	glLightfv(GL_LIGHT2, GL_AMBIENT, LightAmbientTwo);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, LightDiffuseTwo);
	glLightfv(GL_LIGHT2, GL_SPECULAR, LightSpecularTwo);

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);


}


void update(void)
{
	LightAngleZero = LightAngleZero + 0.05f;
	if (LightAngleZero >= 360.0f)
		LightAngleZero = LightAngleZero - 360.0f;

	LightAngleOne = LightAngleOne + 0.05f;
	if (LightAngleOne >= 360.0f)
		LightAngleOne = LightAngleOne - 360.0f;

	LightAngleTwo = LightAngleTwo + 0.05f;
	if (LightAngleTwo >= 360.0f)
		LightAngleTwo = LightAngleTwo - 360.0f;
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	//glTranslatef(0.0f, 0.0f, -0.75f);    //we can use translate also but here we are moving camera not model so using LookAt instead of translate

	glPushMatrix();

	gluLookAt(0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	
	//for LIGHT0

	glPushMatrix(); //nested push
	glRotatef(LightAngleZero, 1.0f, 0.0f, 0.0f);
	LightPositionZero[1] = LightAngleZero;
	glLightfv(GL_LIGHT0, GL_POSITION, LightPositionZero);
	glPopMatrix();

	//for LIGHT1

	glPushMatrix(); //nested push
	glRotatef(LightAngleOne, 0.0f, 1.0f, 0.0f);
	LightPositionOne[0] = LightAngleOne;
	glLightfv(GL_LIGHT1, GL_POSITION, LightPositionOne);
	glPopMatrix();

	//for LIGHT2

	glPushMatrix(); //nested push
	glRotatef(LightAngleTwo, 0.0f, 0.0f, 1.0f);
	LightPositionTwo[0] = LightAngleTwo;
	glLightfv(GL_LIGHT2, GL_POSITION, LightPositionTwo);
	glPopMatrix();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	//for sphere

	quadric = gluNewQuadric();

	gluSphere(quadric, 0.5f, 30, 30);

	glPopMatrix();
	
	glXSwapBuffers(gpDisplay,gWindow);
}

void resize(int width,int height)
{
	if (height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, ((GLfloat)width / (GLfloat)height), 0.1f, 100.0f);
}

void uninitialize(void)
{
	GLXContext currentGLXContext;
	currentGLXContext=glXGetCurrentContext();

	if(currentGLXContext!=NULL && currentGLXContext==gGLXContext)
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
}

