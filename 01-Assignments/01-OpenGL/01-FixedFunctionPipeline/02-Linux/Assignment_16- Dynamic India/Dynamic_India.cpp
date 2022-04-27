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

GLfloat angle = 0.0f;

static GLXContext gGLXContext;
char keys[26];

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

static float p1x = -3.5f;       //translation of plane 
static float p2y = -3.5f;
static float p2x = -3.5f;
static float p3x = -3.5f;
static float p3y = -3.5f;

//saffron color     
static float mfs1 = 23.0f;			//for mid plane flag saffron 
static float mfs2 = 23.0f;
static float mfs3 = 23.0f;

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

//#################################################################################################################

void LetterOnMiddlePlaneI(void)
{
	glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.93f, 0.008f, 0.0f);

	glVertex3f(-0.93f, -0.008f, 0.0f);

	//I horizontal lines

	glVertex3f(-0.92f, 0.008f, 0.0f);

	glVertex3f(-0.94f, 0.008f, 0.0f);

	//I horizontal lines
	glVertex3f(-0.92f, -0.008f, 0.0f);

	glVertex3f(-0.94f, -0.008f, 0.0f);

	glEnd();
}

void LetterOnMiddlePlaneA(void)
{
	glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter A
	//A diagonal line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.90f, 0.0099f, 0.0f);

	glVertex3f(-0.91f, -0.009f, 0.0f);

	glVertex3f(-0.90f, 0.0099f, 0.0f);

	glVertex3f(-0.89f, -0.009f, 0.0f);

	//A horizontal line
	glVertex3f(-0.908f, 0.001f, 0.0f);

	glVertex3f(-0.895f, 0.001f, 0.0f);


	glEnd();

}

void LetterOnMiddlePlaneF(void)
{
	glLineWidth(3.0f);

	glBegin(GL_LINES);
	//Letter F
	//F vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.88f, 0.008f, 0.0f);

	glVertex3f(-0.88f, -0.008f, 0.0f);

	//horizontal lines
	glVertex3f(-0.88f, 0.008f, 0.0f);

	glVertex3f(-0.86f, 0.008f, 0.0f);

	glVertex3f(-0.88f, -0.001f, 0.0f);

	glVertex3f(-0.86f, -0.001f, 0.0f);

	glEnd();
}


//##################################################################################################################

void LetterOnTopPlaneI(void)
{
	/*glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.93f, 0.499f, 0.0f);

	glVertex3f(-0.93f, 0.481f, 0.0f);

	//I horizontal lines

	glVertex3f(-0.92f, 0.499f, 0.0f);

	glVertex3f(-0.94f, 0.499f, 0.0f);

	//I horizontal lines
	glVertex3f(-0.92f, 0.481f, 0.0f);

	glVertex3f(-0.94f, 0.481f, 0.0f);

	glEnd();*/

	glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.93f, 0.43f, 0.0f);

	glVertex3f(-0.93f, 0.412f, 0.0f);

	//I horizontal lines

	glVertex3f(-0.92f, 0.43f, 0.0f);

	glVertex3f(-0.94f, 0.43f, 0.0f);

	//I horizontal lines
	glVertex3f(-0.92f, 0.412f, 0.0f);

	glVertex3f(-0.94f, 0.412f, 0.0f);

	glEnd();
}

void LetterOnTopPlaneA(void)
{
	/*glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter A
	//A diagonal line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.90f, 0.499f, 0.0f);

	glVertex3f(-0.91f, 0.481f, 0.0f);

	glVertex3f(-0.90f, 0.499f, 0.0f);

	glVertex3f(-0.89f, 0.481f, 0.0f);

	//A horizontal line
	glVertex3f(-0.908f, 0.49f, 0.0f);

	glVertex3f(-0.895f, 0.49f, 0.0f);


	glEnd();*/

	glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter A
	//A diagonal line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.90f, 0.43f, 0.0f);

	glVertex3f(-0.91f, 0.412f, 0.0f);

	glVertex3f(-0.90f, 0.43f, 0.0f);

	glVertex3f(-0.89f, 0.412f, 0.0f);

	//A horizontal line
	glVertex3f(-0.908f, 0.42f, 0.0f);

	glVertex3f(-0.895f, 0.42f, 0.0f);


	glEnd();

}

void LetterOnTopPlaneF(void)
{
	/*glLineWidth(3.0f);

	glBegin(GL_LINES);
	//Letter F
	//F vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.88f, 0.499f, 0.0f);

	glVertex3f(-0.88f, 0.481f, 0.0f);

	//horizontal lines
	glVertex3f(-0.88f, 0.499f, 0.0f);

	glVertex3f(-0.86f, 0.499f, 0.0f);

	glVertex3f(-0.88f, 0.49f, 0.0f);

	glVertex3f(-0.86f, 0.49f, 0.0f);

	glEnd();*/

	glLineWidth(3.0f);

	glBegin(GL_LINES);
	//Letter F
	//F vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.88f, 0.43f, 0.0f);

	glVertex3f(-0.88f, 0.412f, 0.0f);

	//horizontal lines
	glVertex3f(-0.88f, 0.43f, 0.0f);

	glVertex3f(-0.86f, 0.43f, 0.0f);

	glVertex3f(-0.88f, 0.42f, 0.0f);

	glVertex3f(-0.86f, 0.42f, 0.0f);

	glEnd();
}

//##################################################################################################################

void LetterOnBottomPlaneI(void)
{
	/*glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.93f, -0.499f, 0.0f);

	glVertex3f(-0.93f, -0.481f, 0.0f);

	//I horizontal lines

	glVertex3f(-0.92f, -0.499f, 0.0f);

	glVertex3f(-0.94f, -0.499f, 0.0f);

	//I horizontal lines
	glVertex3f(-0.92f, -0.481f, 0.0f);

	glVertex3f(-0.94f, -0.481f, 0.0f);

	glEnd();*/

	glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.93f, -0.43f, 0.0f);

	glVertex3f(-0.93f, -0.412f, 0.0f);

	//I horizontal lines

	glVertex3f(-0.92f, -0.43f, 0.0f);

	glVertex3f(-0.94f, -0.43f, 0.0f);

	//I horizontal lines
	glVertex3f(-0.92f, -0.412f, 0.0f);

	glVertex3f(-0.94f, -0.412f, 0.0f);

	glEnd();
}

void LetterOnBottomPlaneA(void)
{
	/*glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter A
	//A diagonal line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.90f, -0.481f, 0.0f);

	glVertex3f(-0.91f, - 0.499f, 0.0f);

	glVertex3f(-0.90f, -0.481f, 0.0f);

	glVertex3f(-0.89f, - 0.499f, 0.0f);

	//A horizontal line
	glVertex3f(-0.908f, -0.49f, 0.0f);

	glVertex3f(-0.895f, -0.49f, 0.0f);


	glEnd();*/

	glLineWidth(3.0f);

	glBegin(GL_LINES);

	//Letter A
	//A diagonal line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.90f, -0.412f, 0.0f);

	glVertex3f(-0.91f, -0.43f, 0.0f);

	glVertex3f(-0.90f, -0.412f, 0.0f);

	glVertex3f(-0.89f, -0.43f, 0.0f);

	//A horizontal line
	glVertex3f(-0.908f, -0.42f, 0.0f);

	glVertex3f(-0.895f, -0.42f, 0.0f);


	glEnd();

}

void LetterOnBottomPlaneF(void)
{
	/*glLineWidth(3.0f);

	glBegin(GL_LINES);
	//Letter F
	//F vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.88f, -0.499f, 0.0f);

	glVertex3f(-0.88f, -0.481f, 0.0f);

	//horizontal lines
	glVertex3f(-0.88f, -0.488f, 0.0f);

	glVertex3f(-0.86f, -0.488f, 0.0f);

	glVertex3f(-0.88f, -0.49f, 0.0f);

	glVertex3f(-0.86f, -0.49f, 0.0f);

	glEnd();*/

	glLineWidth(3.0f);

	glBegin(GL_LINES);
	//Letter F
	//F vertical line
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-0.88f, -0.43f, 0.0f);

	glVertex3f(-0.88f, -0.412f, 0.0f);

	//horizontal lines
	glVertex3f(-0.88f, -0.412f, 0.0f);

	glVertex3f(-0.86f, -0.412f, 0.0f);

	glVertex3f(-0.88f, -0.42f, 0.0f);

	glVertex3f(-0.86f, -0.42f, 0.0f);

	glEnd();
}

//################################################################################################################

void DrawTopPlane(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//GLfloat fxTrans = -4.0f, fytrans = 2.3f, angle = -90.0f;
	/*angle = angle + 0.001f;

	if (angle < 2.0f*PI*0.75)
	{
		glTranslatef(cos(angle) +0.5, sin(angle) + 0.6, -1.5f);
	}*/

	//glTranslatef(p2x, -p2y, -1.5f);
	//glRotatef(angle, 0.0f, 0.0f, 1.0f);
	//glTranslatef(p2x, -0.1f, -1.5f);

	/*glLineWidth(3.0f);

	glBegin(GL_POINTS);


	glColor3f(1.0f, 0.0f, 0.0f); //red

	for (GLfloat angle = 0.0f;angle < 2.0f*PI*0.25;angle = angle + 0.001f)
	{
		glVertex3f(cos(angle) - 1.9, sin(angle) +0.0, -1.5f);
	}

	glEnd();*/
//________________________________________________________________________________________________________
	/*glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, 0.48f, 0.0f);

	glVertex3f(-0.96f, 0.5f, 0.0f);

	glVertex3f(-0.85f, 0.5f, 0.0f);

	glVertex3f(-0.85f, 0.48f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//lower wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.91f, 0.48f, 0.0f);

	glVertex3f(-0.93f, 0.44f, 0.0f);

	glVertex3f(-0.96f, 0.44f, 0.0f);

	glVertex3f(-0.88f, 0.48f, 0.0f);


	glEnd();


	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//upper wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.91f, 0.5f, 0.0f);

	glVertex3f(-0.93f, 0.54f, 0.0f);

	glVertex3f(-0.96f, 0.54f, 0.0f);

	glVertex3f(-0.88f, 0.5f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail up
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, 0.5f, 0.0f);

	glVertex3f(-0.99f, 0.52f, 0.0f);

	glVertex3f(-0.97f, 0.52f, 0.0f);

	glVertex3f(-0.94f, 0.5f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	


	//filled circle
	float tx1, ty1, tx2, ty2;
	float angle1;
	double radius1 = 0.01;

	tx1 = -0.85, ty1 = 0.49;

	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(tx1, ty1);

	for (angle1 = 1.0f;angle1<361.0f;angle1 += 0.2)
	{
		tx2 = tx1 + sin(angle1)*radius1;
		ty2 = ty1 + cos(angle1)*radius1;
		glVertex2f(tx2, ty2);
	}

	glEnd();*/



	glTranslatef(p2x, -p2y, -1.4f);

	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, 0.44f, 0.0f);

	glVertex3f(-0.96f, 0.40f, 0.0f);

	glVertex3f(-0.80f, 0.385f, 0.0f);

	glVertex3f(-0.80f, 0.455f, 0.0f);

	glEnd();

	//------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.80f, 0.385f, 0.0f);

	glVertex3f(-0.80f, 0.455f, 0.0f);

	glVertex3f(-0.75f, 0.430f, 0.0f);

	glVertex3f(-0.75f, 0.410f, 0.0f);

	glEnd();

	//-----------------------------------------------------------------------------------------------------
	//filled circle
	float x1, y1, x2, y2;
	float angle;
	double radius = 0.01;

	x1 = -0.75, y1 = 0.420;

	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x1, y1);

	for (angle = 1.0f;angle<361.0f;angle += 0.2)
	{
		x2 = x1 + sin(angle)*radius;
		y2 = y1 + cos(angle)*radius;
		glVertex2f(x2, y2);
	}

	glEnd();

	//---------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//upper wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.92f, 0.44f, 0.0f);

	glVertex3f(-0.92f, 0.50f, 0.0f);

	glVertex3f(-0.89f, 0.50f, 0.0f);

	glVertex3f(-0.84f, 0.44f, 0.0f);

	glEnd();

	//---------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//lower wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.92f, 0.40f, 0.0f);

	glVertex3f(-0.92f, 0.34f, 0.0f);

	glVertex3f(-0.89f, 0.34f, 0.0f);

	glVertex3f(-0.84f, 0.40f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail up
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.95f, 0.42f, 0.0f);

	glVertex3f(-0.97f, 0.42f, 0.0f);

	glVertex3f(-0.99f, 0.46f, 0.0f);

	glVertex3f(-0.97f, 0.46f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail down
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.95f, 0.42f, 0.0f);

	glVertex3f(-0.97f, 0.42f, 0.0f);

	glVertex3f(-0.99f, 0.38f, 0.0f);

	glVertex3f(-0.97f, 0.38f, 0.0f);

	glEnd();
}

void DrawMiddlePlane(void)
{
	//Middle Plane

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/*glTranslatef(p1x, 0.0f, -1.5f);

	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, 0.01f, 0.0f);

	glVertex3f(-0.96f, -0.01f, 0.0f);

	glVertex3f(-0.85f, -0.01f, 0.0f);

	glVertex3f(-0.85f, 0.01f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);
        
        
        //lower wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.91f, -0.01f, 0.0f);

	glVertex3f(-0.93f, -0.05f, 0.0f);

	glVertex3f(-0.96f, -0.05f, 0.0f);

	glVertex3f(-0.88f, -0.01f, 0.0f);


	glEnd();


	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//upper wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.91f, 0.01f, 0.0f);

	glVertex3f(-0.93f, 0.05f, 0.0f);

	glVertex3f(-0.96f, 0.05f, 0.0f);

	glVertex3f(-0.88f, 0.01f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, 0.01f, 0.0f);

	glVertex3f(-0.99f, 0.03f, 0.0f);

	glVertex3f(-0.97f, 0.03f, 0.0f);

	glVertex3f(-0.94f, 0.01f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________

	//filled circle
	float x1, y1, x2, y2;
	float angle;
	double radius = 0.01;

	x1 = -0.85, y1 = 0.0;

	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x1, y1);

	for (angle = 1.0f;angle<361.0f;angle += 0.2)
	{
		x2 = x1 + sin(angle)*radius;
		y2 = y1 + cos(angle)*radius;
		glVertex2f(x2, y2);
	}

	glEnd();*/

	glTranslatef(p1x, 0.0f, -1.4f);

	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, 0.02f, 0.0f);

	glVertex3f(-0.96f, -0.02f, 0.0f);

	glVertex3f(-0.80f, -0.035f, 0.0f);

	glVertex3f(-0.80f, 0.035f, 0.0f);

	glEnd();

	//------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.80f, -0.035f, 0.0f);

	glVertex3f(-0.80f, 0.035f, 0.0f);

	glVertex3f(-0.75f, 0.01f, 0.0f);

	glVertex3f(-0.75f, -0.01f, 0.0f);

	glEnd();

	//-----------------------------------------------------------------------------------------------------
	//filled circle
	float x1, y1, x2, y2;
	float angle;
	double radius = 0.01;

	x1 = -0.75, y1 = 0.0;

	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x1, y1);

	for (angle = 1.0f;angle<361.0f;angle += 0.2)
	{
		x2 = x1 + sin(angle)*radius;
		y2 = y1 + cos(angle)*radius;
		glVertex2f(x2, y2);
	}

	glEnd();

	//---------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//upper wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.92f, 0.02f, 0.0f);

	glVertex3f(-0.92f, 0.09f, 0.0f);

	glVertex3f(-0.89f, 0.09f, 0.0f);

	glVertex3f(-0.84f, 0.02f, 0.0f);

	glEnd();

	//---------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//lower wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.92f, -0.02f, 0.0f);

	glVertex3f(-0.92f, -0.09f, 0.0f);

	glVertex3f(-0.89f, -0.09f, 0.0f);

	glVertex3f(-0.84f, -0.02f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail up
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.95f, 0.00f, 0.0f);

	glVertex3f(-0.97f, 0.00f, 0.0f);

	glVertex3f(-0.99f, 0.04f, 0.0f);

	glVertex3f(-0.97f, 0.04f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail down
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.95f, 0.00f, 0.0f);

	glVertex3f(-0.97f, 0.00f, 0.0f);

	glVertex3f(-0.99f, -0.04f, 0.0f);

	glVertex3f(-0.97f, -0.04f, 0.0f);

	glEnd();
}

void DrawBottomPlane(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/*glTranslatef(p3x, p3y, -1.5f);

	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, -0.48f, 0.0f);

	glVertex3f(-0.96f, -0.5f, 0.0f);

	glVertex3f(-0.85f, -0.5f, 0.0f);

	glVertex3f(-0.85f, -0.48f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//lower wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.91f, -0.48f, 0.0f);

	glVertex3f(-0.93f, -0.44f, 0.0f);

	glVertex3f(-0.96f, -0.44f, 0.0f);

	glVertex3f(-0.88f, -0.48f, 0.0f);


	glEnd();


	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//upper wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.91f, -0.5f, 0.0f);

	glVertex3f(-0.93f, -0.54f, 0.0f);

	glVertex3f(-0.96f, -0.54f, 0.0f);

	glVertex3f(-0.88f, -0.5f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, -0.48f, 0.0f);

	glVertex3f(-0.99f, -0.46f, 0.0f);

	glVertex3f(-0.97f, -0.46f, 0.0f);

	glVertex3f(-0.94f, -0.48f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________

	//filled circle
	float bx1, by1, bx2, by2;
	float angle2;
	double radius2 = 0.01;

	bx1 = -0.85, by1 = -0.49;

	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(bx1, by1);

	for (angle2 = 1.0f;angle2<361.0f;angle2 += 0.2)
	{
		bx2 = bx1 + sin(angle2)*radius2;
		by2 = by1 + cos(angle2)*radius2;
		glVertex2f(bx2, by2);
	}

	glEnd();*/
	glTranslatef(p3x, p3y, -1.4f);

	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.96f, -0.44f, 0.0f);

	glVertex3f(-0.96f, -0.40f, 0.0f);

	glVertex3f(-0.80f, -0.385f, 0.0f);

	glVertex3f(-0.80f, -0.455f, 0.0f);

	glEnd();

	//------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//rectangle
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.80f, -0.385f, 0.0f);

	glVertex3f(-0.80f, -0.455f, 0.0f);

	glVertex3f(-0.75f, -0.430f, 0.0f);

	glVertex3f(-0.75f, -0.410f, 0.0f);

	glEnd();

	//-----------------------------------------------------------------------------------------------------
	//filled circle
	float x1, y1, x2, y2;
	float angle;
	double radius = 0.01;

	x1 = -0.75, y1 = -0.420;

	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x1, y1);

	for (angle = 1.0f;angle<361.0f;angle += 0.2)
	{
		x2 = x1 + sin(angle)*radius;
		y2 = y1 + cos(angle)*radius;
		glVertex2f(x2, y2);
	}

	glEnd();

	//---------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//upper wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.92f, -0.44f, 0.0f);

	glVertex3f(-0.92f, -0.50f, 0.0f);

	glVertex3f(-0.89f, -0.50f, 0.0f);

	glVertex3f(-0.84f, -0.44f, 0.0f);

	glEnd();

	//---------------------------------------------------------------------------------------------------------
	glBegin(GL_QUADS);

	//lower wing
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.92f, -0.40f, 0.0f);

	glVertex3f(-0.92f, -0.34f, 0.0f);

	glVertex3f(-0.89f, -0.34f, 0.0f);

	glVertex3f(-0.84f, -0.40f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail up
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.95f, -0.42f, 0.0f);

	glVertex3f(-0.97f, -0.42f, 0.0f);

	glVertex3f(-0.99f, -0.46f, 0.0f);

	glVertex3f(-0.97f, -0.46f, 0.0f);

	glEnd();

	//_______________________________________________________________________________________________________________
	glBegin(GL_QUADS);

	//back tail down
	glColor3f(0.7294117647f, 0.8862745098, 0.9333333f);  //186,226,238 -powder blue

	glVertex3f(-0.95f, -0.42f, 0.0f);

	glVertex3f(-0.97f,- 0.42f, 0.0f);

	glVertex3f(-0.99f, -0.38f, 0.0f);

	glVertex3f(-0.97f, -0.38f, 0.0f);

	glEnd();
	
}

//##################################################################################################################

void DrawLetterI1(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(i1x, i1y, -1.5f);

	glLineWidth(60.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.56f, 0.3f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.56f, -0.3f, 0.0f);

	//I horizontal lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.65f, 0.3f, 0.0f);

	glVertex3f(-0.47f, 0.3f, 0.0f);

	//I horizontal lines
	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.65f, -0.3f, 0.0f);

	glVertex3f(-0.47f, -0.3f, 0.0f);

	glEnd();
}

void DrawLetterN(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(nx, ny, -1.5f);

	glLineWidth(60.0f);

	glBegin(GL_LINES);

	//Letter N
	//N vertical lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.37f, 0.3f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.37f, -0.3f, 0.0f);



	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.19f, 0.3f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.19f, -0.3f, 0.0f);


	//N diagonal line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.37f, 0.3f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.19f, -0.3f, 0.0f);


	glEnd();
}

void DrawLetterD(void)
{
	/*//saffron color
	static float s1 = 0.0f;
	static float s2 = 0.0f;
	static float s3 = 0.0f;

	//green color
	static float g1 = 0.0f;
	static float g2 = 0.0f;
	static float g3 = 0.0f;*/


	//glColor3f(1.0f, 0.6f, 0.2f);
	//glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -1.5f);

	glLineWidth(60.0f);

	glBegin(GL_LINES);

	//Letter D
	//D vertical lines
	glColor3f(s1, s2, s3);

	glVertex3f(-0.09f, 0.3f, 0.0f);


	glColor3f(g1, g2, g3);

	glVertex3f(-0.09f, -0.3f, 0.0f);


	glColor3f(s1, s2, s3);

	glVertex3f(0.09f, 0.2f, 0.0f);


	glColor3f(g1, g2, g3);

	glVertex3f(0.09f, -0.2f, 0.0f);


	//D horizontal lines
	glColor3f(s1, s2, s3);

	glVertex3f(-0.09f, 0.3f, 0.0f);

	glVertex3f(0.04f, 0.3f, 0.0f);


	glColor3f(g1, g2, g3);

	glVertex3f(-0.09f, -0.3f, 0.0f);

	glVertex3f(0.04f, -0.3f, 0.0f);


	//D diagonal lines

	glColor3f(s1, s2, s3);

	glVertex3f(0.04f, 0.3f, 0.0f);

	glVertex3f(0.09f, 0.2f, 0.0f);


	glColor3f(g1, g2, g3);

	glVertex3f(0.04f, -0.3f, 0.0f);

	glVertex3f(0.09f, -0.2f, 0.0f);


	glEnd();
}

void DrawLetterI2(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(i2x, i2y, -1.5f);

	glLineWidth(60.0f);

	glBegin(GL_LINES);

	//Letter I
	//I vertical line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.28f, 0.3f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.28f, -0.3f, 0.0f);

	//I horizontal lines
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.19f, 0.3f, 0.0f);

	glVertex3f(0.37f, 0.3f, 0.0f);

	//I horizontal lines
	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.19f, -0.3f, 0.0f);

	glVertex3f(0.37f, -0.3f, 0.0f);

	glEnd();
}

void DrawLetterA(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(ax, ay, -1.5f);

	glLineWidth(60.0f);

	glBegin(GL_LINES);

	//Letter A
	//A diagonal line
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.56f, 0.3f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.47f, -0.3f, 0.0f);


	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(0.56f, 0.3f, 0.0f);


	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(0.65f, -0.3f, 0.0f);


	glEnd();
}

//#################################################################################################################

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

void PositionPlane(void)
{
	//for movement of middle plane
	p1x = p1x + 0.001;


	//for movement of top plane
	p2x = p2x + 0.001;

	p2y = p2y + 0.001;
	if (p2y >= 0.42)
		p2y = 0.42f;

	if (p2x >=1.6f)
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

//################################################################################################################

void PositionFlag(void)
{
	//glColor3f(1.0f, 0.6f, 0.2f);
	//glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	mfs1 = mfs1 + 0.001f;

	if (mfs1 >= 1.0f)
	{
		mfs1 = 1.0f;
	}
		
	mfs2 = mfs2 + 0.001f;

	if (mfs2 >= 0.6f)
	{
		mfs2 = 0.6f;
	}
		
	mfs3 = mfs3 + 0.001f;

	if (mfs3 >= 0.2f)
	{
		mfs3 = 0.2f;
	}
	
}

//################################################################################################################

void positionFlagToA(void)
{
	//for letter A

	//glColor3f(1.0f, 0.6f, 0.2f);
	//glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);


	//saffron color
	As1 = As1 + 0.001f;

	if (As1 >= 1.0f)
		As1 = 1.0f;


	As2 =As2 + 0.001f;

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

//#################################################################################################################

void TriFlagMidPlane(void)
{
	//saffron color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p1x, 0.0f, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//saffron color
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.96f, 0.01f, 0.0f);

	glVertex3f(-1.2f, 0.01f, 0.0f);

	glEnd();


	//white color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p1x, 0.0f, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	glColor3f(1.0f, 1.0f, 1.0f);

	glVertex3f(-0.96f, 0.0f, 0.0f);

	glVertex3f(-1.2f, 0.0f, 0.0f);

	glEnd();


	//green color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p1x, 0.0f, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.96f, -0.01f, 0.0f);

	glVertex3f(-1.2f, -0.01f, 0.0f);

	glEnd();
}

void TriFlagTopPlane(void)
{
	//saffron color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p2x, -p2y, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//saffron color
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.96f, 0.41f, 0.0f);

	glVertex3f(-1.2f, 0.41f, 0.0f);

	glEnd();


	//white color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p2x, -p2y, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	glColor3f(1.0f, 1.0f, 1.0f);

	glVertex3f(-0.96f, 0.42f, 0.0f);

	glVertex3f(-1.2f, 0.42f, 0.0f);

	glEnd();


	//green color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p2x, -p2y, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.96f, 0.43f, 0.0f);

	glVertex3f(-1.2f, 0.43f, 0.0f);

	glEnd();
}

void TriFlagBottomPlane(void)
{
	//saffron color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p3x, p3y, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	//saffron color
	glColor3f(1.0f, 0.6f, 0.2f);

	glVertex3f(-0.96f, -0.43f, 0.0f);

	glVertex3f(-1.2f, -0.43f, 0.0f);

	glEnd();


	//white color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p3x, p3y, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	glColor3f(1.0f, 1.0f, 1.0f);

	glVertex3f(-0.96f, -0.42f, 0.0f);

	glVertex3f(-1.2f, -0.42f, 0.0f);

	glEnd();


	//green color 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(p3x, p3y, -1.4f);

	glLineWidth(9.0f);

	glBegin(GL_LINES);

	glColor3f(0.0705882353f, 0.53333f, 0.0274509804f);

	glVertex3f(-0.96f, -0.41f, 0.0f);

	glVertex3f(-1.2f, -0.41f, 0.0f);

	glEnd();
}

void FlagToA(void)
{
		//saffron color 

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glTranslatef(ax, ay, -1.5f);

		glLineWidth(9.0f);

		glBegin(GL_LINES);

		glColor3f(As1, As2, As3);

		glVertex3f(0.542f, 0.01f, 0.0f);

		glVertex3f(0.578f, 0.01f, 0.0f);

		glEnd();


		//white color 

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glTranslatef(ax, ay, -1.5f);

		glLineWidth(9.0f);

		glBegin(GL_LINES);


		glColor3f(Aw1, Aw2, Aw3);

		glVertex3f(0.54f, 0.0f, 0.0f);

		glVertex3f(0.58f, 0.0f, 0.0f);

		glEnd();


		//green color 

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glTranslatef(ax, ay, -1.5f);

		glLineWidth(9.0f);

		glBegin(GL_LINES);

		glColor3f(Ag1, Ag2, Ag3);

		glVertex3f(0.538f, -0.01f, 0.0f);

		glVertex3f(0.581f, -0.01f, 0.0f);

		glEnd();
}

//***************************************************************************************************************



void display(void)
{
        void DrawLetterI1(void);
	void DrawLetterN(void);
	void DrawLetterD(void);
	void DrawLetterI2(void);
	void DrawLetterA(void);


	void DrawTopPlane(void);
	void DrawMiddlePlane(void);
	void DrawBottomPlane(void);


	void LetterOnMiddlePlaneI(void);
	void LetterOnMiddlePlaneA(void);
	void LetterOnMiddlePlaneF(void);


	void LetterOnTopPlaneI(void);
	void LetterOnTopPlaneA(void);
	void LetterOnTopPlaneF(void);


	void LetterOnBottomPlaneI(void);
	void LetterOnBottomPlaneA(void);
	void LetterOnBottomPlaneF(void);

	


	//---------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------


	glClear(GL_COLOR_BUFFER_BIT);
	
	//Sleep(1);

	DrawLetterI1();
	
	DrawLetterN();
	
	DrawLetterD();
	
	DrawLetterI2();
	
	DrawLetterA();
	FlagToA();
		
	//-------------------------------------------------------------------------------------------------------------
	
	//Middle plane   
	
	//Sleep(0.9999);

	DrawMiddlePlane();
	
	LetterOnMiddlePlaneI();
	
	LetterOnMiddlePlaneA();

	LetterOnMiddlePlaneF();

	TriFlagMidPlane();

	//PositionFlag();
	//--------------------------------------------------------------------------------------------------------------

	//Top plane   


	//Sleep(0.9999);

	DrawTopPlane();
	                                                                
	LetterOnTopPlaneI();

	LetterOnTopPlaneA();

	LetterOnTopPlaneF();

	TriFlagTopPlane();

	//--------------------------------------------------------------------------------------------------------------

	//Bottom plane  

	//Sleep(0.9999);

	DrawBottomPlane();

	LetterOnBottomPlaneI();

	LetterOnBottomPlaneA();

	LetterOnBottomPlaneF();

	TriFlagBottomPlane();

	//--------------------------------------------------------------------------------------------------------------


	
	glXSwapBuffers(gpDisplay,gWindow);
	
	//--------------------------------------------------------------------------------------------------------------

	//Move letters
	PositionLetter();

	//Move Planes
	PositionPlane();

	//Fade In flag to A
	positionFlagToA();
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

