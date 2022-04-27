#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "vmath.h"

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

struct Light
{
    GLfloat Ambient[4];
    GLfloat Diffuse[4];
    GLfloat Specular[4];
    GLfloat Position[4];
};

Light Lights[2];

float MaterialAmbient[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float MaterialDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float MaterialSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float MaterialShininess = 128.0f;

GLfloat angle_pyramid = 0.0f;

BOOL bLKeyPressed = NO;
BOOL bAnimate = NO;

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
    
    window=[[NSWindow alloc] initWithContentRect:win_rect
                                       styleMask:NSWindowStyleMaskTitled |
            NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                                         backing:NSBackingStoreBuffered
                                           defer:NO];
    [window setTitle:@"macOS Window:Two Lights On Rotating Pyramid"];
    [window center];
    
    glView=[[GLView alloc]initWithFrame:win_rect];
    
    [window setContentView:glView];
    [window setDelegate:self];
    [window makeKeyAndOrderFront:self];
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
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
    CVDisplayLinkRef displayLink;
    GLuint gVertexShaderObject;
    GLuint gFragmentShaderObject;
    GLuint gShaderProgramObject;

    GLuint vao_pyramid;
    GLuint vbo_pyramid_position;
    GLuint vbo_pyramid_normal;

    GLuint modelUniform;
    GLuint viewUniform;
    GLuint projectionUniform;

    GLuint LaUniform_red;
    GLuint LaUniform_blue;
    GLuint LdUniform_red;
    GLuint LdUniform_blue;
    GLuint LsUniform_red;
    GLuint LsUniform_blue;
    GLuint lightPositionUniform_red;
    GLuint lightPositionUniform_blue;

    GLuint KaUniform;
    GLuint KdUniform;
    GLuint KsUniform;
    GLuint shininessUniform;
    GLuint LKeyPressedUniform;
    
    mat4 perspectiveProjectionMatrix;}

- (id)initWithFrame:(NSRect)frame;
{
    //code
    self=[super initWithFrame:frame];
    
    if(self)
    {
        [[self window]setContentView:self];
        
        NSOpenGLPixelFormatAttribute attrs[]=
        {
            NSOpenGLPFAOpenGLProfile,
            NSOpenGLProfileVersion4_1Core,
            NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFADepthSize, 24,
            NSOpenGLPFAAlphaSize, 8,
            NSOpenGLPFADoubleBuffer,
            0};
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

- (void)prepareOpenGL
{
    //variables
    GLint iShaderCompileStatus = 0;
    GLint iProgramLinkStatus = 0;
    GLint iInfoLogLength = 0;
    GLchar *szInfoLog = NULL;
    
    fprintf(gpFile, "OpenGL Version : %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    [[self openGLContext]makeCurrentContext];
    
    GLint swapInt=1;
    
    [[self openGLContext]setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    
    Lights[0].Ambient[0] = 0.0f;
    Lights[0].Ambient[1] = 0.0f;
    Lights[0].Ambient[2] = 0.0f;
    Lights[0].Ambient[3] = 1.0f;

    Lights[0].Diffuse[0] = 1.0f;
    Lights[0].Diffuse[1] = 0.0f;
    Lights[0].Diffuse[2] = 0.0f;
    Lights[0].Diffuse[3] = 1.0f;

    Lights[0].Specular[0] = 1.0f;
    Lights[0].Specular[1] = 0.0f;
    Lights[0].Specular[2] = 0.0f;
    Lights[0].Specular[3] = 1.0f;

    Lights[0].Position[0] = -2.0f;
    Lights[0].Position[1] = 0.0f;
    Lights[0].Position[2] = 0.0f;
    Lights[0].Position[3] = 1.0f;

    Lights[1].Ambient[0] = 0.0f;
    Lights[1].Ambient[1] = 0.0f;
    Lights[1].Ambient[2] = 0.0f;
    Lights[1].Ambient[3] = 1.0f;

    Lights[1].Diffuse[0] = 0.0f;
    Lights[1].Diffuse[1] = 0.0f;
    Lights[1].Diffuse[2] = 1.0f;
    Lights[1].Diffuse[3] = 1.0f;

    Lights[1].Specular[0] = 0.0f;
    Lights[1].Specular[1] = 0.0f;
    Lights[1].Specular[2] = 1.0f;
    Lights[1].Specular[3] = 1.0f;

    Lights[1].Position[0] = 2.0f;
    Lights[1].Position[1] = 0.0f;
    Lights[1].Position[2] = 0.0f;
    Lights[1].Position[3] = 1.0f;
    
    //VERTEX SHADER
    //define vertex shader object
    gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //write vertex shader code
    const GLchar *vertexShaderSourceCode =
        "#version 410" \
        "\n" \
        "in vec4 vPosition;" \
        "in vec3 vNormal;" \
        "uniform mat4 u_model_matrix;" \
        "uniform mat4 u_view_matrix;" \
        "uniform mat4 u_projection_matrix;" \
        "uniform int u_LKeyPressed;" \
        "uniform vec3 u_La_red;" \
        "uniform vec3 u_La_blue;" \
        "uniform vec3 u_Ld_red;" \
        "uniform vec3 u_Ld_blue;" \
        "uniform vec3 u_Ls_red;" \
        "uniform vec3 u_Ls_blue;" \
        "uniform vec4 u_light_position_red;" \
        "uniform vec4 u_light_position_blue;" \
        "uniform vec3 u_Ka;" \
        "uniform vec3 u_Kd;" \
        "uniform vec3 u_Ks;" \
        "uniform float shininess;" \
        "out vec3 phong_ads_light;" \
        "void main(void)" \
        "{" \
            "if (u_LKeyPressed == 1)" \
            "{" \
                "vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;" \
                "mat3 normal_matrix = mat3(transpose(inverse(u_view_matrix * u_model_matrix)));" \
                "vec3 transformed_normal = normalize(normal_matrix * vNormal);" \
                "vec3 light_direction_red = normalize(vec3(u_light_position_red - eye_coordinates));" \
                "vec3 light_direction_blue = normalize(vec3(u_light_position_blue - eye_coordinates));" \
                "float tn_dot_LightDirection_red = max(dot(light_direction_red, transformed_normal), 0.0);" \
                "float tn_dot_LightDirection_blue = max(dot(light_direction_blue, transformed_normal), 0.0);" \
                "vec3 reflection_vector_red = reflect(-light_direction_red, transformed_normal);" \
                "vec3 reflection_vector_blue = reflect(-light_direction_blue, transformed_normal);" \
                "vec3 viewer_vector = normalize(vec3(-eye_coordinates.xyz));" \

                "vec3 ambient = (u_La_red * u_Ka) + (u_La_blue * u_Ka);" \
                "vec3 diffuse = (u_Ld_red * u_Kd * tn_dot_LightDirection_red) + ( u_Ld_blue * u_Kd * tn_dot_LightDirection_blue);" \
                "vec3 specular = (u_Ls_red * u_Ks * pow(max(dot(reflection_vector_red, viewer_vector), 0.0), shininess)) + (u_Ls_blue * u_Ks * pow(max(dot(reflection_vector_blue, viewer_vector), 0.0), shininess));" \

                "phong_ads_light = ambient + diffuse + specular;" \
            "}" \
            "else" \
            "{" \
                "phong_ads_light = vec3(1.0, 1.0, 1.0);" \
            "}" \
            "gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
        "}";

    //specify above source code to vertex shader object
    glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

    //compile the vertex shader
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
        "in vec3 phong_ads_light;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
            "FragColor = vec4(phong_ads_light, 0.0);" \
        "}";
    //specify above shader code to fragment shader object
    glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);
    
    //compile the shader
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
    glBindAttribLocation(gShaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");
    
    //link the shader porgram
    glLinkProgram(gShaderProgramObject);

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
    modelUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
    viewUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
    projectionUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
    LKeyPressedUniform = glGetUniformLocation(gShaderProgramObject, "u_LKeyPressed");

    LaUniform_red = glGetUniformLocation(gShaderProgramObject, "u_La_red");
    LaUniform_blue = glGetUniformLocation(gShaderProgramObject, "u_La_blue");
    LdUniform_red = glGetUniformLocation(gShaderProgramObject, "u_Ld_red");
    LdUniform_blue = glGetUniformLocation(gShaderProgramObject, "u_Ld_blue");
    LsUniform_red = glGetUniformLocation(gShaderProgramObject, "u_Ls_red");
    LsUniform_blue = glGetUniformLocation(gShaderProgramObject, "u_Ls_blue");

    KaUniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");
    KdUniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
    KsUniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");
    shininessUniform = glGetUniformLocation(gShaderProgramObject, "shininess");

    lightPositionUniform_red = glGetUniformLocation(gShaderProgramObject, "u_light_position_red");
    lightPositionUniform_blue = glGetUniformLocation(gShaderProgramObject, "u_light_position_blue");

    //triangle vertices declaration
    const GLfloat pyramidVertices[] =
    {
        0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,

        0.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,

        0.0f, 1.0f, 0.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,

        0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f
    };
    const GLfloat pyramidNormals[] =
    {
        0.0f, 0.447214f, 0.894427f,
        0.0f, 0.447214f, 0.894427f,
        0.0f, 0.447214f, 0.894427f,

        0.89427f, 0.447214f, 0.0f,
        0.89427f, 0.447214f, 0.0f,
        0.89427f, 0.447214f, 0.0f,

        0.0f, 0.447214f, -0.894427f,
        0.0f, 0.447214f, -0.894427f,
        0.0f, 0.447214f, -0.894427f,

        -0.89427f, 0.447214f, 0.0f,
        -0.89427f, 0.447214f, 0.0f,
        -0.89427f, 0.447214f, 0.0f

    };


    //create vao and vbo
    //pyramid

    glGenVertexArrays(1, &vao_pyramid);
    
    glBindVertexArray(vao_pyramid);
    
    //position
    
    glGenBuffers(1, &vbo_pyramid_position);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_position);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //colour
    
    glGenBuffers(1, &vbo_pyramid_normal);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_normal);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNormals), pyramidNormals, GL_STATIC_DRAW);
    
    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    
    //depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    
    glDepthFunc(GL_LEQUAL);

    perspectiveProjectionMatrix = mat4::identity();
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
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

    //declaration of metrices
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 translationMatrix;
    mat4 rotationMatrix;
    //init above metrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();
    translationMatrix = mat4::identity();
    rotationMatrix = mat4::identity();
    //do necessary transformations here
    translationMatrix = translate(0.0f, 0.0f, -4.0f);
    rotationMatrix = rotate(angle_pyramid, 0.0f, 1.0f, 0.0f);
    //do necessary matrix multiplication
    modelMatrix = modelMatrix * translationMatrix;
    modelMatrix = modelMatrix * rotationMatrix;
    projectionMatrix *= perspectiveProjectionMatrix;
    //send necessary matrics to shaders in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //if light is enabled
    if (bLKeyPressed == YES)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform3fv(LaUniform_red, 1, Lights[0].Ambient);
        glUniform3fv(LdUniform_red, 1, Lights[0].Diffuse);
        glUniform3fv(LsUniform_red, 1, Lights[0].Specular);
        glUniform3fv(LaUniform_blue, 1, Lights[1].Ambient);
        glUniform3fv(LdUniform_blue, 1, Lights[1].Diffuse);
        glUniform3fv(LsUniform_blue, 1, Lights[1].Specular);
        glUniform3fv(KaUniform, 1, MaterialAmbient);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);
        glUniform3fv(KsUniform, 1, MaterialSpecular);
        glUniform1f(shininessUniform, MaterialShininess);
        glUniform4fv(lightPositionUniform_red, 1, Lights[0].Position);
        glUniform4fv(lightPositionUniform_blue, 1, Lights[1].Position);
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //bind with vao
    glBindVertexArray(vao_pyramid);

    //draw scene
    glDrawArrays(GL_TRIANGLES, 0, 12);

    //unbind vao
    glBindVertexArray(0);

    //unuse program
    glUseProgram(0);
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    
    if (bAnimate == YES)
    {
        angle_pyramid = angle_pyramid + 1.0f;
        if (angle_pyramid >= 360.0f)
        {
            angle_pyramid = 0.0f;
        }
    }
    
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

        case 'L':
        case 'l':
            if (bLKeyPressed == NO)
            {
                bLKeyPressed = YES;
            }
            else
            {
                bLKeyPressed = NO;
            }
            break;

        case 'A':
        case 'a':
            if (bAnimate == NO)
            {
                bAnimate = YES;
            }
            else
            {
                bAnimate = NO;
            }
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
    if (vbo_pyramid_position)
    {
        glDeleteBuffers(1, &vbo_pyramid_position);
        vbo_pyramid_position = 0;
    }
    if (vbo_pyramid_normal)
    {
        glDeleteBuffers(1, &vbo_pyramid_normal);
        vbo_pyramid_normal = 0;
    }
    if (vao_pyramid)
    {
        glDeleteVertexArrays(1, &vao_pyramid);
        vao_pyramid = 0;
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
