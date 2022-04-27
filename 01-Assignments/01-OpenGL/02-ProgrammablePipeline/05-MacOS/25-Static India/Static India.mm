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

    GLuint vao_first_I;
GLuint vbo_first_I_pos;
GLuint vbo_first_I_col;

GLuint vao_N;
GLuint vbo_N_pos;
GLuint vbo_N_col;

GLuint vao_D;
GLuint vbo_D_pos;
GLuint vbo_D_col;

GLuint vao_last_I;
GLuint vbo_last_I_pos;
GLuint vbo_last_I_col;

GLuint vao_A;
GLuint vbo_A_pos;
GLuint vbo_A_col;

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

-(void) initializeFirstI
{

	const GLfloat first_I_pos[] =
	{
		-0.56f, 0.5f, 0.0f,
		-0.56f, -0.5f, 0.0f,
		-0.65f, 0.5f, 0.0f,
		-0.47f, 0.5f, 0.0f,
		-0.65f, -0.5f, 0.0f,
		-0.47f, -0.5f, 0.0f,
	};

	const GLfloat first_I_col[] =
	{
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		0.0705882353f, 0.53333f, 0.0274509804f,

	};


	//create vao and vbo
	glGenVertexArrays(1, &vao_first_I);
	glBindVertexArray(vao_first_I);

	//yellow
	glGenBuffers(1, &vbo_first_I_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_first_I_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(first_I_pos), first_I_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_first_I_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_first_I_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(first_I_col), first_I_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

-(void) initializeN
{
	GLfloat lineNpos[] = {
		-0.37f, 0.5f, 0.0f,
		-0.37f, -0.5f, 0.0f,
		-0.19f, 0.5f, 0.0f,
		-0.19f, -0.5f, 0.0f,
		-0.37f, 0.5f, 0.0f,
		-0.19f, -0.5f, 0.0f,
	};

	GLfloat lineNCol[] = {
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
	};

	glGenVertexArrays(1, &vao_N);
	glBindVertexArray(vao_N);

	glGenBuffers(1, &vbo_N_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_N_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineNpos), lineNpos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_N_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_N_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineNCol), lineNCol, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

}

-(void) initializeD
{
	GLfloat Dpos[] = {
		-0.09f, 0.5f, 0.0f,
		-0.09f, -0.5f, 0.0f,
		0.09f, 0.4f, 0.0f,
		0.09f, -0.4f, 0.0f,
		-0.09f, 0.5f, 0.0f,
		0.04f, 0.5f, 0.0f,
		-0.09f, -0.5f, 0.0f,
		0.04f, -0.5f, 0.0f,
		0.04f, 0.5f, 0.0f,
		0.09f, 0.4f, 0.0f,
		0.04f, -0.5f, 0.0f,
		0.09f, -0.4f, 0.0f,
	};

	GLfloat DCol[] = {
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		0.0705882353f, 0.53333f, 0.0274509804f,
	};

	glGenVertexArrays(1, &vao_D);
	glBindVertexArray(vao_D);

	glGenBuffers(1, &vbo_D_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_D_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Dpos), Dpos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_D_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_D_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(DCol), DCol, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

-(void) initializeSecondI
{

	const GLfloat second_I_pos[] =
	{
		0.28f, 0.5f, 0.0f,
		0.28f, -0.5f, 0.0f,
		0.19f, 0.5f, 0.0f,
		0.37f, 0.5f, 0.0f,
		0.19f, -0.5f, 0.0f,
		0.37f, -0.5f, 0.0f,
	};

	const GLfloat second_I_col[] =
	{
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		0.0705882353f, 0.53333f, 0.0274509804f,

	};


	//create vao and vbo
	glGenVertexArrays(1, &vao_last_I);
	glBindVertexArray(vao_last_I);

	//yellow
	glGenBuffers(1, &vbo_last_I_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_last_I_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(second_I_pos), second_I_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_last_I_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_last_I_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(second_I_col), second_I_col, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

-(void) initializeA
{

	const GLfloat A_pos[] =
	{
		0.56f, 0.5f, 0.0f,
		0.47f, -0.5f, 0.0f,
		0.56f, 0.5f, 0.0f,
		0.65f, -0.5f, 0.0f,
		0.52f, 0.0f, 0.0f,
		0.60f, 0.0f, 0.0f,
		0.525f, 0.05f, 0.0f,
		0.595f, 0.05f, 0.0f,
		0.51f, -0.05f, 0.0f,
		0.61f, -0.05f, 0.0f,
	};

	const GLfloat A_col[] =
	{
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 0.6f, 0.2f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		0.0705882353f, 0.53333f, 0.0274509804f,
	};


	//create vao and vbo
	glGenVertexArrays(1, &vao_A);
	glBindVertexArray(vao_A);

	//yellow
	glGenBuffers(1, &vbo_A_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_A_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(A_pos), A_pos, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_A_col);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_A_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(A_col), A_col, GL_STATIC_DRAW);
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

    [self initializeD];
    [self initializeFirstI];
    [self initializeN];
    [self initializeSecondI];
    [self initializeA];

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

   // OpenGL Drawing
	//declaration of matrices
	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;

	//-------------------------------------------------------------------------------------------------------------------

	//for first I

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(0.0f, 0.0f, -2.0f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glLineWidth(9.0f);

	//bind with vao
	glBindVertexArray(vao_first_I);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//------------------------------------------------------------------------------------------------------------------

	//for line N

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(0.0f, 0.0f, -2.0f);

	//do necessary matrix multiplication

	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glLineWidth(9.0f);

	//bind with vao
	glBindVertexArray(vao_N);

	//draw scene
	//glLineWidth(2.0f);

	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//------------------------------------------------------------------------------------------------------------------

	//for D letter

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(0.0f, 0.0f, -2.0f);

	//do necessary matrix multiplication

	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glBindVertexArray(vao_D);

	glLineWidth(9.0f);

	//draw scene

	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);
	glDrawArrays(GL_LINES, 6, 2);
	glDrawArrays(GL_LINES, 8, 2);
	glDrawArrays(GL_LINES, 10, 2);

	//unbind vao
	glBindVertexArray(0);

	//--------------------------------------------------------------------------------------------------------------------
	

	//for last I

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(0.0f, 0.0f, -2.0f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glLineWidth(9.0f);

	//bind with vao
	glBindVertexArray(vao_last_I);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	//unbind vao
	glBindVertexArray(0);
	//-------------------------------------------------------------------------------------------------------------------

	//for A

	//initialize above matrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = translate(0.0f, 0.0f, -2.0f);

	//do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	glLineWidth(9.0f);

	//bind with vao
	glBindVertexArray(vao_A);

	//draw scene
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);
	glDrawArrays(GL_LINES, 6, 2);
	glDrawArrays(GL_LINES, 8, 2);

	//unbind vao
	glBindVertexArray(0);
	//-------------------------------------------------------------------------------------------------------------------
    //unuse program
    glUseProgram(0);
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
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
