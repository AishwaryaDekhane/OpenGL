#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "vmath.h"
#import "sphere.h"

#define RADIUS  100.0f
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

struct
{
    GLuint modelUniform;
    GLuint viewUniform;
    GLuint projectionUniform;

    GLuint LaUniform_red;
    GLuint LaUniform_blue;
    GLuint LaUniform_green;
    GLuint LdUniform_red;
    GLuint LdUniform_green;
    GLuint LdUniform_blue;
    GLuint LsUniform_red;
    GLuint LsUniform_green;
    GLuint LsUniform_blue;
    GLuint lightPositionUniform_red;
    GLuint lightPositionUniform_green;
    GLuint lightPositionUniform_blue;

    GLuint KaUniform;
    GLuint KdUniform;
    GLuint KsUniform;
    GLuint shininessUniform;

    GLuint LKeyPressedUniform;
}pv, pf;

//sphere variables
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_texture[764];
short sphere_elements[2280];
unsigned int gNumVertices;
unsigned int gNumElements;

//flags
BOOL bLKeyPressed = FALSE;
BOOL bPerVertex = TRUE;
BOOL bPerFragment = FALSE;


float LightAmbient_one[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float LightDiffuse_one[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
float LightSpecular_one[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
float LightPosition_one[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

float LightAmbient_two[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float LightDiffuse_two[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
float LightSpecular_two[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
float LightPosition_two[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

float LightAmbient_three[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float LightDiffuse_three[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
float LightSpecular_three[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
float LightPosition_three[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

float MaterialAmbient[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float MaterialDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float MaterialSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float MaterialShininess = 128.0f;

float LightAngle_red = 0.0f;
float LightAngle_green = 0.0f;
float LightAngle_blue = 0.0f;

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
    
    window=[[NSWindow alloc] initWithContentRect:win_rect
                                       styleMask:NSWindowStyleMaskTitled |
            NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                                         backing:NSBackingStoreBuffered
                                           defer:NO];
    [window setTitle:@"macOS Window:Per Vertex Per Fragment Toggle"];
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
    GLuint gVertexShaderObject_pv;
    GLuint gVertexShaderObject_pf;
    GLuint gFragmentShaderObject_pv;
    GLuint gFragmentShaderObject_pf;
    GLuint gShaderProgramObject_pv;
    GLuint gShaderProgramObject_pf;
        
    GLuint vao_sphere;
    GLuint vbo_sphere_position;
    GLuint vbo_sphere_normal;
    GLuint vbo_sphere_element;
    
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
    
    /**** Per Vertex ****/

    
    //VERTEX SHADER
    //define vertex shader object
    gVertexShaderObject_pv = glCreateShader(GL_VERTEX_SHADER);

    //write vertex shader code
    const GLchar *vertexShaderSourceCode_pv =
        "#version 410" \
        "\n" \
        "in vec4 vPosition;" \
        "in vec3 vNormal;" \
        "uniform mat4 u_model_matrix;" \
        "uniform mat4 u_view_matrix;" \
        "uniform mat4 u_projection_matrix;" \
        "uniform int u_LKeyPressed;" \
        
        "uniform vec3 u_La_red;" \
        "uniform vec3 u_La_green;" \
        "uniform vec3 u_La_blue;" \
        
        "uniform vec3 u_Ld_red;" \
        "uniform vec3 u_Ld_green;" \
        "uniform vec3 u_Ld_blue;" \
        
        "uniform vec3 u_Ls_red;" \
        "uniform vec3 u_Ls_green;" \
        "uniform vec3 u_Ls_blue;" \

        "uniform vec4 u_light_position_red;" \
        "uniform vec4 u_light_position_green;" \
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
                "vec3 light_direction_green = normalize(vec3(u_light_position_green - eye_coordinates));" \
                "vec3 light_direction_blue = normalize(vec3(u_light_position_blue - eye_coordinates));" \

                "float tn_dot_LightDirection_red = max(dot(light_direction_red, transformed_normal), 0.0);" \
                "float tn_dot_LightDirection_green = max(dot(light_direction_green, transformed_normal), 0.0);" \
                "float tn_dot_LightDirection_blue = max(dot(light_direction_blue, transformed_normal), 0.0);" \

                "vec3 reflection_vector_red = reflect(-light_direction_red, transformed_normal);" \
                "vec3 reflection_vector_green = reflect(-light_direction_green, transformed_normal);" \
                "vec3 reflection_vector_blue = reflect(-light_direction_blue, transformed_normal);" \

                "vec3 viewer_vector = normalize(vec3(-eye_coordinates.xyz));" \

                "vec3 ambient = (u_La_red * u_Ka) + (u_La_green * u_Ka) + (u_La_blue * u_Ka);" \
                "vec3 diffuse = (u_Ld_red * u_Kd * tn_dot_LightDirection_red) + (u_Ld_green * u_Kd * tn_dot_LightDirection_green) + (u_Ld_blue * u_Kd * tn_dot_LightDirection_blue);" \
                "vec3 specular = (u_Ls_red * u_Ks * pow(max(dot(reflection_vector_red, viewer_vector), 0.0), shininess)) + (u_Ls_green * u_Ks * pow(max(dot(reflection_vector_green, viewer_vector), 0.0), shininess)) + (u_Ls_blue * u_Ks * pow(max(dot(reflection_vector_blue, viewer_vector), 0.0), shininess));" \

                "phong_ads_light = ambient + diffuse + specular;" \
            "}" \
            "else" \
            "{" \
                "phong_ads_light = vec3(1.0, 1.0, 1.0);" \
            "}" \
            "gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
        "}";

    //specify above source code to vertex shader object
    glShaderSource(gVertexShaderObject_pv, 1, (const GLchar **)&vertexShaderSourceCode_pv, NULL);
    glCompileShader(gVertexShaderObject_pv);

    //step - 1
    glGetShaderiv(gVertexShaderObject_pv, GL_COMPILE_STATUS, &iShaderCompileStatus);
    //step - 2
    if (iShaderCompileStatus == GL_FALSE)
    {
        glGetShaderiv(gVertexShaderObject_pv, GL_INFO_LOG_LENGTH, &iInfoLogLength);

        if (iInfoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(iInfoLogLength);

            if (szInfoLog != NULL)
            {
                GLsizei Written;
                glGetShaderInfoLog(gVertexShaderObject_pv,
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
    gFragmentShaderObject_pv = glCreateShader(GL_FRAGMENT_SHADER);

    //write shader code
    const GLchar *fragmentShaderSourceCode_pv =
        "#version 410" \
        "\n" \
        "in vec3 phong_ads_light;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
        "FragColor = vec4(phong_ads_light, 0.0);" \
        "}";
    //specify above shader code to fragment shader object
    glShaderSource(gFragmentShaderObject_pv, 1, (const GLchar **)&fragmentShaderSourceCode_pv, NULL);
    glCompileShader(gFragmentShaderObject_pv);

    iShaderCompileStatus = 0;
    iInfoLogLength = 0;
    szInfoLog = NULL;

    //step - 1
    glGetShaderiv(gFragmentShaderObject_pv, GL_COMPILE_STATUS, &iShaderCompileStatus);

    //step - 2
    if (iShaderCompileStatus == GL_FALSE)
    {
        glGetShaderiv(gFragmentShaderObject_pv, GL_INFO_LOG_LENGTH, &iInfoLogLength);

        if (iInfoLogLength > 0)
        {

            szInfoLog = (GLchar *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei Written;
                glGetShaderInfoLog(gFragmentShaderObject_pv,
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
    gShaderProgramObject_pv = glCreateProgram();

    //Attach Vertex Shader
    glAttachShader(gShaderProgramObject_pv, gVertexShaderObject_pv);

    //Attach Fragment Shader
    glAttachShader(gShaderProgramObject_pv, gFragmentShaderObject_pv);

    //pre linking bonding to vertex attributes
    glBindAttribLocation(gShaderProgramObject_pv, AMC_ATTRIBUTE_POSITION, "vPosition");
    glBindAttribLocation(gShaderProgramObject_pv, AMC_ATTRIBUTE_NORMAL, "vNormal");

    //link the shader porgram
    glLinkProgram(gShaderProgramObject_pv);

    iInfoLogLength = 0;
    szInfoLog = NULL;
    
    //step - 1
    glGetProgramiv(gShaderProgramObject_pv, GL_LINK_STATUS, &iProgramLinkStatus);

    //step - 2
    if (iProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(gShaderProgramObject_pv, GL_INFO_LOG_LENGTH, &iInfoLogLength);

        if (iInfoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(iInfoLogLength);

            if (szInfoLog != NULL)
            {
                GLsizei Written;
                glGetProgramInfoLog(gShaderProgramObject_pv, iInfoLogLength, &Written, szInfoLog);
                fprintf(gpFile, "Program Link Error : \n %s\n", szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }

    //post linking retriving uniform location
    pv.LKeyPressedUniform = glGetUniformLocation(gShaderProgramObject_pv, "u_LKeyPressed");

    pv.modelUniform = glGetUniformLocation(gShaderProgramObject_pv, "u_model_matrix");
    pv.viewUniform = glGetUniformLocation(gShaderProgramObject_pv, "u_view_matrix");
    pv.projectionUniform = glGetUniformLocation(gShaderProgramObject_pv, "u_projection_matrix");

    
    pv.LaUniform_red = glGetUniformLocation(gShaderProgramObject_pv, "u_La_red");
    pv.LaUniform_green = glGetUniformLocation(gShaderProgramObject_pv, "u_La_green");
    pv.LaUniform_blue = glGetUniformLocation(gShaderProgramObject_pv, "u_La_blue");

    pv.LdUniform_red = glGetUniformLocation(gShaderProgramObject_pv, "u_Ld_red");
    pv.LdUniform_green = glGetUniformLocation(gShaderProgramObject_pv, "u_Ld_green");
    pv.LdUniform_blue = glGetUniformLocation(gShaderProgramObject_pv, "u_Ld_blue");

    pv.LsUniform_red = glGetUniformLocation(gShaderProgramObject_pv, "u_Ls_red");
    pv.LsUniform_green = glGetUniformLocation(gShaderProgramObject_pv, "u_Ls_green");
    pv.LsUniform_blue = glGetUniformLocation(gShaderProgramObject_pv, "u_Ls_blue");

    pv.KaUniform = glGetUniformLocation(gShaderProgramObject_pv, "u_Ka");
    pv.KdUniform = glGetUniformLocation(gShaderProgramObject_pv, "u_Kd");
    pv.KsUniform = glGetUniformLocation(gShaderProgramObject_pv, "u_Ks");
    
    pv.shininessUniform = glGetUniformLocation(gShaderProgramObject_pv, "shininess");
    
    pv.lightPositionUniform_red = glGetUniformLocation(gShaderProgramObject_pv, "u_light_position_red");
    pv.lightPositionUniform_green= glGetUniformLocation(gShaderProgramObject_pv, "u_light_position_green");
    pv.lightPositionUniform_blue = glGetUniformLocation(gShaderProgramObject_pv, "u_light_position_blue");
    
    /**** Per Fragment ****/

    //VERTEX SHADER
    //define vertex shader object
    gVertexShaderObject_pf = glCreateShader(GL_VERTEX_SHADER);

    //write vertex shader code
    const GLchar *vertexShaderSourceCode_pf =
        "#version 410" \
        "\n" \
       "in vec4 vPosition;" \
        "in vec3 vNormal;" \
        "uniform mat4 u_model_matrix;" \
        "uniform mat4 u_view_matrix;" \
        "uniform mat4 u_projection_matrix;" \
        "uniform int u_LKeyPressed;" \

        "uniform vec4 u_light_position_red;" \
        "uniform vec4 u_light_position_green;" \
        "uniform vec4 u_light_position_blue;" \
        
        "out vec3 t_norm;" \
        
        "out vec3 light_direction_red;" \
        "out vec3 light_direction_green;" \
        "out vec3 light_direction_blue;" \

        "out vec3 viewer_vector;" \
        "void main(void)" \
        "{" \
            "if (u_LKeyPressed == 1)" \
            "{" \
                "vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;" \
                "mat3 normal_matrix = mat3(transpose(inverse(u_view_matrix * u_model_matrix)));" \
                "t_norm = normal_matrix * vNormal;" \

                "light_direction_red = vec3(u_light_position_red - eye_coordinates);" \
                "light_direction_green = vec3(u_light_position_green - eye_coordinates);" \
                "light_direction_blue = vec3(u_light_position_blue - eye_coordinates);" \

                "viewer_vector = vec3(-eye_coordinates);" \
            "}" \
            "gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
        "}";

    //specify above source code to vertex shader object
    glShaderSource(gVertexShaderObject_pf, 1, (const GLchar **)&vertexShaderSourceCode_pf, NULL);
    glCompileShader(gVertexShaderObject_pf);
    
    //step - 1
    glGetShaderiv(gVertexShaderObject_pf, GL_COMPILE_STATUS, &iShaderCompileStatus);

    //step - 2
    if (iShaderCompileStatus == GL_FALSE)
    {
        glGetShaderiv(gVertexShaderObject_pf, GL_INFO_LOG_LENGTH, &iInfoLogLength);

        if (iInfoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(iInfoLogLength);

            if (szInfoLog != NULL)
            {
                GLsizei Written;
                glGetShaderInfoLog(gVertexShaderObject_pf,
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
    gFragmentShaderObject_pf = glCreateShader(GL_FRAGMENT_SHADER);

    //write shader code
    const GLchar *fragmentShaderSourceCode_pf =
        "#version 410" \
        "\n" \
        "in vec3 t_norm;" \

        "in vec3 light_direction_red;" \
        "in vec3 light_direction_green;" \
        "in vec3 light_direction_blue;" \

        "in vec3 viewer_vector;" \

        "uniform int u_LKeyPressed;" \

        "uniform vec3 u_La_red;" \
        "uniform vec3 u_La_green;" \
        "uniform vec3 u_La_blue;" \

        "uniform vec3 u_Ld_red;" \
        "uniform vec3 u_Ld_green;" \
        "uniform vec3 u_Ld_blue;" \

        "uniform vec3 u_Ls_red;" \
        "uniform vec3 u_Ls_green;" \
        "uniform vec3 u_Ls_blue;" \

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

                "vec3 normalised_light_direction_red = normalize(light_direction_red);" \
                "vec3 normalised_light_direction_green = normalize(light_direction_green);" \
                "vec3 normalised_light_direction_blue = normalize(light_direction_blue);" \

                "vec3 normalised_viewer_vector = normalize(viewer_vector);" \

                "vec3 reflection_vector_red = reflect(-normalised_light_direction_red, normalised_transformed_normal);" \
                "vec3 reflection_vector_green = reflect(-normalised_light_direction_green, normalised_transformed_normal);" \
                "vec3 reflection_vector_blue = reflect(-normalised_light_direction_blue, normalised_transformed_normal);" \

                "float tn_dot_LightDirection_red = max(dot(normalised_light_direction_red, normalised_transformed_normal), 0.0);" \
                "float tn_dot_LightDirection_green = max(dot(normalised_light_direction_green, normalised_transformed_normal), 0.0);" \
                "float tn_dot_LightDirection_blue = max(dot(normalised_light_direction_blue, normalised_transformed_normal), 0.0);" \

                "vec3 ambient = (u_La_red * u_Ka) + (u_La_green * u_Ka) + (u_La_blue * u_Ka);" \
                "vec3 diffuse = (u_Ld_red * u_Kd * tn_dot_LightDirection_red) + (u_Ld_green * u_Kd * tn_dot_LightDirection_green) + (u_Ld_blue * u_Kd * tn_dot_LightDirection_blue);" \
                "vec3 specular = (u_Ls_red * u_Ks * pow(max(dot(reflection_vector_red, normalised_viewer_vector), 0.0), shininess)) + (u_Ls_green * u_Ks * pow(max(dot(reflection_vector_green, normalised_viewer_vector), 0.0), shininess)) + (u_Ls_blue * u_Ks * pow(max(dot(reflection_vector_blue, normalised_viewer_vector), 0.0), shininess));" \
                
                "phong_ads_light = ambient + diffuse + specular;" \
            "}" \
            "else" \
            "{" \
                "phong_ads_light = vec3(1.0, 1.0, 1.0);" \
            "}" \
            "FragColor = vec4(phong_ads_light, 0.0);" \
        "}";    //specify above shader code to fragment shader object
    glShaderSource(gFragmentShaderObject_pf, 1, (const GLchar **)&fragmentShaderSourceCode_pf, NULL);

    //compile the shader
    glCompileShader(gFragmentShaderObject_pf);

    iShaderCompileStatus = 0;
    iInfoLogLength = 0;
    szInfoLog = NULL;
    
    //step - 1
    glGetShaderiv(gFragmentShaderObject_pf, GL_COMPILE_STATUS, &iShaderCompileStatus);

    //step - 2
    if (iShaderCompileStatus == GL_FALSE)
    {
        glGetShaderiv(gFragmentShaderObject_pf, GL_INFO_LOG_LENGTH, &iInfoLogLength);

        if (iInfoLogLength > 0)
        {

            szInfoLog = (GLchar *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei Written;
                glGetShaderInfoLog(gFragmentShaderObject_pf,
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
    gShaderProgramObject_pf = glCreateProgram();

    //Attach Vertex Shader
    glAttachShader(gShaderProgramObject_pf, gVertexShaderObject_pf);

    //Attach Fragment Shader
    glAttachShader(gShaderProgramObject_pf, gFragmentShaderObject_pf);

    //pre linking bonding to vertex attributes
    glBindAttribLocation(gShaderProgramObject_pf, AMC_ATTRIBUTE_POSITION, "vPosition");
    glBindAttribLocation(gShaderProgramObject_pf, AMC_ATTRIBUTE_NORMAL, "vNormal");

    //link the shader porgram
    glLinkProgram(gShaderProgramObject_pf);

    //error checking

    iInfoLogLength = 0;
    szInfoLog = NULL;

    glGetProgramiv(gShaderProgramObject_pf, GL_LINK_STATUS, &iProgramLinkStatus);

    if (iProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(gShaderProgramObject_pf, GL_INFO_LOG_LENGTH, &iInfoLogLength);

        if (iInfoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(iInfoLogLength);

            if (szInfoLog != NULL)
            {
                GLsizei Written;
                glGetProgramInfoLog(gShaderProgramObject_pf, iInfoLogLength, &Written, szInfoLog);
                fprintf(gpFile, "Program Link Error : \n %s\n", szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }

    //post linking retriving uniform location
    pf.LKeyPressedUniform = glGetUniformLocation(gShaderProgramObject_pf, "u_LKeyPressed");

    pf.modelUniform = glGetUniformLocation(gShaderProgramObject_pf, "u_model_matrix");
    pf.viewUniform = glGetUniformLocation(gShaderProgramObject_pf, "u_view_matrix");
    pf.projectionUniform = glGetUniformLocation(gShaderProgramObject_pf, "u_projection_matrix");

    pf.LaUniform_red = glGetUniformLocation(gShaderProgramObject_pf, "u_La_red");
    pf.LaUniform_green = glGetUniformLocation(gShaderProgramObject_pf, "u_La_green");
    pf.LaUniform_blue = glGetUniformLocation(gShaderProgramObject_pf, "u_La_blue");

    pf.LdUniform_red = glGetUniformLocation(gShaderProgramObject_pf, "u_Ld_red");
    pf.LdUniform_green = glGetUniformLocation(gShaderProgramObject_pf, "u_Ld_green");
    pf.LdUniform_blue = glGetUniformLocation(gShaderProgramObject_pf, "u_Ld_blue");

    pf.LsUniform_red = glGetUniformLocation(gShaderProgramObject_pf, "u_Ls_red");
    pf.LsUniform_green = glGetUniformLocation(gShaderProgramObject_pf, "u_Ls_green");
    pf.LsUniform_blue = glGetUniformLocation(gShaderProgramObject_pf, "u_Ls_blue");

    pf.KaUniform = glGetUniformLocation(gShaderProgramObject_pf, "u_Ka");
    pf.KdUniform = glGetUniformLocation(gShaderProgramObject_pf, "u_Kd");
    pf.KsUniform = glGetUniformLocation(gShaderProgramObject_pf, "u_Ks");

    pf.shininessUniform = glGetUniformLocation(gShaderProgramObject_pf, "shininess");

    pf.lightPositionUniform_red = glGetUniformLocation(gShaderProgramObject_pf, "u_light_position_red");
    pf.lightPositionUniform_green = glGetUniformLocation(gShaderProgramObject_pf, "u_light_position_green");
    pf.lightPositionUniform_blue = glGetUniformLocation(gShaderProgramObject_pf, "u_light_position_blue");
    
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
    
    //declaration of metrices
    mat4 modelMatrix_pv;
    mat4 modelMatrix_pf;
    mat4 viewMatrix_pv;
    mat4 viewMatrix_pf;
    mat4 projectionMatrix_pv;
    mat4 projectionMatrix_pf;
    mat4 translationMatrix_pv;

    mat4 rotationMatrixRed_pv;
    mat4 rotationMatrixGreen_pv;
    mat4 rotationMatrixBlue_pv;

    mat4 translationMatrix_pf;

    mat4 rotationMatrixRed_pf;
    mat4 rotationMatrixGreen_pf;
    mat4 rotationMatrixBlue_pf;

    //init above metrices to identity
    modelMatrix_pv = mat4::identity();
    viewMatrix_pv = mat4::identity();
    projectionMatrix_pv = mat4::identity();
    translationMatrix_pv = mat4::identity();

    //do necessary transformations here
    translationMatrix_pv = translate(0.0f, 0.0f, -3.0f);

    //do necessary matrix multiplication
    modelMatrix_pv = modelMatrix_pv * translationMatrix_pv;
    projectionMatrix_pv *= perspectiveProjectionMatrix;

    //init above metrices to identity
    modelMatrix_pf = mat4::identity();
    viewMatrix_pf = mat4::identity();
    projectionMatrix_pf = mat4::identity();
    translationMatrix_pf = mat4::identity();

    //do necessary transformations 
    translationMatrix_pf = translate(0.0f, 0.0f, -3.0f);

    //do necessary matrix multiplication
    modelMatrix_pf = modelMatrix_pf * translationMatrix_pf;
    projectionMatrix_pf *= perspectiveProjectionMatrix;

    if (bPerVertex)
    {
        glUseProgram(gShaderProgramObject_pv);

        glUniformMatrix4fv(pv.modelUniform, 1, GL_FALSE, modelMatrix_pv);
        glUniformMatrix4fv(pv.viewUniform, 1, GL_FALSE, viewMatrix_pv);
        glUniformMatrix4fv(pv.projectionUniform, 1, GL_FALSE, projectionMatrix_pv);

        if (bLKeyPressed)
        {
            glUniform1i(pv.LKeyPressedUniform, 1);
            glUniform3fv(pv.LaUniform_red, 1, LightAmbient_one);
            glUniform3fv(pv.LaUniform_green, 1, LightAmbient_two);
            glUniform3fv(pv.LaUniform_blue, 1, LightAmbient_three);

            glUniform3fv(pv.LdUniform_red, 1, LightDiffuse_one);
            glUniform3fv(pv.LdUniform_green, 1, LightDiffuse_two);
            glUniform3fv(pv.LdUniform_blue, 1, LightDiffuse_three);

            glUniform3fv(pv.LsUniform_red, 1, LightSpecular_one);
            glUniform3fv(pv.LsUniform_green, 1, LightSpecular_two);
            glUniform3fv(pv.LsUniform_blue, 1, LightSpecular_three);
            
            glUniform3fv(pv.KaUniform, 1, MaterialAmbient);
            glUniform3fv(pv.KdUniform, 1, MaterialDiffuse);
            glUniform3fv(pv.KsUniform, 1, MaterialSpecular);
            glUniform1f(pv.shininessUniform, MaterialShininess);
            
            LightPosition_one[0] = 0.0f;
            LightPosition_one[1] = RADIUS * cosf(LightAngle_red);
            LightPosition_one[2] = RADIUS * sinf(LightAngle_red);
            LightPosition_one[3] = 1.0f;
            glUniform4fv(pv.lightPositionUniform_red, 1, LightPosition_one);

            LightPosition_two[0] = RADIUS * cosf(LightAngle_green);
            LightPosition_two[1] = 0.0f;
            LightPosition_two[2] = RADIUS * sinf(LightAngle_green);
            LightPosition_two[3] = 1.0f;
            glUniform4fv(pv.lightPositionUniform_green, 1, LightPosition_two);

            LightPosition_three[0] = RADIUS * cosf(LightAngle_blue);
            LightPosition_three[1] = RADIUS * sinf(LightAngle_blue);
            LightPosition_three[2] = 0.0f;
            LightPosition_three[3] = 1.0f;
            glUniform4fv(pv.lightPositionUniform_blue, 1, LightPosition_three);
        }
        else
        {
            glUniform1i(pv.LKeyPressedUniform, 0);
        }
    }

    if (bPerFragment)
    {
        glUseProgram(gShaderProgramObject_pf);

        glUniformMatrix4fv(pf.modelUniform, 1, GL_FALSE, modelMatrix_pf);
        glUniformMatrix4fv(pf.viewUniform, 1, GL_FALSE, viewMatrix_pf);
        glUniformMatrix4fv(pf.projectionUniform, 1, GL_FALSE, projectionMatrix_pf);

        if (bLKeyPressed)
        {
            glUniform1i(pf.LKeyPressedUniform, 1);
            glUniform3fv(pf.LaUniform_red, 1, LightAmbient_one);
            glUniform3fv(pf.LaUniform_green, 1, LightAmbient_two);
            glUniform3fv(pf.LaUniform_blue, 1, LightAmbient_three);

            glUniform3fv(pf.LdUniform_red, 1, LightDiffuse_one);
            glUniform3fv(pf.LdUniform_green, 1, LightDiffuse_two);
            glUniform3fv(pf.LdUniform_blue, 1, LightDiffuse_three);

            glUniform3fv(pf.LsUniform_red, 1, LightSpecular_one);
            glUniform3fv(pf.LsUniform_green, 1, LightSpecular_two);
            glUniform3fv(pf.LsUniform_blue, 1, LightSpecular_three);

            glUniform3fv(pf.KaUniform, 1, MaterialAmbient);
            glUniform3fv(pf.KdUniform, 1, MaterialDiffuse);
            glUniform3fv(pf.KsUniform, 1, MaterialSpecular);
            glUniform1f(pf.shininessUniform, MaterialShininess);

            LightPosition_one[0] = 0.0f;
            LightPosition_one[1] = RADIUS * cosf(LightAngle_red);
            LightPosition_one[2] = RADIUS * sinf(LightAngle_red);
            LightPosition_one[3] = 1.0f;
            glUniform4fv(pf.lightPositionUniform_red, 1, LightPosition_one);

            LightPosition_two[0] = RADIUS * cosf(LightAngle_green);
            LightPosition_two[1] = 0.0f;
            LightPosition_two[2] = RADIUS * sinf(LightAngle_green);
            LightPosition_two[3] = 1.0f;
            glUniform4fv(pf.lightPositionUniform_green, 1, LightPosition_two);

            LightPosition_three[0] = RADIUS * cosf(LightAngle_blue);
            LightPosition_three[1] = RADIUS * sinf(LightAngle_blue);
            LightPosition_three[2] = 0.0f;
            LightPosition_three[3] = 1.0f;
            glUniform4fv(pf.lightPositionUniform_blue, 1, LightPosition_three);
        }

        else
        {
            glUniform1i(pf.LKeyPressedUniform, 0);
        }
    }

    if (bPerVertex == TRUE || bPerFragment == TRUE)
    {
        //bind with vao
        glBindVertexArray(vao_sphere);

        //draw scene
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
        glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

        //unbind vao
        glBindVertexArray(0);
    }

    //unuse program
    glUseProgram(0);
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    
    LightAngle_red = LightAngle_red + 0.02f;
    if (LightAngle_red >= 360)
    {
        LightAngle_red = 0.0f;
    }

    LightAngle_green = LightAngle_green + 0.02f;
    if (LightAngle_green >= 360)
    {
        LightAngle_green = 0.0f;
    }

    LightAngle_blue = LightAngle_blue + 0.02f;
    if (LightAngle_blue >= 360)
    {
        LightAngle_blue = 0.0f;
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
            [[self window]toggleFullScreen:self];
            break;

        case 'Q':
        case 'q':
            [self release];
            [NSApp terminate:self];
            break;
            
        case 'F':
        case 'f':
            if (bPerFragment == FALSE)
            {
            
                bPerFragment = TRUE;
                if (bPerVertex == TRUE)
                {
                    bPerVertex = FALSE;
                }
            }
            else
            {
                bPerFragment = FALSE;
            }
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

        case 'V':
        case 'v':
            if (bPerVertex == FALSE)
            {
                bPerVertex = TRUE;
                if (bPerFragment == TRUE)
                {
                    bPerFragment = FALSE;
                }
            }
            else
            {
                bPerVertex = FALSE;
            }
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

    if (gShaderProgramObject_pv)
    {
        GLsizei shaderCount;
        GLsizei shaderNumber;

        glUseProgram(gShaderProgramObject_pv);

        glGetProgramiv(gShaderProgramObject_pv, GL_ATTACHED_SHADERS, &shaderCount);

        GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);

        if (pShaders)
        {
            glGetAttachedShaders(gShaderProgramObject_pv, shaderCount, &shaderCount, pShaders);

            for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
            {
                glDetachShader(gShaderProgramObject_pv, pShaders[shaderNumber]);
                glDeleteShader(pShaders[shaderNumber]);
                pShaders[shaderNumber] = 0;
            }
            free(pShaders);
        }
        glDeleteProgram(gShaderProgramObject_pv);
        gShaderProgramObject_pv = 0;
        glUseProgram(0);
    }

    if (gShaderProgramObject_pf)
    {
        GLsizei shaderCount;
        GLsizei shaderNumber;

        glUseProgram(gShaderProgramObject_pf);

        glGetProgramiv(gShaderProgramObject_pf, GL_ATTACHED_SHADERS, &shaderCount);

        GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);

        if (pShaders)
        {
            glGetAttachedShaders(gShaderProgramObject_pf, shaderCount, &shaderCount, pShaders);

            for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
            {
                glDetachShader(gShaderProgramObject_pf, pShaders[shaderNumber]);
                glDeleteShader(pShaders[shaderNumber]);
                pShaders[shaderNumber] = 0;
            }
            free(pShaders);
        }
        glDeleteProgram(gShaderProgramObject_pf);
        gShaderProgramObject_pf = 0;
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
