#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>  
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "vmath.h"
#import <math.h>

enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOODR_0
};
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef,const CVTimeStamp *, const CVTimeStamp *, CVOptionFlags, CVOptionFlags *, void *);
using namespace vmath;
FILE *gpFile = NULL;
#define PI 3.141592653589793238463
GLfloat angle_Tri = 0.0f;
GLfloat angle_Circle = 0.0f;
GLfloat angle_line = 0.0f;

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@interface GLView : NSOpenGLView
@end

//entry point function
int main(int argc, char *argv[])
{
    //code
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc]init];
    
    NSApp=[NSApplication sharedApplication];
    
    [NSApp setDelegate:[[AppDelegate alloc]init]];
    
    [NSApp run];
    
    [pool release];
    
    return(0);
}

//interface implementation
@implementation AppDelegate
{
@private
    NSWindow *window;
    GLView *glView;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    //code
    
    //log file
    NSBundle *mainBundle=[NSBundle mainBundle];
    NSString *appDirName=[mainBundle bundlePath];
    NSString *parentDirPath=[appDirName stringByDeletingLastPathComponent];
    NSString *logFileNameWithPath=[NSString stringWithFormat:@"%@/Log.txt", parentDirPath];
    const char *pszLogFileNameWithPath=[logFileNameWithPath cStringUsingEncoding:NSASCIIStringEncoding];
    
    gpFile=fopen(pszLogFileNameWithPath,"w");
    if(gpFile==NULL)
    {
        printf("Can Not Create A Log File.\nExitting...\n");
        [self release];
        [NSApp terminate:self];
    }
    fprintf(gpFile, "Log File Created Successfully.\n");
    
    //window
    NSRect win_rect;
    win_rect=NSMakeRect(0.0,0.0,800.0,600.0);
    
    //create simple window
    window=[[NSWindow alloc] initWithContentRect:win_rect
                                       styleMask:NSWindowStyleMaskTitled |
            NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                                         backing:NSBackingStoreBuffered
                                           defer:NO];
    [window setTitle:@"macOS Window:Blue window"];
    [window center];
    
    glView=[[GLView alloc]initWithFrame:win_rect];
    
    [window setContentView:glView];
    [window setDelegate:self];
    [window makeKeyAndOrderFront:self];
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    //code
    
    if(gpFile)
    {
        fclose(gpFile);
        gpFile=NULL;
    }
}

- (void)windowWillClose:(NSNotification *)notification
{
    //code
    [NSApp terminate:self];
}

- (void)dealloc
{
    //code
    [glView release];
    
    [window release];
    
    [super dealloc];
}
@end

@implementation GLView
{
    @private
    CVDisplayLinkRef displayLink;   //meansCVDisplayLink *displayLink;
    GLuint gVertexShaderObject;
    GLuint gFragmentShaderObject;
    GLuint gShaderProgramObject;

    GLuint vao_triangle;
    GLuint vbo_triangle_pos;
    GLuint vbo_triangle_col;

    GLuint vao_line;
    GLuint vbo_line_pos;
    GLuint vbo_line_col;

    GLuint vao_inner_circle;
    GLuint vbo_inner_circle_pos;
    GLuint vbo_inner_circle_col;

    GLuint mvpUniform;                  //mvp = ModelViewProjection
    mat4 perspectiveProjectionMatrix;   //mat4 is from vmath
}

- (id)initWithFrame:(NSRect)frame;
{
    //code
    self=[super initWithFrame:frame];
    
    if(self)
    {
        [[self window]setContentView:self];
        
        NSOpenGLPixelFormatAttribute attrs[]=
        {
            //must specify the 4.1 core version
            NSOpenGLPFAOpenGLProfile,
            NSOpenGLProfileVersion4_1Core,
            //specify the display ID to associates the GL context with(main display for now)
            NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFADepthSize, 24,
            NSOpenGLPFAAlphaSize, 8,
            NSOpenGLPFADoubleBuffer,
            0};//last 0 is must
        NSOpenGLPixelFormat *pixelFormat=[[[NSOpenGLPixelFormat alloc]initWithAttributes:attrs] autorelease];
        
        if(pixelFormat==nil)
        {
            fprintf(gpFile, "No Valid OpenGL Pixel Format Is Available.Exitting...");
            [self release];
            [NSApp terminate:self];
        }
        fprintf(gpFile, "pixelFormat is not nil\n");
        NSOpenGLContext *glContext=[[[NSOpenGLContext alloc]initWithFormat:pixelFormat shareContext:nil]autorelease];
        
        [self setPixelFormat:pixelFormat];
        
        [self setOpenGLContext:glContext];
    }
    return(self);
}

- (CVReturn)getFrameForTime:(const CVTimeStamp *)pOutputTime
{
    //code
    NSAutoreleasePool *pool=[[NSAutoreleasePool alloc]init];
    
    [self drawView];
    
    [pool release];
    return(kCVReturnSuccess);
}

-(void)initializeTriangle
{

    const GLfloat triangle_pos[] =
    {
        0.0f,0.5f,0.0f,
        -0.5f,-0.5f,0.0f,
        -0.5f,-0.5f,0.0f,
        0.5f,-0.5f,0.0f,
        0.5f,-0.5f,0.0f,
        0.0f,0.5f,0.0f,
    };

    const GLfloat triangle_col[] =
    {
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
    };


    //create vao and vbo
    glGenVertexArrays(1, &vao_triangle);
    glBindVertexArray(vao_triangle);

    //yellow
    glGenBuffers(1, &vbo_triangle_pos);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_pos), triangle_pos, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_triangle_col);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle_col);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_col), triangle_col, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

-(void) initializeLine
{
    GLfloat linepos[] = {
        0.0f, 0.5f, 0.0f,
        0.0f, -0.5f, 0.0f
    };

    GLfloat lineCol[] = {
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &vao_line);
    glBindVertexArray(vao_line);

    glGenBuffers(1, &vbo_line_pos);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_line_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(linepos), linepos, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_line_col);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_line_col);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineCol), lineCol, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

}

-(void) initializeInnerCircle
{
    GLfloat innerCircleVertex[10];
    GLfloat radius_inner = 0.308;

    //glVertex3f(radius*cos(angle) + x_coordinate_of_center, radius*sin(angle) + y_coordinate_of_center,0.0f);




    //Calculation done here and their ans

    /*radius of incircle = area of triangle/ semiperimeter  = A/S                                0.3088 = 0.4998 / 1.618

    area of triangle(A) = sqrt(S*(S-a)*(S-b)*(S-c))             //sqrt= square root of()         0.4988 = sqrt(1.618*(1.618-1.118)*(1.618-1.118)*(1.618-1))

    semiperimeter= a+b+c/2                // where a,b,c are sides of triangle                   1.618 = (1.118+1.118+1) /2

    To find lenght of triangle use distance formula -
    dist = sqrt((x2-x1)^2 + (y2-y1)^2 + (z2-z1)^2)                                               for side1 - 1.118 = sqrt((0.5-0.0)^2 + (-0.5-0.5)^2)

    center of triangle -
    x = (x1+x2+x3) / 3                                                                           x=0.0+(-0.5)+0.5 / 3
    y = (y1+y2+y3) / 3                                                                           y=0.5+(-0.5)+(-0.5) / 3
    */

    GLfloat innerCircleCol[] = {
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,

        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,

        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &vao_inner_circle);
    glBindVertexArray(vao_inner_circle);

    for (GLfloat angle = 0.0f; angle < (2.0f * PI); angle = angle + 0.01f)
    {
        innerCircleVertex[0] = ((cos(angle) * radius_inner));
        innerCircleVertex[1] = ((sin(angle) * radius_inner));
        innerCircleVertex[2] = 0.0f;
    }

    glGenBuffers(1, &vbo_inner_circle_pos);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_inner_circle_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(innerCircleVertex), innerCircleVertex, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_inner_circle_col);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_inner_circle_col);
    glBufferData(GL_ARRAY_BUFFER, sizeof(innerCircleCol), innerCircleCol, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

- (void)prepareOpenGL
{
    //variables
    GLint iShaderCompileStatus = 0;
    GLint iProgramLinkStatus = 0;
    GLint iInfoLogLength = 0;
    GLchar *szInfoLog = NULL;
    
    //code
    //OpenGL Info
    fprintf(gpFile, "OpenGL Version : %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    [[self openGLContext]makeCurrentContext];
    
    GLint swapInt=1;
    
    [[self openGLContext]setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    
    //VERTEX SHADER
    //define vertex shader object
    gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //write vertex shader code
    const GLchar *vertexShaderSourceCode =
       "#version 410 core" \
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
    glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);
    glCompileShader(gVertexShaderObject);

    //step - 1
    glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);

    //step - 2
    if (iShaderCompileStatus == GL_FALSE)
    {
        glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);

        if (iInfoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(iInfoLogLength);

            if (szInfoLog != NULL)
            {
                GLsizei Written;
                glGetShaderInfoLog(gVertexShaderObject,
                    iInfoLogLength,
                    &Written,
                    szInfoLog);

                fprintf(gpFile, "Vertex Shader Error : \n %s \n", szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }

    //FRAGMENT SHADER
    //define fragment shader object
    gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //write shader code
    const GLchar *fragmentShaderSourceCode =
        "#version 410" \
        "\n" \
        "out vec4 FragColor;" \
        "in vec4 out_color;" \
        "void main(void)" \
        "{" \
        "FragColor = out_color;" \
        "}";
    //specify above shader code to fragment shader object
    glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

    glCompileShader(gFragmentShaderObject);
    iShaderCompileStatus = 0;
    iInfoLogLength = 0;
    szInfoLog = NULL;

    //step - 1
    glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);

    //step - 2
    if (iShaderCompileStatus == GL_FALSE)
    {
        glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);

        if (iInfoLogLength > 0)
        {
            
            szInfoLog = (GLchar *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei Written;
                glGetShaderInfoLog(gFragmentShaderObject,
                    iInfoLogLength,
                    &Written,
                    szInfoLog);
                fprintf(gpFile, "Fragment Shader Error : \n %s \n", szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
                
            }
        }
    }

    //create shader program object
    gShaderProgramObject = glCreateProgram();

    //Attach Vertex Shader
    glAttachShader(gShaderProgramObject, gVertexShaderObject);

    //Attach Fragment Shader
    glAttachShader(gShaderProgramObject, gFragmentShaderObject);

    //pre linking bonding to vertex attributes
    glBindAttribLocation(gShaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");
    glBindAttribLocation(gShaderProgramObject, AMC_ATTRIBUTE_COLOR, "vColor");
    
    //link the shader porgram
    glLinkProgram(gShaderProgramObject);

    //error checking

    iInfoLogLength = 0;
    szInfoLog = NULL;

    //step - 1
    glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iProgramLinkStatus);
    
    //step - 2
    if (iProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);

        if (iInfoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(iInfoLogLength);

            if (szInfoLog != NULL)
            {
                GLsizei Written;
                glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &Written, szInfoLog);
                fprintf(gpFile, "Program Link Error : \n %s\n", szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
                
            }
        }
    }

    //post linking retriving uniform location
    mvpUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");

    [self initializeInnerCircle];
    [self initializeTriangle];
    [self initializeLine];

    //clear the window
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    //depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    perspectiveProjectionMatrix = mat4::identity();   
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
    CGLContextObj cglContext=(CGLContextObj)[[self openGLContext]CGLContextObj];
    CGLPixelFormatObj cglPixelFormat=(CGLPixelFormatObj)[[self pixelFormat]CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    fprintf(gpFile, "before CVDisplayLinkStart\n");
    CVDisplayLinkStart(displayLink);
}

- (void)reshape
{
    //code
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    
    NSRect rect=[self bounds];
    
    GLfloat width=rect.size.width;
    GLfloat height=rect.size.height;
    
    if(height==0)
        height=1;
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    
    perspectiveProjectionMatrix = perspective(45.0f, ((GLfloat)width / (GLfloat)height), 0.1f, 100.0f);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
}

- (void)drawRect:(NSRect)dirtyRect
{
    //code
    
    [self drawView];
}

- (void)drawView
{
    //code
    [[self openGLContext]makeCurrentContext];
    
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(gShaderProgramObject);

    mat4 modelViewMatrix;
    mat4 modelViewProjectionMatrix;
    mat4 ratationMatrix;

    modelViewMatrix = mat4::identity();
    modelViewProjectionMatrix = mat4::identity();
    ratationMatrix = mat4::identity();

    modelViewMatrix = translate(0.0f, 0.001f, -2.0f);

    ratationMatrix = rotate(angle_Tri, 0.0f, 1.0f, 0.0f);

    modelViewMatrix = modelViewMatrix * ratationMatrix;

    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

    glBindVertexArray(vao_triangle);

    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 2, 2);
    glDrawArrays(GL_LINES, 4, 2);

    glBindVertexArray(0);


    modelViewMatrix = mat4::identity();
    modelViewProjectionMatrix = mat4::identity();
    ratationMatrix = mat4::identity();

    modelViewMatrix = translate(0.0f, 0.0f, -2.0f);

    ratationMatrix = rotate(angle_line, 0.0f, 1.0f, 0.0f);

    modelViewMatrix = modelViewMatrix * ratationMatrix;

    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
    
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

    //bind with vao
    glBindVertexArray(vao_line);

    glDrawArrays(GL_LINES, 0, 2);

    //unbind vao
    glBindVertexArray(0);

    GLfloat innerCircleVertex[10];
    GLfloat radius_inner = 0.308;

    modelViewMatrix = mat4::identity();
    modelViewProjectionMatrix = mat4::identity();
    ratationMatrix = mat4::identity();

    modelViewMatrix = translate(0.0f, -0.19f, -2.0f);

    ratationMatrix = rotate(angle_Circle, 0.0f, 1.0f, 0.0f);

    modelViewMatrix = modelViewMatrix * ratationMatrix;

    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

    glBindVertexArray(vao_inner_circle);

    for (GLfloat angle = 0.0f; angle < (2.0f * PI); angle = angle + 0.01f)
    {
        innerCircleVertex[0] = ((cos(angle) * radius_inner));
        innerCircleVertex[1] = ((sin(angle) * radius_inner));
        innerCircleVertex[2] = 0.0f;

        glBindBuffer(GL_ARRAY_BUFFER, vbo_inner_circle_pos);
        glBufferData(GL_ARRAY_BUFFER, sizeof(innerCircleVertex), innerCircleVertex, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        //draw scene
        glPointSize(2.0f);
        glDrawArrays(GL_POINTS, 0, 1);
    }

    //unbind vao
    glBindVertexArray(0);

    //unuse program
    glUseProgram(0);
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);

       angle_Tri = angle_Tri + 1.0f;
        if (angle_Tri >= 360.0f)
            angle_Tri = angle_Tri - 360.0f;

        angle_Circle = angle_Circle + 1.0f;
        if (angle_Circle >= 360.0f)
            angle_Circle = angle_Circle - 360.0f;

        angle_line = angle_line + 1.0f;
        if (angle_line >= 360.0f)
            angle_line = angle_line - 360.0f;
}

- (BOOL)acceptsFirstResponder
{
    //code
    [[self window]makeFirstResponder:self];
    return(YES);
}

- (void)keyDown:(NSEvent *)theEvent
{
    //code
    int key=(int)[[theEvent characters]characterAtIndex:0];
    switch(key)
    {
        case 27: 
            [self release];
            [NSApp terminate:self];
            break;
            
        case 'F':
        case 'f':
            [[self window]toggleFullScreen:self];   
            break;

        default:
            break;
    }
}

- (void)mouseDown:(NSEvent *)theEvent
{
    //code    
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    //code
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
    //code   
}

- (void)dealloc
{
    //code
    if (vao_triangle)
    {
        glDeleteVertexArrays(1, &vao_triangle);
        vao_triangle = 0;
    }

    if (vbo_triangle_pos)
    {
        glDeleteVertexArrays(1, &vbo_triangle_pos);
        vbo_triangle_pos = 0;
    }

    if (vbo_triangle_col)
    {
        glDeleteVertexArrays(1, &vbo_triangle_col);
        vbo_triangle_col = 0;
    }

    if (vao_inner_circle)
    {
        glDeleteVertexArrays(1, &vao_inner_circle);
        vao_inner_circle = 0;
    }

    if (vbo_inner_circle_pos)
    {
        glDeleteVertexArrays(1, &vbo_inner_circle_pos);
        vbo_inner_circle_pos = 0;
    }

    if (vbo_inner_circle_col)
    {
        glDeleteVertexArrays(1, &vbo_inner_circle_col);
        vbo_inner_circle_col = 0;
    }

    if (vao_line)
    {
        glDeleteVertexArrays(1, &vao_line);
        vao_line = 0;
    }

    if (vbo_line_pos)
    {
        glDeleteVertexArrays(1, &vbo_line_pos);
        vbo_line_pos = 0;
    }

    if (vbo_line_col)
    {
        glDeleteVertexArrays(1, &vbo_line_col);
        vbo_line_col = 0;
    }

    //safe release
    
    if (gShaderProgramObject)
    {
        GLsizei shaderCount;
        GLsizei shaderNumber;

        glUseProgram(gShaderProgramObject);

        glGetProgramiv(gShaderProgramObject, GL_ATTACHED_SHADERS, &shaderCount);

        GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);

        if (pShaders)
        {
            glGetAttachedShaders(gShaderProgramObject, shaderCount, &shaderCount, pShaders);

            for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
            {
                glDetachShader(gShaderProgramObject, pShaders[shaderNumber]);
                glDeleteShader(pShaders[shaderNumber]);
                pShaders[shaderNumber] = 0;
            }
            free(pShaders);
        }
        glDeleteProgram(gShaderProgramObject);
        gShaderProgramObject = 0;
        glUseProgram(0);
    }
    [super dealloc];
}
@end

CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,const CVTimeStamp *pNow, const CVTimeStamp *pOutputTime, CVOptionFlags flagsIn, CVOptionFlags *pFlagsOut, void *pDisplayLinkContext)
{
    CVReturn result = [(GLView *)pDisplayLinkContext getFrameForTime:pOutputTime];
    return(result);
}
