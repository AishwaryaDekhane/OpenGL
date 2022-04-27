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
	void resize(int,int,int,int);
	void uninitialize(void);
	
	int winWidth=giWindowWidth;
	int winHeight=giWindowHeight;
	
	 int width;
	 int height;
	
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
						
						
						        case '0':
						                //full screen
						                
						                glViewport(0,0,(GLsizei)(winWidth),(GLsizei)(winHeight));
			                                      
			                                      //resize(0, 0, rc.right, rc.bottom);
			                                        break;

		                                        case '1':
			                                         //left bottom
			                                       
			                                       glViewport(0,0,(GLsizei)(winWidth/2),(GLsizei)(winHeight/2));
		                                                
		                                               // resize(0, 0, rc.right/2, rc.bottom/2);
			                                        break;

		                                        case '2':
		                                                //right bottom
			                                       
			                                       glViewport(winWidth/2, 0,(GLsizei)(winWidth/2),(GLsizei)(winHeight/2));
			                                        
			                                       
			                                        //resize(rc.right/2, 0, rc.right / 2, rc.bottom / 2);
			                                        
			                                        break;

		                                        case '3':
			                                        //right top
			                                       
			                                       glViewport(winWidth/2, winHeight/2,(GLsizei)(winWidth/2),(GLsizei)(winHeight/2));
			                                       
			                                       
			                                        //resize(rc.right / 2, rc.bottom/2, rc.right / 2, rc.bottom / 2);

			                                        break;

		                                        case '4':
			                                      //right bottom
			                                       
			                                       glViewport(0, winHeight/2, (GLsizei)(winWidth/2),(GLsizei)(winHeight/2));
			                                       
			                                        //resize(0, rc.bottom/2, rc.right / 2, rc.bottom / 2);

			                                        break;

		                                        case '5':
			                                       //left half
			                                       
			                                       glViewport(0, 0, (GLsizei)(winWidth/2),(GLsizei)(winHeight));
			                                       // resize(0, 0, rc.right / 2, rc.bottom);
			                                        
			                                        break;

		                                        case '6':
		                                                //right half
			                                        
			                                        glViewport(winWidth/2, 0, (GLsizei)(winWidth/2),(GLsizei)(winHeight));
			                                        //resize(rc.right / 2, 0, rc.right / 2, rc.bottom);

			                                        break;

		                                        case '7':
		                                                //upper half
		                                                
			                                        glViewport(0, winHeight/2, (GLsizei)(winWidth),(GLsizei)(winHeight/2));
			                                        //resize(0, rc.bottom/2, rc.right, rc.bottom / 2);

			                                        break;

		                                        case '8':
		                                                //lower half
			                                        
			                                        glViewport(0, 0, (GLsizei)(winWidth),(GLsizei)(winHeight/2));
			                                        //resize(0, 0, rc.right , rc.bottom / 2);

			                                        break;

		                                        case '9':
		                                                //at center
		                                                
		                                                glViewport(winWidth/4, winHeight/4, (GLsizei)(winWidth/2),(GLsizei)(winHeight/2));
			                                       
			                                        //resize(rc.right / 4, rc.bottom/4, rc.right / 2, rc.bottom / 2);

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
					resize(0,0,winWidth,winHeight);
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
	//void resize(int, int);
	void resize(int,int,int, int);
	
	gGLXContext=glXCreateContext(gpDisplay,gpXVisualInfo,NULL,GL_TRUE);
	
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	
	resize(0,0,giWindowWidth,giWindowHeight);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(0.0f, 1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(-1.0f, -1.0f);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2f(1.0f, -1.0f);
	glEnd();
	
	glXSwapBuffers(gpDisplay,gWindow);
}

void resize(int x, int y, int width, int height)
{
	glViewport(x, y, (GLsizei)width, (GLsizei)height);
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

