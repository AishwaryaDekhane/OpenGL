#include <iostream>
#include <stdio.h>
#include <stdlib.h> 
#include <memory.h> 

#include <X11/Xlib.h>
#include <X11/Xutil.h> 
#include <X11/XKBlib.h> 
#include <X11/keysym.h> 

//namespaces
using namespace std;

//global variable declarations
bool bFullscreen=false;
Display *gpDisplay=NULL;
XVisualInfo gpXVisualInfo;
Colormap gColormap;
Window gWindow;
int giWindowWidth=800;
int giWindowHeight=600;

//entry point fuction
int main(void)
{

        //fuction prototypes
	void CreateWindow(void);
	void ToggleFullscreen(void);
	void uninitialize();
	
	//variable declarations
	int winWidth=giWindowWidth;
	int winHeight=giWindowHeight;
	
	//code
	CreateWindow();

        //message loop
	XEvent event;
	KeySym keysym;
	
	while(1)        //WndProc
	{
		XNextEvent(gpDisplay,&event);           //GetMessage
		switch(event.type)
		{
			case MapNotify:                 //WM_CREATE
				break;
			case KeyPress:                  //keydown
				keysym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
				switch(keysym)
				{
					case XK_Escape:         //VK_ESCAPE
						uninitialize();
						exit(0);
					case XK_F:              //fullscreen
					case XK_f:
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
					default:
						break;
				}
				break;
			case ButtonPress:                       //mouse button
				switch(event.xbutton.button)
				{
					case 1:                 //left button
					    break;
					case 2:                 //middle button
					    break;
					case 3:                 //right button
					    break;
					default:
					    break;
				}
				break;
			case MotionNotify:                      //WM_MOUSEMOVE
				break;
			case ConfigureNotify:                   //WM_SIZE
				winWidth=event.xconfigure.width;
				winHeight=event.xconfigure.height;
				break;
			case Expose:                            //WM_PAINT
				break;
			case DestroyNotify:                     //WM_DESTROY
				break;
			case 33:                                //close button
				uninitialize();
				exit(0);
			default:
				break;
		}
	}
	uninitialize();
	return(0);
}

void CreateWindow(void)
{
        //fuction prototype
	void uninitialize(void);

        //variable declaraions
	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int defaultDepth;
	int styleMask;

	//code
	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL)
	{
		printf("ERROR : Unable To Open X Display.\nExitting Now...\n");
		uninitialize();
		exit(1);
	}
	
	defaultScreen=XDefaultScreen(gpDisplay);
	
	defaultDepth=DefaultDepth(gpDisplay,defaultScreen);
	
	Status status = XMatchVisualInfo(gpDisplay,defaultScreen,defaultDepth,TrueColor,&gpXVisualInfo);

	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(gpDisplay, 
					    RootWindow(gpDisplay, gpXVisualInfo.screen), 
					    gpXVisualInfo.visual,
 					    AllocNone);
	gColormap=winAttribs.colormap;

	winAttribs.background_pixel=BlackPixel(gpDisplay,defaultScreen);
	
	winAttribs.event_mask= ExposureMask | VisibilityChangeMask | ButtonPressMask | KeyPressMask | PointerMotionMask |
			       StructureNotifyMask;
        	
	styleMask=CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;
	
	gWindow=XCreateWindow(gpDisplay,
			      RootWindow(gpDisplay,gpXVisualInfo.screen),
			      0,
			      0,
			      giWindowWidth,
			      giWindowHeight,
			      0,
			      gpXVisualInfo.depth,
			      InputOutput,
			      gpXVisualInfo.visual,
			      styleMask,
			      &winAttribs);
	if(!gWindow)
	{
		printf("ERROR : Failed To Create Main Window.\nExitting Now...\n");
		uninitialize();
		exit(1);
	}
	
	XStoreName(gpDisplay,gWindow,"First XWindow");
	
	Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_DELETE_WINDOW",True);
	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);
	
	XMapWindow(gpDisplay,gWindow);
}

void ToggleFullscreen(void)
{
        //variable declarations
	Atom wm_state;
	Atom fullscreen;
	XEvent xev={0};
	
	//code
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
	           RootWindow(gpDisplay,gpXVisualInfo.screen),
	           False,
	           StructureNotifyMask,
	           &xev);
}

void uninitialize(void)
{
	if(gWindow)
	{
		XDestroyWindow(gpDisplay,gWindow);
	}
	
	if(gColormap)
	{
		XFreeColormap(gpDisplay,gColormap);
	}

	if(gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;
	}
}

