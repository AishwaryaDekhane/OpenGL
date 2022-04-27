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

GLfloat AngleOfXRotation = 0.0f;
GLfloat AngleOfYRotation = 0.0f;
GLfloat AngleOfZRotation = 0.0f;

static GLXContext gGLXContext;
char keys[26];

GLint keyPress = 0;

bool bLight = false;

GLfloat LightAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat LightDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat LightPosition[] = { 0.0f,0.0f,0.0f,1.0f };

GLfloat Light_Model_Ambient[] = { 0.2f,0.2f,0.2f,1.0f };
GLfloat Light_Model_Local_Viewer[] = { 0.0f };

GLUquadric *quadric [26];

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
						
						
				 case 'X':
		                 case 'x':
			                keyPress = 1;
			                AngleOfXRotation = 0.0f;
			                break;

		                case 'y':
		                case 'Y':
		                	keyPress = 2;
		                	AngleOfYRotation = 0.0f;
			                break;

		                case 'z':
		                case 'Z':
		                      	keyPress = 3;
		                	AngleOfZRotation = 0.0f;
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
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);

	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, Light_Model_Ambient);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, Light_Model_Local_Viewer);

	glEnable(GL_LIGHT0);

	for (int i = 0;i <= 24;i++)
	{
		quadric[i] = gluNewQuadric();
	}
}


void update(void)
{
	AngleOfXRotation = AngleOfXRotation + 0.05f;
	if (AngleOfXRotation >= 360.0f)
		AngleOfXRotation = AngleOfXRotation - 360.0f;

	AngleOfYRotation = AngleOfYRotation + 0.05f;
	if (AngleOfYRotation >= 360.0f)
		AngleOfYRotation = AngleOfYRotation - 360.0f;

	AngleOfZRotation = AngleOfZRotation + 0.05f;
	if (AngleOfZRotation >= 360.0f)
		AngleOfZRotation = AngleOfZRotation - 360.0f;
}

void Draw24Spheres(void)
{
	GLfloat MaterialAmbient[4];
	GLfloat MaterialDiffuse[4];;
	GLfloat MaterialSpecular[4];
	GLfloat MaterialShininess[1];

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//-------------------- 1st sphere on 1st column, emerald ------------------------------

	MaterialAmbient[0] = 0.0215f;	//r
	MaterialAmbient[1] = 0.1745f;	//g
	MaterialAmbient[2] = 0.0215f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.07568f;	//r
	MaterialDiffuse[1] = 0.61424f;	//g
	MaterialDiffuse[2] = 0.07568f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.633f;	//r
	MaterialSpecular[1] = 0.727811f;	//g
	MaterialSpecular[2] = 0.633f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.6f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(1.5f, 14.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30); //draw sphere

										 //-------------------- 2nd sphere on 1st column, jade ------------------------------

	MaterialAmbient[0] = 0.135f;	//r
	MaterialAmbient[1] = 0.2225f;	//g
	MaterialAmbient[2] = 0.1575f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.54f;	//r
	MaterialDiffuse[1] = 0.89f;	//g
	MaterialDiffuse[2] = 0.63f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.316228f;	//r
	MaterialSpecular[1] = 0.316228f;	//g
	MaterialSpecular[2] = 0.316228f; //b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.1f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(1.5f, 11.65f, 0.0f);
	gluSphere(quadric[1], 1.0f, 30, 30); //draw sphere

										 //-------------------- 3rd sphere on 1st column, obsidian ------------------------------

	MaterialAmbient[0] = 0.05375f;	//r
	MaterialAmbient[1] = 0.05f;	//g
	MaterialAmbient[2] = 0.06625f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.18275f;	//r
	MaterialDiffuse[1] = 0.17f;	//g
	MaterialDiffuse[2] = 0.22525f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.332741f;	//r
	MaterialSpecular[1] = 0.328634f;	//g
	MaterialSpecular[2] = 0.346435f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.3f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(1.5f, 9.32f, 0.0f);
	gluSphere(quadric[2], 1.0f, 30, 30); //draw sphere

										 //-------------------- 4th sphere on 1st column, pearl ------------------------------

	MaterialAmbient[0] = 0.25f;	//r
	MaterialAmbient[1] = 0.20725f;	//g
	MaterialAmbient[2] = 0.20725f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 1.0f;	//r
	MaterialDiffuse[1] = 0.829f;	//g
	MaterialDiffuse[2] = 0.829f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.296648f;	//r
	MaterialSpecular[1] = 0.296648f;	//g
	MaterialSpecular[2] = 0.296648f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.088f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(1.5f, 6.99f, 0.0f);
	gluSphere(quadric[3], 1.0f, 30, 30); //draw sphere

										 //-------------------- 5th sphere on 1st column, ruby ------------------------------

	MaterialAmbient[0] = 0.1745f;	//r
	MaterialAmbient[1] = 0.01175f;	//g
	MaterialAmbient[2] = 0.01175f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.61424f;	//r
	MaterialDiffuse[1] = 0.04136f;	//g
	MaterialDiffuse[2] = 0.04136f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.727811f;	//r
	MaterialSpecular[1] = 0.626959f;	//g
	MaterialSpecular[2] = 0.626959f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.6f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(1.5f, 4.66f, 0.0f);
	gluSphere(quadric[4], 1.0f, 30, 30); //draw sphere

										 //-------------------- 6th sphere on 1st column, turquoise ------------------------------

	MaterialAmbient[0] = 0.1f;	//r
	MaterialAmbient[1] = 0.18725f;	//g
	MaterialAmbient[2] = 0.1745f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.396f;	//r
	MaterialDiffuse[1] = 0.74151f;	//g
	MaterialDiffuse[2] = 0.69102f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.297254f;	//r
	MaterialSpecular[1] = 0.30829f;	//g
	MaterialSpecular[2] = 0.306678f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.1f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(1.5f, 2.33f, 0.0f);
	gluSphere(quadric[5], 1.0f, 30, 30); //draw sphere

										 //-------------------- 7- 1st sphere on 2nd column, brass ------------------------------

	MaterialAmbient[0] = 0.329412f;	//r
	MaterialAmbient[1] = 0.223529f;	//g
	MaterialAmbient[2] = 0.027451f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.780392f;	//r
	MaterialDiffuse[1] = 0.568627f;	//g
	MaterialDiffuse[2] = 0.113725f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.992157f;	//r
	MaterialSpecular[1] = 0.941176f;	//g
	MaterialSpecular[2] = 0.807843f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.21794872f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(9.5f, 14.0f, 0.0f);
	gluSphere(quadric[6], 1.0f, 30, 30); //draw sphere

										 //-------------------- 8- 2nd sphere on 2nd column, bronze ------------------------------

	MaterialAmbient[0] = 0.2125f;	//r
	MaterialAmbient[1] = 0.1275f;	//g
	MaterialAmbient[2] = 0.054f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.714f;	//r
	MaterialDiffuse[1] = 0.4284f;	//g
	MaterialDiffuse[2] = 0.18144f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.393548f;	//r
	MaterialSpecular[1] = 0.271906f;	//g
	MaterialSpecular[2] = 0.166721f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.2f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(9.5f, 11.65f, 0.0f);
	gluSphere(quadric[8], 1.0f, 30, 30); //draw sphere

										 //-------------------- 9- 3rd sphere on 2nd column, chrome ------------------------------

	MaterialAmbient[0] = 0.25f;	//r
	MaterialAmbient[1] = 0.25f;	//g
	MaterialAmbient[2] = 0.25f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.4f;	//r
	MaterialDiffuse[1] = 0.4f;	//g
	MaterialDiffuse[2] = 0.4f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.774597f;	//r
	MaterialSpecular[1] = 0.774597f;	//g
	MaterialSpecular[2] = 0.774597f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.6f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(9.5f, 9.32f, 0.0f);
	gluSphere(quadric[9], 1.0f, 30, 30); //draw sphere

										 //-------------------- 10- 4th sphere on 2nd column, copper ------------------------------

	MaterialAmbient[0] = 0.19125f;	//r
	MaterialAmbient[1] = 0.0735f;	//g
	MaterialAmbient[2] = 0.0225f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.7038f;	//r
	MaterialDiffuse[1] = 0.27048f;	//g
	MaterialDiffuse[2] = 0.0828f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.256777f;	//r
	MaterialSpecular[1] = 0.137622f;	//g
	MaterialSpecular[2] = 0.086014f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.1f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(9.5f, 6.99f, 0.0f);
	gluSphere(quadric[10], 1.0f, 30, 30); //draw sphere

										 //-------------------- 11- 5th sphere on 2nd column, gold ------------------------------

	MaterialAmbient[0] = 0.24725f;	//r
	MaterialAmbient[1] = 0.1995f;	//g
	MaterialAmbient[2] = 0.0745f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.75164f;	//r
	MaterialDiffuse[1] = 0.60648f;	//g
	MaterialDiffuse[2] = 0.22648f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.628281f;	//r
	MaterialSpecular[1] = 0.555802f;	//g
	MaterialSpecular[2] = 0.366065f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.4f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(9.5f, 4.66f, 0.0f);
	gluSphere(quadric[11], 1.0f, 30, 30); //draw sphere

										 //-------------------- 12- 6th sphere on 2nd column, silver ------------------------------

	MaterialAmbient[0] = 0.19225f;	//r
	MaterialAmbient[1] = 0.19225f;	//g
	MaterialAmbient[2] = 0.19225f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.50754f;	//r
	MaterialDiffuse[1] = 0.50754f;	//g
	MaterialDiffuse[2] = 0.50754f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.508273f;	//r
	MaterialSpecular[1] = 0.508273f;	//g
	MaterialSpecular[2] = 0.508273f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.4f * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(9.5f, 2.33f, 0.0f);
	gluSphere(quadric[12], 1.0f, 30, 30); //draw sphere

										 //-------------------- 13- 1st sphere on 3rd column, black plastic ------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.01f;	//r
	MaterialDiffuse[1] = 0.01f;	//g
	MaterialDiffuse[2] = 0.01f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.50f;	//r
	MaterialSpecular[1] = 0.50f;	//g
	MaterialSpecular[2] = 0.50f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(17.5f, 14.0f, 0.0f);
	gluSphere(quadric[13], 1.0f, 30, 30); //draw sphere

										 //-------------------- 14- 2nd sphere on 3rd column, cyan plastic ------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.1f;	//g
	MaterialAmbient[2] = 0.06f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.0f;	//r
	MaterialDiffuse[1] = 0.50980392f;	//g
	MaterialDiffuse[2] = 0.50980392f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.50196078f;	//r
	MaterialSpecular[1] = 0.50196078f;	//g
	MaterialSpecular[2] = 0.50196078f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(17.5f, 11.65f, 0.0f);
	gluSphere(quadric[14], 1.0f, 30, 30); //draw sphere

										 //-------------------- 15- 3rd sphere on 2nd column, green plastic------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.1f;	//r
	MaterialDiffuse[1] = 0.35f;	//g
	MaterialDiffuse[2] = 0.1f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.45f;	//r
	MaterialSpecular[1] = 0.55f;	//g
	MaterialSpecular[2] = 0.45f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(17.5f, 9.32f, 0.0f);
	gluSphere(quadric[15], 1.0f, 30, 30); //draw sphere

										 //-------------------- 16- 4th sphere on 3rd column, red plastic------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.5f;	//r
	MaterialDiffuse[1] = 0.0f;	//g
	MaterialDiffuse[2] = 0.0f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.7f;	//r
	MaterialSpecular[1] = 0.6f;	//g
	MaterialSpecular[2] = 0.6f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(17.5f, 6.99f, 0.0f);
	gluSphere(quadric[16], 1.0f, 30, 30); //draw sphere

										 //-------------------- 17- 5th sphere on 3rd column, white plastic------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.02f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.55f;	//r
	MaterialDiffuse[1] = 0.55f;	//g
	MaterialDiffuse[2] = 0.55f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.70f;	//r
	MaterialSpecular[1] = 0.70f;	//g
	MaterialSpecular[2] = 0.70f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(17.5f, 4.66f, 0.0f);
	gluSphere(quadric[17], 1.0f, 30, 30); //draw sphere

										 //-------------------- 18- 6th sphere on 3rd column, yellow plastic ------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.5f;	//r
	MaterialDiffuse[1] = 0.5f;	//g
	MaterialDiffuse[2] = 0.0f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.60f;	//r
	MaterialSpecular[1] = 0.60f;	//g
	MaterialSpecular[2] = 0.50f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(17.5f, 2.33f, 0.0f);
	gluSphere(quadric[18], 1.0f, 30, 30); //draw sphere

										 //-------------------- 19- 1st sphere on 4th column, black rubber ------------------------------

	MaterialAmbient[0] = 0.02f;	//r
	MaterialAmbient[1] = 0.02f;	//g
	MaterialAmbient[2] = 0.02f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.01f;	//r
	MaterialDiffuse[1] = 0.01f;	//g
	MaterialDiffuse[2] = 0.01f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.4f;	//r
	MaterialSpecular[1] = 0.4f;	//g
	MaterialSpecular[2] = 0.4f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(25.5f, 14.0f, 0.0f);
	gluSphere(quadric[19], 1.0f, 30, 30); //draw sphere

										 //-------------------- 20- 2nd sphere on 4th column, cyan rubber ------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.05f;	//g
	MaterialAmbient[2] = 0.05f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.4f;	//r
	MaterialDiffuse[1] = 0.5f;	//g
	MaterialDiffuse[2] = 0.5f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.04f;	//r
	MaterialSpecular[1] = 0.7f;	//g
	MaterialSpecular[2] = 0.7f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(25.5f, 11.65f, 0.0f);
	gluSphere(quadric[20], 1.0f, 30, 30); //draw sphere

										 //-------------------- 21- 3rd sphere on 4th column, green rubber ------------------------------

	MaterialAmbient[0] = 0.0f;	//r
	MaterialAmbient[1] = 0.05f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.4f;	//r
	MaterialDiffuse[1] = 0.5f;	//g
	MaterialDiffuse[2] = 0.4f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.04f;	//r
	MaterialSpecular[1] = 0.7f;	//g
	MaterialSpecular[2] = 0.04f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(25.5f, 9.32f, 0.0f);
	gluSphere(quadric[21], 1.0f, 30, 30); //draw sphere

										 //-------------------- 22- 4th sphere on 4th column, red rubber------------------------------

	MaterialAmbient[0] = 0.05f;	//r
	MaterialAmbient[1] = 0.0f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.5f;	//r
	MaterialDiffuse[1] = 0.4f;	//g
	MaterialDiffuse[2] = 0.4f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.7f;	//r
	MaterialSpecular[1] = 0.04f;	//g
	MaterialSpecular[2] = 0.04f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(25.5f, 6.99f, 0.0f);
	gluSphere(quadric[22], 1.0f, 30, 30); //draw sphere

										 //-------------------- 23- 5th sphere on 4th column, white ------------------------------

	MaterialAmbient[0] = 0.05f;	//r
	MaterialAmbient[1] = 0.05f;	//g
	MaterialAmbient[2] = 0.05f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.5f;	//r
	MaterialDiffuse[1] = 0.5f;	//g
	MaterialDiffuse[2] = 0.5f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.7f;	//r
	MaterialSpecular[1] = 0.7f;	//g
	MaterialSpecular[2] = 0.7f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(25.5f, 4.66f, 0.0f);
	gluSphere(quadric[23], 1.0f, 30, 30); //draw sphere

										 //-------------------- 24- 6th sphere on 4th column, yellow rubber ------------------------------

	MaterialAmbient[0] = 0.05f;	//r
	MaterialAmbient[1] = 0.05f;	//g
	MaterialAmbient[2] = 0.0f;	//b
	MaterialAmbient[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.5f;	//r
	MaterialDiffuse[1] = 0.5f;	//g
	MaterialDiffuse[2] = 0.4f;	//b
	MaterialDiffuse[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.7f;	//r
	MaterialSpecular[1] = 0.7f;	//g
	MaterialSpecular[2] = 0.04f;	//b
	MaterialSpecular[3] = 1.0f;		//a

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;

	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);	//not necessary but discipline
	glLoadIdentity();

	glTranslatef(25.5f, 2.33f, 0.0f);
	gluSphere(quadric[24], 1.0f, 30, 30); //draw sphere*/

}

void display(void)
{
	//prototype
	void Draw24Spheres(void);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	
	if (keyPress == 1)
	{
		glRotatef(AngleOfXRotation, 1.0f, 0.0f, 0.0f);
		LightPosition[1] = AngleOfXRotation;
	}
	else if (keyPress == 2)
	{
		glRotatef(AngleOfYRotation, 0.0f, 1.0f, 0.0f);
		LightPosition[2] = AngleOfYRotation;
	}
	else if (keyPress == 3)
	{
		glRotatef(AngleOfZRotation, 0.0f, 0.0f, 1.0f);
		LightPosition[0] = AngleOfZRotation;
	}

	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
	Draw24Spheres();
	
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

	if (width <= height)
		glOrtho(0.0f, 15.5f, 0.0f, (15.5f*((GLfloat)height / (GLfloat)width)), -10.0f, 10.0f);//multiply to width - (left,right,bottom,top,near,far)
	else
		glOrtho(0.0f, (15.5f*((GLfloat)width / (GLfloat)height)), 0.0f, 15.5f, -10.0f, 10.0f);//multiply to height

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

