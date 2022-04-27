#include <iostream>
#include <stdio.h>
#include <stdlib.h> 
#include <memory.h> 
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h> 
#include <X11/XKBlib.h>
#include <X11/keysym.h> 

#include <GL/gl.h>
#include <GL/glx.h>
#include<GL/glu.h>

using namespace std;

bool bFullscreen=false;
Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;
Colormap gColormap;
Window gWindow;
int giWindowWidth=800;
int giWindowHeight=600;

static GLXContext gGLXContext;
char keys[26];

int main(void)
{

	void CreateWindow(void);
	void ToggleFullscreen(void);
	void initialize(void);
	void display(void);
	void resize(int,int);
	void uninitialize(void);
	
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
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	
        glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.0f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.56f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.56f, -0.5f, 0.0f);

	//I horizontal lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.65f, 0.5f, 0.0f);

	glVertex3f(-0.47f, 0.5f, 0.0f);

	//I horizontal lines
	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.65f, -0.5f, 0.0f);

	glVertex3f(-0.47f, -0.5f, 0.0f);

	glEnd();


//--------------------------------------------------------------------------------------------------------------
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.0f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//Letter N
	//N vertical lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.37f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.37f, -0.5f, 0.0f);



	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.19f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.19f, -0.5f, 0.0f);


	//N diagonal line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.37f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.19f, -0.5f, 0.0f);


	glEnd();

	//----------------------------------------------------------------------------------------------------------------

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.0f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//Letter D
	//D vertical lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.09f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.09f, -0.5f, 0.0f);


	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.09f, 0.4f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.09f, -0.4f, 0.0f);


	//D horizontal lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.09f, 0.5f, 0.0f);

	glVertex3f(0.04f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.09f, -0.5f, 0.0f);

	glVertex3f(0.04f, -0.5f, 0.0f);


	//D diagonal lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.04f, 0.5f, 0.0f);

	glVertex3f(0.09f, 0.4f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);
	
	glVertex3f(0.04f, -0.5f, 0.0f);

	glVertex3f(0.09f, -0.4f, 0.0f);


	glEnd();

	//------------------------------------------------------------------------------------------------------------------
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.0f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.28f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.28f, -0.5f, 0.0f);

	//I horizontal lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.19f, 0.5f, 0.0f);

	glVertex3f(0.37f, 0.5f, 0.0f);

	//I horizontal lines
	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.19f, -0.5f, 0.0f);

	glVertex3f(0.37f, -0.5f, 0.0f);

	glEnd();


	//--------------------------------------------------------------------------------------------------------------
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.0f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//Letter A
	//A diagonal line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.56f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.47f, -0.5f, 0.0f);


	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.56f, 0.5f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.65f, -0.5f, 0.0f);

	//A horizontal lines -white
	glColor3f(1.0f, 1.0f, 1.0f);

	glVertex3f(0.52f, 0.0f, 0.0f);

	glVertex3f(0.60f, 0.0f, 0.0f);

	//A horizontal lines -saffron
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.525f, 0.05f, 0.0f);

	glVertex3f(0.595f, 0.05f, 0.0f);

	//A horizontal lines -green
	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.51f, -0.05f, 0.0f);

	glVertex3f(0.61f, -0.05f, 0.0f);

	glEnd();


	//--------------------------------------------------------------------------------------------------------------

	
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
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
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

