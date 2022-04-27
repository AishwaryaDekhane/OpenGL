#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include <SOIL/SOIL.h>

//namespace
using namespace std;

//global
bool bFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColormap;
Window gWindow;

int giWindowWidth = 800;
int giWindowHeight = 600;

GLuint texture_smiley;

GLXContext gGlxContext;
bool bLightStatus = false;

int main()
{
    void initialize(void);
    void uninitialize(void);
    void createWindow(void);
    void resize(int, int);
    void toggleFullScreen(void);
    void display();

    bool bDone = false;

    static int winWidth = giWindowWidth;
    static int winHeight = giWindowHeight;

    char keys[26];
    createWindow();
    initialize();

    XEvent event;
    KeySym keysym;

    while (bDone == false)
    {
        while (XPending(gpDisplay))
        {
            XNextEvent(gpDisplay, &event);
            switch (event.type)
            {
            case MapNotify:
                break;
            case KeyPress:
                keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
                switch (keysym)
                {
                case XK_Escape:
                    bDone = true;
                    break;
                }
                XLookupString(&event.xkey, keys, sizeof(keys), NULL, NULL);
                switch (keys[0])
                {
                case 'f':
                case 'F':
                    toggleFullScreen();
                    bFullScreen = !bFullScreen;
                    break;
               
                }
                break;

            case ButtonPress:
                switch (event.xbutton.button)

                {
                case 1:
                    break;
                case 2:
                    break;
                case 3:
                    break;
                }

                break;
            case MotionNotify:
                break;
            case ConfigureNotify:
                winWidth = event.xconfigure.width;
                winHeight = event.xconfigure.height;
                printf("Window width : %d height : %d \n",winWidth, winHeight);
                resize(winWidth, winHeight);
                break;
            case Expose:
                break;
            case 33:
            
                bDone = true;
                break;
            }
        }
        display();
    }

    uninitialize();
    return (0);
}

void createWindow()
{
    void uninitialize();
    XSetWindowAttributes winAttribs;
    int defaultScreen;
    int defaultDepth;
    int styleMask;

    static int frameBufferAttributes[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER, True,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        None};

    gpDisplay = XOpenDisplay(NULL);
    if (gpDisplay == NULL)
    {
        printf("ERROR: Unable to open window.\n");
        uninitialize();
        exit(1);
    }

    defaultScreen = XDefaultScreen(gpDisplay);
    defaultDepth = DefaultDepth(gpDisplay, defaultScreen);
    gpXVisualInfo = glXChooseVisual(gpDisplay, defaultScreen, frameBufferAttributes);

    winAttribs.border_pixel = 0;
    winAttribs.background_pixel = 0;
    winAttribs.colormap = XCreateColormap(gpDisplay,

                                          RootWindow(gpDisplay, gpXVisualInfo->screen),
                                          gpXVisualInfo->visual,
                                          AllocNone);
    gColormap = winAttribs.colormap;

    winAttribs.background_pixmap = 0;
    winAttribs.border_pixmap = 0;
    winAttribs.event_mask = ExposureMask | VisibilityChangeMask | StructureNotifyMask |
                            ButtonPressMask | KeyPressMask | PointerMotionMask;

    styleMask = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;

    gWindow = XCreateWindow(gpDisplay,
                            RootWindow(gpDisplay, gpXVisualInfo->screen),
                            0, 0,
                            giWindowWidth,
                            giWindowHeight,
                            0,
                            gpXVisualInfo->depth,
                            InputOutput,
                            gpXVisualInfo->visual,
                            styleMask,
                            &winAttribs);
    if (!gWindow)
    {
        printf("ERROR: gWindow.\n");
        uninitialize();
        exit(1);
    }

    XStoreName(gpDisplay, gWindow, "My Window");
    Atom windowManagerDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(gpDisplay, gWindow, &windowManagerDelete, 1);

    XMapWindow(gpDisplay, gWindow);
}

void toggleFullScreen(void)
{
    Atom wm_state ;
    Atom full_screen;
    XEvent xev= {0};

    wm_state = XInternAtom(gpDisplay,"_NET_WM_STATE",False);
    memset(&xev, 0, sizeof(xev));

    xev.type  = ClientMessage;
    xev.xclient.window = gWindow;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = bFullScreen ? 0: 1;

    full_screen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN",False);
    xev.xclient.data.l[1] = full_screen;

    XSendEvent(gpDisplay,
                RootWindow(gpDisplay, gpXVisualInfo->screen),
                false,
                StructureNotifyMask,
                &xev);
}

bool LoadTexture(GLuint *texture, const char *path)
{
    bool bResult;
    int imageWidth, imageHeight;
    unsigned char *imageData = NULL;
    imageData = SOIL_load_image(path, &imageWidth, &imageHeight, 0, SOIL_LOAD_RGBA);

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

    //glPixelStorei()
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_2D,
                    GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D,
                      3,
                      imageWidth,
                      imageHeight,
                      GL_RGBA,
                      GL_UNSIGNED_BYTE,
                      imageData);

    SOIL_free_image_data(imageData);
    return bResult;
}


void initialize(void)
{
    void resize(int, int);
    bool LoadTexture(GLuint * texture, const char *path);

    gGlxContext = glXCreateContext(gpDisplay, gpXVisualInfo, NULL, GL_TRUE);
    glXMakeCurrent(gpDisplay, gWindow, gGlxContext);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_TEXTURE_2D);

    LoadTexture(&texture_smiley, "Smiley.bmp");
    resize(giWindowWidth, giWindowHeight);
}

void resize(int width, int height)
{
    if (height == 0)
    {
        height = 1;
    }
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f,
                   (GLfloat)width / (GLfloat)height,
                   0.1f,
                   100.0f);
}


void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -3.0f);

    glBindTexture(GL_TEXTURE_2D, texture_smiley);
    glBegin(GL_QUADS);

   
        //For A vertext
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 0.0f);
        //for B vertext
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);

        // For C Vertext
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 0.0f);

        // For D Vertext
        glTexCoord2f(01.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 0.0f);
      

    glEnd();

    glXSwapBuffers(gpDisplay, gWindow);
}

void uninitialize(void)
{
    GLXContext currentContext;
    currentContext = glXGetCurrentContext();

    if (currentContext != NULL && currentContext == gGlxContext)
    {
        glXMakeCurrent(gpDisplay, 0, 0);
    }
    if (gGlxContext)
    {
        glXDestroyContext(gpDisplay, gGlxContext);
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
}
