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

#define PI 3.142f

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

	glTranslatef(0.0f, 0.0f, -2.5f);

	glLineWidth(3.0f);

	glBegin(GL_LINES);

	//horizontal lines

	glColor3f(1.0f, 0.0f, 0.0f);

	glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(-1.0f, 0.0f, 0.0f);

	//vertical lines

	glColor3f(0.0f, 1.0f, 0.0f);

	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, -1.0f, 0.0f);


	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.5f);

	glLineWidth(0.5f);

	glBegin(GL_LINES);

	//horizontal lines
	glColor3f(0.0f, 0.0f, 1.0f);


	glVertex3f(1.0f, 0.5f, 0.0f);
	glVertex3f(-1.0f, 0.5f, 0.0f);

	glVertex3f(1.0f, -0.5f, 0.0f);
	glVertex3f(-1.0f, -0.5f, 0.0f);


	glVertex3f(1.0f, 0.25f, 0.0f);
	glVertex3f(-1.0f, 0.25f, 0.0f);

	glVertex3f(1.0f, -0.25f, 0.0f);
	glVertex3f(-1.0f, -0.25f, 0.0f);


	glVertex3f(1.0f, 0.75f, 0.0f);
	glVertex3f(-1.0f, 0.75f, 0.0f);

	glVertex3f(1.0f, -0.75f, 0.0f);
	glVertex3f(-1.0f, -0.75f, 0.0f);


	glVertex3f(1.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 0.0f);

	glVertex3f(1.0f, -1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);


	glVertex3f(1.0f, 0.05f, 0.0f);
	glVertex3f(-1.0f, 0.05f, 0.0f);

	glVertex3f(1.0f, -0.05f, 0.0f);
	glVertex3f(-1.0f, -0.05f, 0.0f);


	glVertex3f(1.0f, 0.1f, 0.0f);
	glVertex3f(-1.0f, 0.1f, 0.0f);

	glVertex3f(1.0f, -0.1f, 0.0f);
	glVertex3f(-1.0f, -0.1f, 0.0f);


	glVertex3f(1.0f, 0.15f, 0.0f);
	glVertex3f(-1.0f, 0.15f, 0.0f);

	glVertex3f(1.0f, -0.15f, 0.0f);
	glVertex3f(-1.0f, -0.15f, 0.0f);


	glVertex3f(1.0f, 0.2f, 0.0f);
	glVertex3f(-1.0f, 0.2f, 0.0f);

	glVertex3f(1.0f, -0.2f, 0.0f);
	glVertex3f(-1.0f, -0.2f, 0.0f);


	glVertex3f(1.0f, 0.3f, 0.0f);
	glVertex3f(-1.0f, 0.3f, 0.0f);

	glVertex3f(1.0f, -0.3f, 0.0f);
	glVertex3f(-1.0f, -0.3f, 0.0f);


	glVertex3f(1.0f, 0.35f, 0.0f);
	glVertex3f(-1.0f, 0.35f, 0.0f);

	glVertex3f(1.0f, -0.35f, 0.0f);
	glVertex3f(-1.0f, -0.35f, 0.0f);


	glVertex3f(1.0f, 0.4f, 0.0f);
	glVertex3f(-1.0f, 0.4f, 0.0f);

	glVertex3f(1.0f, -0.4f, 0.0f);
	glVertex3f(-1.0f, -0.4f, 0.0f);

	glVertex3f(1.0f, 0.45f, 0.0f);
	glVertex3f(-1.0f, 0.45f, 0.0f);

	glVertex3f(1.0f, -0.45f, 0.0f);
	glVertex3f(-1.0f, -0.45f, 0.0f);


	glVertex3f(1.0f, 0.55f, 0.0f);
	glVertex3f(-1.0f, 0.55f, 0.0f);

	glVertex3f(1.0f, -0.55f, 0.0f);
	glVertex3f(-1.0f, -0.55f, 0.0f);


	glVertex3f(1.0f, 0.6f, 0.0f);
	glVertex3f(-1.0f, 0.6f, 0.0f);

	glVertex3f(1.0f, -0.6f, 0.0f);
	glVertex3f(-1.0f, -0.6f, 0.0f);


	glVertex3f(1.0f, 0.65f, 0.0f);
	glVertex3f(-1.0f, 0.65f, 0.0f);

	glVertex3f(1.0f, -0.65f, 0.0f);
	glVertex3f(-1.0f, -0.65f, 0.0f);


	glVertex3f(1.0f, 0.7f, 0.0f);
	glVertex3f(-1.0f, 0.7f, 0.0f);

	glVertex3f(1.0f, -0.7f, 0.0f);
	glVertex3f(-1.0f, -0.7f, 0.0f);


	glVertex3f(1.0f, 0.8f, 0.0f);
	glVertex3f(-1.0f, 0.8f, 0.0f);

	glVertex3f(1.0f, -0.8f, 0.0f);
	glVertex3f(-1.0f, -0.8f, 0.0f);


	glVertex3f(1.0f, 0.85f, 0.0f);
	glVertex3f(-1.0f, 0.85f, 0.0f);

	glVertex3f(1.0f, -0.85f, 0.0f);
	glVertex3f(-1.0f, -0.85f, 0.0f);


	glVertex3f(1.0f, 0.9f, 0.0f);
	glVertex3f(-1.0f, 0.9f, 0.0f);

	glVertex3f(1.0f, -0.9f, 0.0f);
	glVertex3f(-1.0f, -0.9f, 0.0f);


	glVertex3f(1.0f, 0.95f, 0.0f);
	glVertex3f(-1.0f, 0.95f, 0.0f);

	glVertex3f(1.0f, -0.95f, 0.0f);
	glVertex3f(-1.0f, -0.95f, 0.0f);



	
	//vertical lines

	glColor3f(0.0f, 0.0f, 1.0f);


	glVertex3f(0.5f, 1.0f, 0.0f);
	glVertex3f(0.5f, -1.0f, 0.0f);

	glVertex3f(-0.5f, 1.0f, 0.0f);
	glVertex3f(-0.5f, -1.0f, 0.0f);


	glVertex3f(0.25f, 1.0f, 0.0f);
	glVertex3f(0.25f, -1.0f, 0.0f);

	glVertex3f(-0.25f, 1.0f, 0.0f);
	glVertex3f(-0.25f, -1.0f, 0.0f);


	glVertex3f(0.75f, 1.0f, 0.0f);
	glVertex3f(0.75f, -1.0f, 0.0f);

	glVertex3f(-0.75f, 1.0f, 0.0f);
	glVertex3f(-0.75f, -1.0f, 0.0f);


	glVertex3f(1.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);

	glVertex3f(-1.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);


	glVertex3f(0.05f, 1.0f, 0.0f);
	glVertex3f(0.05f, -1.0f, 0.0f);

	glVertex3f(-0.05f, 1.0f, 0.0f);
	glVertex3f(-0.05f, -1.0f, 0.0f);


	glVertex3f(0.1f, 1.0f, 0.0f);
	glVertex3f(0.1f, -1.0f, 0.0f);

	glVertex3f(-0.1f, 1.0f, 0.0f);
	glVertex3f(-0.1f, -1.0f, 0.0f);


	glVertex3f(0.15f, 1.0f, 0.0f);
	glVertex3f(0.15f, -1.0f, 0.0f);

	glVertex3f(-0.15f, 1.0f, 0.0f);
	glVertex3f(-0.15f, -1.0f, 0.0f);


	glVertex3f(0.2f, 1.0f, 0.0f);
	glVertex3f(0.2f, -1.0f, 0.0f);

	glVertex3f(-0.2f, 1.0f, 0.0f);
	glVertex3f(-0.2f, -1.0f, 0.0f);


	glVertex3f(0.3f, 1.0f, 0.0f);
	glVertex3f(0.3f, -1.0f, 0.0f);

	glVertex3f(-0.3f, 1.0f, 0.0f);
	glVertex3f(-0.3f, -1.0f, 0.0f);


	glVertex3f(0.35f, 1.0f, 0.0f);
	glVertex3f(0.35f, -1.0f, 0.0f);

	glVertex3f(-0.35f, 1.0f, 0.0f);
	glVertex3f(-0.35f, -1.0f, 0.0f);


	glVertex3f(0.4f, 1.0f, 0.0f);
	glVertex3f(0.4f, -1.0f, 0.0f);

	glVertex3f(-0.4f, 1.0f, 0.0f);
	glVertex3f(-0.4f, -1.0f, 0.0f);

	glVertex3f(0.45f, 1.0f, 0.0f);
	glVertex3f(0.45f, -1.0f, 0.0f);

	glVertex3f(-0.45f, 1.0f, 0.0f);
	glVertex3f(-0.45f, -1.0f, 0.0f);


	glVertex3f(0.55f, 1.0f, 0.0f);
	glVertex3f(0.55f, -1.0f, 0.0f);

	glVertex3f(-0.55f, 1.0f, 0.0f);
	glVertex3f(-0.55f, -1.0f, 0.0f);


	glVertex3f(0.6f, 1.0f, 0.0f);
	glVertex3f(0.6f, -1.0f, 0.0f);

	glVertex3f(-0.6f, 1.0f, 0.0f);
	glVertex3f(-0.6f, -1.0f, 0.0f);


	glVertex3f(0.65f, 1.0f, 0.0f);
	glVertex3f(0.65f, -1.0f, 0.0f);

	glVertex3f(-0.65f, 1.0f, 0.0f);
	glVertex3f(-0.65f, -1.0f, 0.0f);


	glVertex3f(0.7f, 1.0f, 0.0f);
	glVertex3f(0.7f, -1.0f, 0.0f);

	glVertex3f(-0.7f, 1.0f, 0.0f);
	glVertex3f(-0.7f, -1.0f, 0.0f);


	glVertex3f(0.8f, 1.0f, 0.0f);
	glVertex3f(0.8f, -1.0f, 0.0f);

	glVertex3f(-0.8f, 1.0f, 0.0f);
	glVertex3f(-0.8f, -1.0f, 0.0f);


	glVertex3f(0.85f, 1.0f, 0.0f);
	glVertex3f(0.85f, -1.0f, 0.0f);

	glVertex3f(-0.85f, 1.0f, 0.0f);
	glVertex3f(-0.85f, -1.0f, 0.0f);


	glVertex3f(0.9f, 1.0f, 0.0f);
	glVertex3f(0.9f, -1.0f, 0.0f);

	glVertex3f(-0.9f, 1.0f, 0.0f);
	glVertex3f(-0.9f, -1.0f, 0.0f);


	glVertex3f(0.95f, 1.0f, 0.0f);
	glVertex3f(0.95f, -1.0f, 0.0f);

	glVertex3f(-0.95f, 1.0f, 0.0f);
	glVertex3f(-0.95f, -1.0f, 0.0f);



	glEnd();

	//for triangle
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.5f);

	glLineWidth(3.0f);

	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 0.0f);

	glVertex3f(0.5f, -0.5f, 0.0f);
	glVertex3f(0.0f, 0.5f, 0.0f);

	glVertex3f(0.0f, 0.5f, 0.0f);
	glVertex3f(-0.5f, -0.5f, 0.0f);

	glVertex3f(-0.5f, -0.5f, 0.0f);
	glVertex3f(0.5f, -0.5f, 0.0f);

	glEnd();

	//For incircle
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.5f);

	glLineWidth(3.0f);

	glBegin(GL_POINTS);

	glColor3f(1.0f, 1.0f, 0.0f); 

	for (GLfloat angle = 0.0f;angle < 2.0f*PI;angle = angle + 0.001f)
	{
		glVertex3f(0.308*cos(angle) + 0.0f, 0.308* sin(angle) + (-0.187f), 0.0f);
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
	}

	glEnd();


	//For vertical line
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.5f);

	glLineWidth(3.0f);

	glBegin(GL_LINES);

	glColor3f(1.0f, 1.0f, 0.0f); 

	glVertex3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, -0.5f, 0.0f);

	glEnd();


	//For rectangle
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.5f);

	glLineWidth(3.0f);

	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 0.0f);

	glVertex3f(-0.5f, -0.5f, 0.0f);
	glVertex3f(0.5f, -0.5f, 0.0f);

	glVertex3f(-0.5f, -0.5f, 0.0f);
	glVertex3f(-0.5f, 0.5f, 0.0f);

	glVertex3f(-0.5f, 0.5f, 0.0f);
	glVertex3f(0.5f, 0.5f, 0.0f);

	glVertex3f(0.5f, 0.5f, 0.0f);
	glVertex3f(0.5f, -0.5f, 0.0f);

	glEnd();


	//for outer circle
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -2.5f);

	glLineWidth(3.0f);

	glBegin(GL_POINTS);
	glColor3f(1.0f, 1.0f, 0.0f);

	for (GLfloat angle = 0.0f;angle < 2.0f*PI;angle = angle + 0.001f) //here radius=1
	{
		glVertex3f(cos(angle)*0.71, sin(angle)*0.71, 0.0f);
	}

	glEnd();
	
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

