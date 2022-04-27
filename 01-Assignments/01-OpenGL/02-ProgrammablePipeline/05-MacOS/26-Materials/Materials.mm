#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "vmath.h"
#import "sphere.h"

#define RADIUS 100.0f

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

GLuint modelUniform;
GLuint viewUniform;
GLuint projectionUniform;

GLuint LaUniform;
GLuint LdUniform;
GLuint LsUniform;
GLuint lightPositionUniform;

GLuint KaUniform;
GLuint KdUniform;
GLuint KsUniform;
GLuint shininessUniform;
GLuint LKeyPressedUniform;

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint vao_sphere;
GLuint vbo_sphere_position;
GLuint vbo_sphere_normal;
GLuint vbo_sphere_element;

mat4 perspectiveProjectionMatrix;

//spher related variables
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_texture[764];
short sphere_elements[2280];
unsigned int gNumVertices;
unsigned int gNumElements;

float LightAmbient[] = { 0.0f,0.0f,0.0f,0.0f };
float LightDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
float LightSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
float LightPosition[] = { 100.0f,100.0f,100.0f,1.0f };

float MaterialAmbient[] = { 0.0f,0.0f,0.0f,0.0f };
float MaterialDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
float MaterialSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
float MaterialShininess[] = { 128.0f };

//flags
BOOL bLKeyPressed = FALSE;
int iCount = 0;
float lightAngle = 0.0f;

int giWindowWidth = 1280;
int giWindowHeight = 720;

//interface declaration
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
    
    //create simple window
    window=[[NSWindow alloc] initWithContentRect:win_rect
                                       styleMask:NSWindowStyleMaskTitled |
            NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                                         backing:NSBackingStoreBuffered
                                           defer:NO];
    [window setTitle:@"macOS Window:Materials"];
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
        "uniform mat4 u_mvp_matrix;" \
        "uniform int u_LKeyPressed;" \
        "uniform vec4 u_light_position;" \
        "out vec3 t_norm;" \
        "out vec3 light_direction;" \
        "out vec3 viewer_vector;" \
        "void main(void)" \
        "{" \
            "if (u_LKeyPressed == 1)" \
            "{" \
                "vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;" \
                "mat3 normal_matrix = mat3(transpose(inverse(u_view_matrix * u_model_matrix)));" \
                "t_norm = normal_matrix * vNormal;" \
                "light_direction = vec3(u_light_position - eye_coordinates);" \
                "viewer_vector = vec3(-eye_coordinates);" \
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
        "in vec3 t_norm;" \
        "in vec3 light_direction;" \
        "in vec3 viewer_vector;" \
        "uniform int u_LKeyPressed;" \
        "uniform vec3 u_La;" \
        "uniform vec3 u_Ld;" \
        "uniform vec3 u_Ls;" \
        "uniform vec4 u_light_position;" \
        "uniform vec3 u_Ka;" \
        "uniform vec3 u_Kd;" \
        "uniform vec3 u_Ks;" \
        "uniform float shininess;" \
        "out vec3 phong_ads_light;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
            "if(u_LKeyPressed == 1)" \
            "{" \
                "vec3 normalised_transformed_normal = normalize(t_norm);" \
                "vec3 normalised_light_direction = normalize(light_direction);" \
                "vec3 normalised_viewer_vector = normalize(viewer_vector);" \
                "vec3 reflection_vector = reflect(-normalised_light_direction, normalised_transformed_normal);" \
                "float tn_dot_LightDirection = max(dot(normalised_light_direction, normalised_transformed_normal), 0.0);" \
                "vec3 ambient = u_La * u_Ka;" \
                "vec3 diffuse = u_Ld * u_Kd * tn_dot_LightDirection;" \
                "vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalised_viewer_vector), 0.0), shininess);" \
                "phong_ads_light = ambient + diffuse + specular;" \
            "}" \
            "else" \
            "{" \
                "phong_ads_light = vec3(1.0, 1.0, 1.0);" \
            "}" \
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
    LaUniform = glGetUniformLocation(gShaderProgramObject, "u_La");
    LdUniform = glGetUniformLocation(gShaderProgramObject, "u_Ld");
    LsUniform = glGetUniformLocation(gShaderProgramObject, "u_Ls");
    KaUniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");
    KdUniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
    KsUniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");
    shininessUniform = glGetUniformLocation(gShaderProgramObject, "shininess");
    lightPositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light_position");

    //sphere vertices
    getSphereVertexData(sphere_vertices, sphere_normals, sphere_texture, sphere_elements);
    
    gNumVertices = getNumberOfSphereVertices();
    
    gNumElements = getNumberOfSphereElements();
    
    glGenVertexArrays(1, &vao_sphere);
    
    glBindVertexArray(vao_sphere);
    
    //position
    
    glGenBuffers(1, &vbo_sphere_position);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_position);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //normal
    
    glGenBuffers(1, &vbo_sphere_normal);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_normal);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);
    
    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //elements
    
    glGenBuffers(1, &vbo_sphere_element);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    //depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    
    glDepthFunc(GL_LEQUAL);

    perspectiveProjectionMatrix = mat4::identity();
    
    //material call
   // [self oglInitMaterial];
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
    CGLContextObj cglContext=(CGLContextObj)[[self openGLContext]CGLContextObj];
    CGLPixelFormatObj cglPixelFormat=(CGLPixelFormatObj)[[self pixelFormat]CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    fprintf(gpFile, "before CVDisplayLinkStart\n");
    CVDisplayLinkStart(displayLink);
}
void Draw24Spheres(void)
{
    void resize(int, int);


    GLfloat radius = 100.0f;
    // OpenGL Drawing
    //declaration of matrices
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;

    //for rotation
    mat4 ratationMatrix;
    mat4 translationMatrix;

    //-------------------- 1st sphere on 1st column, emerald ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.0215f;    //r
    MaterialAmbient[1] = 0.1745f;    //g
    MaterialAmbient[2] = 0.0215f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.07568f;    //r
    MaterialDiffuse[1] = 0.61424f;    //g
    MaterialDiffuse[2] = 0.07568f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.633f;    //r
    MaterialSpecular[1] = 0.727811f;    //g
    MaterialSpecular[2] = 0.633f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.6f * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);


        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(-3.5f, 2.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 2nd sphere on 1st column, jade ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.135f;    //r
    MaterialAmbient[1] = 0.2225f;    //g
    MaterialAmbient[2] = 0.1575f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.54f;    //r
    MaterialDiffuse[1] = 0.89f;    //g
    MaterialDiffuse[2] = 0.63f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.316228f;    //r
    MaterialSpecular[1] = 0.316228f;    //g
    MaterialSpecular[2] = 0.316228f; //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.1f * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);


        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(-3.5f, 1.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 3rd sphere on 1st column, obsidian ------------------------------
    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.05375f;    //r
    MaterialAmbient[1] = 0.05f;    //g
    MaterialAmbient[2] = 0.06625f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.18275f;    //r
    MaterialDiffuse[1] = 0.17f;    //g
    MaterialDiffuse[2] = 0.22525f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.332741f;    //r
    MaterialSpecular[1] = 0.328634f;    //g
    MaterialSpecular[2] = 0.346435f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.3f * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(-3.5f, 0.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 4th sphere on 1st column, pearl ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.25f;    //r
    MaterialAmbient[1] = 0.20725f;    //g
    MaterialAmbient[2] = 0.20725f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 1.0f;    //r
    MaterialDiffuse[1] = 0.829f;    //g
    MaterialDiffuse[2] = 0.829f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.296648f;    //r
    MaterialSpecular[1] = 0.296648f;    //g
    MaterialSpecular[2] = 0.296648f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.088f * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(-3.5f, -0.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 5th sphere on 1st column, ruby ------------------------------
    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.1745f;    //r
    MaterialAmbient[1] = 0.01175f;    //g
    MaterialAmbient[2] = 0.01175f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.61424f;    //r
    MaterialDiffuse[1] = 0.04136f;    //g
    MaterialDiffuse[2] = 0.04136f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.727811f;    //r
    MaterialSpecular[1] = 0.626959f;    //g
    MaterialSpecular[2] = 0.626959f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.6f * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(-3.5f, -1.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 6th sphere on 1st column, turquoise ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.1f;    //r
    MaterialAmbient[1] = 0.18725f;    //g
    MaterialAmbient[2] = 0.1745f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.396f;    //r
    MaterialDiffuse[1] = 0.74151f;    //g
    MaterialDiffuse[2] = 0.69102f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.297254f;    //r
    MaterialSpecular[1] = 0.30829f;    //g
    MaterialSpecular[2] = 0.306678f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.1f * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(-3.5f, -2.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 7- 1st sphere on 2nd column, brass ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.329412f;    //r
    MaterialAmbient[1] = 0.223529f;    //g
    MaterialAmbient[2] = 0.027451f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.780392f;    //r
    MaterialDiffuse[1] = 0.568627f;    //g
    MaterialDiffuse[2] = 0.113725f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.992157f;    //r
    MaterialSpecular[1] = 0.941176f;    //g
    MaterialSpecular[2] = 0.807843f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.21794872f * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(-1.3f, 2.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 8- 2nd sphere on 2nd column, bronze ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.2125f;    //r
    MaterialAmbient[1] = 0.1275f;    //g
    MaterialAmbient[2] = 0.054f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.714f;    //r
    MaterialDiffuse[1] = 0.4284f;    //g
    MaterialDiffuse[2] = 0.18144f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.393548f;    //r
    MaterialSpecular[1] = 0.271906f;    //g
    MaterialSpecular[2] = 0.166721f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.2f * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(-1.3f, 1.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 9- 3rd sphere on 2nd column, chrome -----------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.25f;    //r
    MaterialAmbient[1] = 0.25f;    //g
    MaterialAmbient[2] = 0.25f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.4f;    //r
    MaterialDiffuse[1] = 0.4f;    //g
    MaterialDiffuse[2] = 0.4f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.774597f;    //r
    MaterialSpecular[1] = 0.774597f;    //g
    MaterialSpecular[2] = 0.774597f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.6f * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(-1.3f, 0.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 10- 4th sphere on 2nd column, copper -----------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.19125f;    //r
    MaterialAmbient[1] = 0.0735f;    //g
    MaterialAmbient[2] = 0.0225f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.7038f;    //r
    MaterialDiffuse[1] = 0.27048f;    //g
    MaterialDiffuse[2] = 0.0828f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.256777f;    //r
    MaterialSpecular[1] = 0.137622f;    //g
    MaterialSpecular[2] = 0.086014f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.1f * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(-1.3f, -0.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 11- 5th sphere on 2nd column, gold ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.24725f;    //r
    MaterialAmbient[1] = 0.1995f;    //g
    MaterialAmbient[2] = 0.0745f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.75164f;    //r
    MaterialDiffuse[1] = 0.60648f;    //g
    MaterialDiffuse[2] = 0.22648f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.628281f;    //r
    MaterialSpecular[1] = 0.555802f;    //g
    MaterialSpecular[2] = 0.366065f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.4f * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(-1.3f, -1.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 12- 6th sphere on 2nd column, silver ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.19225f;    //r
    MaterialAmbient[1] = 0.19225f;    //g
    MaterialAmbient[2] = 0.19225f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.50754f;    //r
    MaterialDiffuse[1] = 0.50754f;    //g
    MaterialDiffuse[2] = 0.50754f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.508273f;    //r
    MaterialSpecular[1] = 0.508273f;    //g
    MaterialSpecular[2] = 0.508273f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.4f * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(-1.3f, -2.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 13- 1st sphere on 3rd column, black plastic ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.0f;    //r
    MaterialAmbient[1] = 0.0f;    //g
    MaterialAmbient[2] = 0.0f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.01f;    //r
    MaterialDiffuse[1] = 0.01f;    //g
    MaterialDiffuse[2] = 0.01f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.50f;    //r
    MaterialSpecular[1] = 0.50f;    //g
    MaterialSpecular[2] = 0.50f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.25 * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(1.3f, 2.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 14- 2nd sphere on 3rd column, cyan plastic ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.0f;    //r
    MaterialAmbient[1] = 0.1f;    //g
    MaterialAmbient[2] = 0.06f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.0f;    //r
    MaterialDiffuse[1] = 0.50980392f;    //g
    MaterialDiffuse[2] = 0.50980392f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.50196078f;    //r
    MaterialSpecular[1] = 0.50196078f;    //g
    MaterialSpecular[2] = 0.50196078f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.25 * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(1.3f, 1.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 15- 3rd sphere on 2nd column, green plastic------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.0f;    //r
    MaterialAmbient[1] = 0.0f;    //g
    MaterialAmbient[2] = 0.0f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.1f;    //r
    MaterialDiffuse[1] = 0.35f;    //g
    MaterialDiffuse[2] = 0.1f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.45f;    //r
    MaterialSpecular[1] = 0.55f;    //g
    MaterialSpecular[2] = 0.45f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.25 * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(1.3f, 0.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 16- 4th sphere on 3rd column, red plastic------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.0f;    //r
    MaterialAmbient[1] = 0.0f;    //g
    MaterialAmbient[2] = 0.0f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.5f;    //r
    MaterialDiffuse[1] = 0.0f;    //g
    MaterialDiffuse[2] = 0.0f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.7f;    //r
    MaterialSpecular[1] = 0.6f;    //g
    MaterialSpecular[2] = 0.6f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.25 * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(1.3f, -0.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 17- 5th sphere on 3rd column, white plastic------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.0f;    //r
    MaterialAmbient[1] = 0.0f;    //g
    MaterialAmbient[2] = 0.02f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.55f;    //r
    MaterialDiffuse[1] = 0.55f;    //g
    MaterialDiffuse[2] = 0.55f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.70f;    //r
    MaterialSpecular[1] = 0.70f;    //g
    MaterialSpecular[2] = 0.70f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.25 * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(1.3f, -1.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 18- 6th sphere on 3rd column, yellow plastic ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.0f;    //r
    MaterialAmbient[1] = 0.0f;    //g
    MaterialAmbient[2] = 0.0f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.5f;    //r
    MaterialDiffuse[1] = 0.5f;    //g
    MaterialDiffuse[2] = 0.0f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.60f;    //r
    MaterialSpecular[1] = 0.60f;    //g
    MaterialSpecular[2] = 0.50f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.25 * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(1.3f, -2.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 19- 1st sphere on 4th column, black rubber ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.02f;    //r
    MaterialAmbient[1] = 0.02f;    //g
    MaterialAmbient[2] = 0.02f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.01f;    //r
    MaterialDiffuse[1] = 0.01f;    //g
    MaterialDiffuse[2] = 0.01f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.4f;    //r
    MaterialSpecular[1] = 0.4f;    //g
    MaterialSpecular[2] = 0.4f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.078125 * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(3.5f, 2.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 20- 2nd sphere on 4th column, cyan rubber ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.0f;    //r
    MaterialAmbient[1] = 0.05f;    //g
    MaterialAmbient[2] = 0.05f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.4f;    //r
    MaterialDiffuse[1] = 0.5f;    //g
    MaterialDiffuse[2] = 0.5f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.04f;    //r
    MaterialSpecular[1] = 0.7f;    //g
    MaterialSpecular[2] = 0.7f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.078125 * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(3.5f, 1.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 21- 3rd sphere on 4th column, green rubber ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.0f;    //r
    MaterialAmbient[1] = 0.05f;    //g
    MaterialAmbient[2] = 0.0f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.4f;    //r
    MaterialDiffuse[1] = 0.5f;    //g
    MaterialDiffuse[2] = 0.4f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.04f;    //r
    MaterialSpecular[1] = 0.7f;    //g
    MaterialSpecular[2] = 0.04f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.078125 * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(3.5f, 0.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 22- 4th sphere on 4th column, red rubber------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.05f;    //r
    MaterialAmbient[1] = 0.0f;    //g
    MaterialAmbient[2] = 0.0f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.5f;    //r
    MaterialDiffuse[1] = 0.4f;    //g
    MaterialDiffuse[2] = 0.4f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.7f;    //r
    MaterialSpecular[1] = 0.04f;    //g
    MaterialSpecular[2] = 0.04f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.078125 * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(3.5f, -0.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 23- 5th sphere on 4th column, white ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.05f;    //r
    MaterialAmbient[1] = 0.05f;    //g
    MaterialAmbient[2] = 0.05f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.5f;    //r
    MaterialDiffuse[1] = 0.5f;    //g
    MaterialDiffuse[2] = 0.5f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.7f;    //r
    MaterialSpecular[1] = 0.7f;    //g
    MaterialSpecular[2] = 0.7f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.078125 * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(3.5f, -1.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

    //viewMatrix = perspectiveProjectionMatrix * modelMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major


    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    // glDrawArrays(GLenum, GLint , GLsizei)
    // (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

    //unbind va0
    glBindVertexArray(0);

    //------------------------------------------------------------------------------------
    //-------------------- 24- 6th sphere on 4th column, yellow rubber ------------------------------

    //initialize above matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    projectionMatrix = mat4::identity();

    ratationMatrix = mat4::identity();
    translationMatrix = mat4::identity();

    MaterialAmbient[0] = 0.05f;    //r
    MaterialAmbient[1] = 0.05f;    //g
    MaterialAmbient[2] = 0.0f;    //b
    MaterialAmbient[3] = 1.0f;        //a

    MaterialDiffuse[0] = 0.5f;    //r
    MaterialDiffuse[1] = 0.5f;    //g
    MaterialDiffuse[2] = 0.4f;    //b
    MaterialDiffuse[3] = 1.0f;        //a

    MaterialSpecular[0] = 0.7f;    //r
    MaterialSpecular[1] = 0.7f;    //g
    MaterialSpecular[2] = 0.04f;    //b
    MaterialSpecular[3] = 1.0f;        //a

    MaterialShininess[0] = 0.078125 * 128;

    //for lights
    if (bLKeyPressed == true)
    {
        glUniform1i(LKeyPressedUniform, 1);
        glUniform1f(shininessUniform, 1);

        glUniform3fv(LaUniform, 1, LightAmbient);
        glUniform3fv(KaUniform, 1, MaterialAmbient);

        glUniform3fv(LdUniform, 1, LightDiffuse);
        glUniform3fv(KdUniform, 1, MaterialDiffuse);

        glUniform3fv(LsUniform, 1, LightSpecular);
        glUniform3fv(KsUniform, 1, MaterialSpecular);

        if (iCount == 1)
        {
            ratationMatrix = rotate(lightAngle, 1.0f, 0.0f, 0.0f);

            LightPosition[0] = 0.0f;
            LightPosition[1] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 2)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 1.0f, 0.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = 0.0f;
            LightPosition[2] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
        else if (iCount == 3)
        {
            ratationMatrix = rotate(lightAngle, 0.0f, 0.0f, 1.0f);

            LightPosition[0] = (GLfloat)cos(lightAngle) * radius;
            LightPosition[1] = (GLfloat)sin(lightAngle) * radius;
            LightPosition[2] = 0.0f;
            LightPosition[3] = 0.0f;

            glUniform4fv(lightPositionUniform, 1, (GLfloat *)LightPosition);
        }
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    //do necessary transformation if ant required
    translationMatrix = translate(3.5f, -2.5f, -11.0f);

    //do necessary matrix multiplication

    modelMatrix = translationMatrix * ratationMatrix;

    projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);
    //glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
 
    //bind with vao(this will avoid many repetitive binding with vbo)
    glBindVertexArray(vao_sphere);

    //similarly bind the textures if any

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);

    //draw necessary scene
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

    //unbind va0
    glBindVertexArray(0);

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

	Draw24Spheres();
    
    glUseProgram(0);
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    
    lightAngle = lightAngle + 0.005f;
    if (lightAngle >= 360)
    {
        lightAngle = 0.0f;
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
            
        case 'F':
        case 'f':
            [[self window]toggleFullScreen:self];
            break;
            
        case 'L':
        case 'l':
            if (bLKeyPressed == FALSE)
            {
                bLKeyPressed = TRUE;
            }
            else
            {
                bLKeyPressed = FALSE;
            }
            break;
            
        case 'X':
        case 'x':
            iCount = 1;
            break;

        case 'Y':
        case 'y':
            iCount = 2;
            break;

        case 'Z':
        case 'z':
            iCount = 3;
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
    if (vbo_sphere_element)
    {
        glDeleteBuffers(1, &vbo_sphere_element);
        vbo_sphere_element = 0;
    }

    if (vbo_sphere_normal)
    {
        glDeleteBuffers(1, &vbo_sphere_normal);
        vbo_sphere_normal = 0;
    }

    if (vbo_sphere_position)
    {
        glDeleteBuffers(1, &vbo_sphere_position);
        vbo_sphere_position = 0;
    }

    if (vao_sphere)
    {
        glDeleteVertexArrays(1, &vao_sphere);
        vao_sphere = 0;
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
