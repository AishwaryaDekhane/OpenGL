#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import "vmath.h"
#import "GLESView.h"
#import "sphere.h"

#define RADIUS 100.0f

enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOODR_0
};

FILE *gpFile = NULL;

GLuint model_uniform;
GLuint view_uniform;
GLuint projection_uniform;

GLuint La_uniform;
GLuint Ld_uniform;
GLuint Ls_uniform;
GLuint lightPosition_uniform;

GLuint Ka_uniform;
GLuint Kd_uniform;
GLuint Ks_uniform;
GLuint shininess_uniform;
GLuint LKeyPressed_Uniform;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_texture[764];
short sphere_elements[2280];
unsigned int gNumVertices;
unsigned int gNumElements;

float LightAmbient[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float LightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float LightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float LightPosition[4] = { 100.0f, 100.0f, 100.0f, 1.0f };

float MaterialAmbient[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float MaterialDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float MaterialSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float MaterialShininess = 128.0f;

struct material_array
{
    float MaterialAmbient[4];
    float MaterialDiffuse[4];
    float MaterialSpecular[4];
    float MaterialShininess;
};

material_array mArray[24];
int number = 0;
float lightAngle = 0.0f;

BOOL bLight = NO;

int giWindowWidth = 0;
int giWindowHeight = 0;

@implementation GLESView
{
    EAGLContext *eaglContext;
    
    GLuint defaultFramebuffer;
    GLuint colorRenderbuffer;
    GLuint depthRenderbuffer;
    
    id displayLink;
    NSInteger animationFrameInterval;
    BOOL isAnimating;
    
    GLuint gVertexShaderObject;
    GLuint gFragmentShaderObject;
    GLuint gShaderProgramObject;
    
    GLuint vao_sphere;
    GLuint vbo_sphere_position;
    GLuint vbo_sphere_normal;
    GLuint vbo_sphere_element;
    vmath::mat4 perspectiveProjectionMatrix;
    
}

- (id)initWithFrame:(CGRect)frameRect
{
    //variables
    GLint iShaderCompileStatus = 0;
    GLint iProgramLinkStatus = 0;
    GLint iInfoLogLength = 0;
    GLchar *szInfoLog = NULL;
    
    //code
    self=[super initWithFrame:frameRect];
    if(self)
    {
        //initialise code here
        
        CAEAGLLayer *eaglLayer=(CAEAGLLayer *)super.layer;
        
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE],
                                        kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8,
                                        kEAGLDrawablePropertyColorFormat,
                                        nil];
        
        eaglContext = [[EAGLContext alloc]initWithAPI:kEAGLRenderingAPIOpenGLES3];
        if(eaglContext==nil)
        {
            [self release];
            return(nil);
        }
        [EAGLContext setCurrentContext:eaglContext];
        
        glGenFramebuffers(1, &defaultFramebuffer);
        glGenRenderbuffers(1, &colorRenderbuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
        
        [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
        
        GLint backingWidth;
        GLint backingHeight;
        
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
        
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("Failed To Create Complete Framebuffer Object %x\n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
            
            glDeleteFramebuffers(1, &defaultFramebuffer);
            glDeleteRenderbuffers(1, &colorRenderbuffer);
            glDeleteRenderbuffers(1, &depthRenderbuffer);
            
            return(nil);
        }
        
        printf("Renderer : %s | GL version : %s | GLSL version : %s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
        
        //hard coded initialization
        isAnimating = NO;
        animationFrameInterval = 60; 
        
        // Vertex Shader 
        //define vertex shader object
        gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
        
        //write vertex shader code
        const GLchar *vertexShaderSourceCode =
        "#version 300 es" \
        "\n" \
        "precision mediump int;" \
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
       
        //error checking
        glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
        
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
                    
                    printf("Vertex Shader Error : \n %s \n", szInfoLog);
                    free(szInfoLog);
                    [self release];
                }
            }
        }
        // Fragment Shader Code 
        
        //define fragment shader object
        gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
        
        //write shader code
        const GLchar *fragmentShaderSourceCode =
        "#version 300 es" \
        "\n" \
        "precision highp float;" \
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
        "vec3 phong_ads_light;" \
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

        glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);
        
        //compile the shader
        glCompileShader(gFragmentShaderObject);
        
        //error checking
        iShaderCompileStatus = 0;
        iInfoLogLength = 0;
        szInfoLog = NULL;
        
        glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
        
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
                    printf("Fragment Shader Error : \n %s \n", szInfoLog);
                    free(szInfoLog);
                    [self release];
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
        
        //error checking
        
        iInfoLogLength = 0;
        szInfoLog = NULL;
        
        glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iProgramLinkStatus);
        
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
                    printf("Program Link Error : \n %s\n", szInfoLog);
                    free(szInfoLog);
                    [self release];
                }
            }
        }
        
        //post linking retriving uniform location
        model_uniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
        view_uniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
        projection_uniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
        LKeyPressed_Uniform = glGetUniformLocation(gShaderProgramObject, "u_LKeyPressed");
        La_uniform = glGetUniformLocation(gShaderProgramObject, "u_La");
        Ld_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ld");
        Ls_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ls");
        Ka_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");
        Kd_uniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
        Ks_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");
        shininess_uniform = glGetUniformLocation(gShaderProgramObject, "shininess");
        lightPosition_uniform = glGetUniformLocation(gShaderProgramObject, "u_light_position");

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
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        
        //make orthograhic projection matrix a identity matrix
        perspectiveProjectionMatrix = vmath::mat4::identity();
        
        //material call
        [self 24Sphere];
        
        //clear color
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        
        //GESTURE RECOGNITION
        UITapGestureRecognizer *singleTapGestureRecognizer=[[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(onSingleTap:)];
        [singleTapGestureRecognizer setNumberOfTapsRequired:1];
        [singleTapGestureRecognizer setNumberOfTouchesRequired:1]; 
        [singleTapGestureRecognizer setDelegate:self];
        [self addGestureRecognizer:singleTapGestureRecognizer];
        
        UITapGestureRecognizer *doubleTapGestureRecognizer=
        [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(onDoubleTap:)];
        [doubleTapGestureRecognizer setNumberOfTapsRequired:2];
        [doubleTapGestureRecognizer setNumberOfTouchesRequired:1];
        [doubleTapGestureRecognizer setDelegate:self];
        [self addGestureRecognizer:doubleTapGestureRecognizer];
        
        //this allow to differentiate between single tap and double tap
        [singleTapGestureRecognizer requireGestureRecognizerToFail:doubleTapGestureRecognizer];
        
        //swipe gesture
        UISwipeGestureRecognizer *swipeGestureRecognizer=[[UISwipeGestureRecognizer alloc]initWithTarget:self action:@selector(onSwipe:)];
        [self addGestureRecognizer:swipeGestureRecognizer];
        
        //long press gesture
        UILongPressGestureRecognizer *longPressGestureRecognizer=[[UILongPressGestureRecognizer alloc]initWithTarget:self action:@selector(onLongPress:)];
        [self addGestureRecognizer:longPressGestureRecognizer];
    }
    return(self);
}

+(Class)layerClass
{
    //code
    return([CAEAGLLayer class]);
}

- (void)Draw24Spheres
{
    int i = 0;
    float x = -7.0f;
    float y = 3.0f;
    
    //declaration of metrices
    vmath::mat4 modelMatrix;
    vmath::mat4 viewMatrix;
    vmath::mat4 projectionMatrix;
    vmath::mat4 translationMatrix;
    
    for (i = 0; i < 24; i++)
    {
        glViewport((i % 6) * giWindowWidth / 6, giWindowHeight - (i / 6 + 1) * giWindowHeight / 4, (GLsizei)giWindowWidth / 6, (GLsizei)giWindowHeight / 4);
        
        perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)(giWindowWidth / 6) / (GLfloat)(giWindowHeight / 4), 0.1f, 100.0f);
        modelMatrix = vmath::mat4::identity();
        viewMatrix = vmath::mat4::identity();
        projectionMatrix = vmath::mat4::identity();
        translationMatrix = vmath::mat4::identity();
        
        if (x <= 5.0f)
        {
            x = x + 2.0f;
        }
        if (x > 5.0f)
        {
            x = -5.0f;
            y = y - 2.0f;
        }
        translationMatrix = vmath::translate(0.0f, 0.0f, -3.0f);
        
        //do necessary matrix multiplication
        modelMatrix = modelMatrix * translationMatrix;
        projectionMatrix *= perspectiveProjectionMatrix;
        
        //send necessary matrics to shaders in respective uniforms
        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(view_uniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, projectionMatrix);
        
        if (bLight)
        {
            glUniform1i(LKeyPressed_Uniform, 1);

            glUniform3fv(La_uniform, 1, LightAmbient);
            glUniform3fv(Ld_uniform, 1, LightDiffuse);
            glUniform3fv(Ls_uniform, 1, LightSpecular);

            glUniform3fv(Ka_uniform, 1, mArray[i].MaterialAmbient);
            glUniform3fv(Kd_uniform, 1, mArray[i].MaterialDiffuse);
            glUniform3fv(Ks_uniform, 1, mArray[i].MaterialSpecular);

            glUniform1f(shininess_uniform, mArray[i].MaterialShininess);

            if (number == 1)
            {
                LightPosition[0] = 0.0f;
                LightPosition[1] = cosf(lightAngle) * RADIUS;
                LightPosition[2] = sinf(lightAngle) * RADIUS;
                LightPosition[3] = 1.0f;
            }
            
            if (number == 2)
            {
                LightPosition[0] = cosf(lightAngle) * RADIUS;
                LightPosition[1] = 0.0f;
                LightPosition[2] = sinf(lightAngle) * RADIUS;
                LightPosition[3] = 1.0f;
            }
            
            if (number == 3)
            {
                LightPosition[0] = cosf(lightAngle) * RADIUS;
                LightPosition[1] = sinf(lightAngle) * RADIUS;
                LightPosition[2] = 0.0f;
                LightPosition[3] = 1.0f;
            }
            
            glUniform4fv(lightPosition_uniform, 1, LightPosition);
        }
        else
        {
            glUniform1i(LKeyPressed_Uniform, 0);
        }
        
        glBindVertexArray(vao_sphere);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
        glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
        
        glBindVertexArray(0);
    }
}

- (void)24Sphere
{
    //code
    //-------------------- 1st sphere on 1st column, emerald ------------------------------
    mArray[0].MaterialAmbient[0] = 0.0215f;
    mArray[0].MaterialAmbient[1] = 0.1745f;
    mArray[0].MaterialAmbient[2] = 0.0215f;
    mArray[0].MaterialAmbient[3] = 1.0f;
    mArray[0].MaterialDiffuse[0] = 0.07568f;
    mArray[0].MaterialDiffuse[1] = 0.61424f;
    mArray[0].MaterialDiffuse[2] = 0.07568f;
    mArray[0].MaterialDiffuse[3] = 1.0f;
    mArray[0].MaterialSpecular[0] = 0.633f;
    mArray[0].MaterialSpecular[1] = 0.727811f;
    mArray[0].MaterialSpecular[2] = 0.633f;
    mArray[0].MaterialSpecular[3] = 1.0f;
    mArray[0].MaterialShininess = 0.6f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 2nd sphere on 1st column, jade ------------------------------
    mArray[1].MaterialAmbient[0] = 0.135f;
    mArray[1].MaterialAmbient[1] = 0.2225f;
    mArray[1].MaterialAmbient[2] = 0.1575f;
    mArray[1].MaterialAmbient[3] = 1.0f;
    mArray[1].MaterialDiffuse[0] = 0.54f;
    mArray[1].MaterialDiffuse[1] = 0.89f;
    mArray[1].MaterialDiffuse[2] = 0.63f;
    mArray[1].MaterialDiffuse[3] = 1.0f;
    mArray[1].MaterialSpecular[0] = 0.316228f;
    mArray[1].MaterialSpecular[1] = 0.316228f;
    mArray[1].MaterialSpecular[2] = 0.316228f;
    mArray[1].MaterialSpecular[3] = 1.0f;
    mArray[1].MaterialShininess = 0.1f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 3rd sphere on 1st column, obsidian ------------------------------
    mArray[2].MaterialAmbient[0] = 0.05375f;
    mArray[2].MaterialAmbient[1] = 0.05f;
    mArray[2].MaterialAmbient[2] = 0.06625f;
    mArray[2].MaterialAmbient[3] = 1.0f;
    mArray[2].MaterialDiffuse[0] = 0.18275f;
    mArray[2].MaterialDiffuse[1] = 0.17f;
    mArray[2].MaterialDiffuse[2] = 0.22525f;
    mArray[2].MaterialDiffuse[3] = 1.0f;
    mArray[2].MaterialSpecular[0] = 0.332741f;
    mArray[2].MaterialSpecular[1] = 0.328634f;
    mArray[2].MaterialSpecular[2] = 0.346435f;
    mArray[2].MaterialSpecular[3] = 1.0f;
    mArray[2].MaterialShininess = 0.3f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 4th sphere on 1st column, pearl ------------------------------
    mArray[3].MaterialAmbient[0] = 0.25f;
    mArray[3].MaterialAmbient[1] = 0.20725f;
    mArray[3].MaterialAmbient[2] = 0.20725f;
    mArray[3].MaterialAmbient[3] = 1.0f;
    mArray[3].MaterialDiffuse[0] = 1.0f;
    mArray[3].MaterialDiffuse[1] = 0.829f;
    mArray[3].MaterialDiffuse[2] = 0.829f;
    mArray[3].MaterialDiffuse[3] = 1.0f;
    mArray[3].MaterialSpecular[0] = 0.296648f;
    mArray[3].MaterialSpecular[1] = 0.296648f;
    mArray[3].MaterialSpecular[2] = 0.296648f;
    mArray[3].MaterialSpecular[3] = 1.0f;
    mArray[3].MaterialShininess = 0.088f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 5th sphere on 1st column, ruby ------------------------------
    mArray[4].MaterialAmbient[0] = 0.1745f;
    mArray[4].MaterialAmbient[1] = 0.01175f;
    mArray[4].MaterialAmbient[2] = 0.01175f;
    mArray[4].MaterialAmbient[3] = 1.0f;
    mArray[4].MaterialDiffuse[0] = 0.61424f;
    mArray[4].MaterialDiffuse[1] = 0.04136f;
    mArray[4].MaterialDiffuse[2] = 0.04136f;
    mArray[4].MaterialDiffuse[3] = 1.0f;
    mArray[4].MaterialSpecular[0] = 0.727811f;
    mArray[4].MaterialSpecular[1] = 0.626959f;
    mArray[4].MaterialSpecular[2] = 0.626959f;
    mArray[4].MaterialSpecular[3] = 1.0f;
    mArray[4].MaterialShininess = 0.6f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 6th sphere on 1st column, turquoise ------------------------------
    mArray[5].MaterialAmbient[0] = 0.1f;
    mArray[5].MaterialAmbient[1] = 0.18725f;
    mArray[5].MaterialAmbient[2] = 0.1745f;
    mArray[5].MaterialAmbient[3] = 1.0f;
    mArray[5].MaterialDiffuse[0] = 0.396f;
    mArray[5].MaterialDiffuse[1] = 0.74151f;
    mArray[5].MaterialDiffuse[2] = 0.69102f;
    mArray[5].MaterialDiffuse[3] = 1.0f;
    mArray[5].MaterialSpecular[0] = 0.297254f;
    mArray[5].MaterialSpecular[1] = 0.30829f;
    mArray[5].MaterialSpecular[2] = 0.306678f;
    mArray[5].MaterialSpecular[3] = 1.0f;
    mArray[5].MaterialShininess = 0.1f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 7- 1st sphere on 2nd column, brass ------------------------------
    mArray[6].MaterialAmbient[0] = 0.329412f;
    mArray[6].MaterialAmbient[1] = 0.223529f;
    mArray[6].MaterialAmbient[2] = 0.027451f;
    mArray[6].MaterialAmbient[3] = 1.0f;
    mArray[6].MaterialDiffuse[0] = 0.782392f;
    mArray[6].MaterialDiffuse[1] = 0.568627f;
    mArray[6].MaterialDiffuse[2] = 0.113725f;
    mArray[6].MaterialDiffuse[3] = 1.0f;
    mArray[6].MaterialSpecular[0] = 0.992157f;
    mArray[6].MaterialSpecular[1] = 0.941176f;
    mArray[6].MaterialSpecular[2] = 0.807843f;
    mArray[6].MaterialSpecular[3] = 1.0f;
    mArray[6].MaterialShininess = 0.21794872f * 128.0f;
    
   //------------------------------------------------------------------------------------
	//-------------------- 8- 2nd sphere on 2nd column, bronze ------------------------------
    mArray[7].MaterialAmbient[0] = 0.2125f;
    mArray[7].MaterialAmbient[1] = 0.1275f;
    mArray[7].MaterialAmbient[2] = 0.054f;
    mArray[7].MaterialAmbient[3] = 1.0f;
    mArray[7].MaterialDiffuse[0] = 0.714f;
    mArray[7].MaterialDiffuse[1] = 0.4284f;
    mArray[7].MaterialDiffuse[2] = 0.18144f;
    mArray[7].MaterialDiffuse[3] = 1.0f;
    mArray[7].MaterialSpecular[0] = 0.393548f;
    mArray[7].MaterialSpecular[1] = 0.271906f;
    mArray[7].MaterialSpecular[2] = 0.166721f;
    mArray[7].MaterialSpecular[3] = 1.0f;
    mArray[7].MaterialShininess = 0.2f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 9- 3rd sphere on 2nd column, chrome -----------------------

    mArray[8].MaterialAmbient[0] = 0.25f;
    mArray[8].MaterialAmbient[1] = 0.25f;
    mArray[8].MaterialAmbient[2] = 0.25f;
    mArray[8].MaterialAmbient[3] = 1.0f;
    mArray[8].MaterialDiffuse[0] = 0.4f;
    mArray[8].MaterialDiffuse[1] = 0.4f;
    mArray[8].MaterialDiffuse[2] = 0.4f;
    mArray[8].MaterialDiffuse[3] = 1.0f;
    mArray[8].MaterialSpecular[0] = 0.774597f;
    mArray[8].MaterialSpecular[1] = 0.774597f;
    mArray[8].MaterialSpecular[2] = 0.774597f;
    mArray[8].MaterialSpecular[3] = 1.0f;
    mArray[8].MaterialShininess = 0.6f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 10- 4th sphere on 2nd column, copper -----------------------
    mArray[9].MaterialAmbient[0] = 0.19125f;
    mArray[9].MaterialAmbient[1] = 0.0735f;
    mArray[9].MaterialAmbient[2] = 0.0225f;
    mArray[9].MaterialAmbient[3] = 1.0f;
    mArray[9].MaterialDiffuse[0] = 0.7038f;
    mArray[9].MaterialDiffuse[1] = 0.27048f;
    mArray[9].MaterialDiffuse[2] = 0.0828f;
    mArray[9].MaterialDiffuse[3] = 1.0f;
    mArray[9].MaterialSpecular[0] = 0.256777f;
    mArray[9].MaterialSpecular[1] = 0.137622f;
    mArray[9].MaterialSpecular[2] = 0.086014f;
    mArray[9].MaterialSpecular[3] = 1.0f;
    mArray[9].MaterialShininess = 0.1f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 11- 5th sphere on 2nd column, gold ------------------------------
    mArray[10].MaterialAmbient[0] = 0.24725f;
    mArray[10].MaterialAmbient[1] = 0.1995f;
    mArray[10].MaterialAmbient[2] = 0.0745f;
    mArray[10].MaterialAmbient[3] = 1.0f;
    mArray[10].MaterialDiffuse[0] = 0.75164f;
    mArray[10].MaterialDiffuse[1] = 0.60648f;
    mArray[10].MaterialDiffuse[2] = 0.22648f;
    mArray[10].MaterialDiffuse[3] = 1.0f;
    mArray[10].MaterialSpecular[0] = 0.628281f;
    mArray[10].MaterialSpecular[1] = 0.555802f;
    mArray[10].MaterialSpecular[2] = 0.366065f;
    mArray[10].MaterialSpecular[3] = 1.0f;
    mArray[10].MaterialShininess = 0.4f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 12- 6th sphere on 2nd column, silver ------------------------------
    mArray[11].MaterialAmbient[0] = 0.19225f;
    mArray[11].MaterialAmbient[1] = 0.19225f;
    mArray[11].MaterialAmbient[2] = 0.19225f;
    mArray[11].MaterialAmbient[3] = 1.0f;
    mArray[11].MaterialDiffuse[0] = 0.50754f;
    mArray[11].MaterialDiffuse[1] = 0.50754f;
    mArray[11].MaterialDiffuse[2] = 0.50754f;
    mArray[11].MaterialDiffuse[3] = 1.0f;
    mArray[11].MaterialSpecular[0] = 0.508273f;
    mArray[11].MaterialSpecular[1] = 0.508273f;
    mArray[11].MaterialSpecular[2] = 0.508273f;
    mArray[11].MaterialSpecular[3] = 1.0f;
    mArray[11].MaterialShininess = 0.4f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 13- 1st sphere on 3rd column, black plastic ------------------------------
    mArray[12].MaterialAmbient[0] = 0.0f;
    mArray[12].MaterialAmbient[1] = 0.0f;
    mArray[12].MaterialAmbient[2] = 0.0f;
    mArray[12].MaterialAmbient[3] = 1.0f;
    mArray[12].MaterialDiffuse[0] = 0.01f;
    mArray[12].MaterialDiffuse[1] = 0.01f;
    mArray[12].MaterialDiffuse[2] = 0.01f;
    mArray[12].MaterialDiffuse[3] = 1.0f;
    mArray[12].MaterialSpecular[0] = 0.50f;
    mArray[12].MaterialSpecular[1] = 0.50f;
    mArray[12].MaterialSpecular[2] = 0.50f;
    mArray[12].MaterialSpecular[3] = 1.0f;
    mArray[12].MaterialShininess = 0.25f * 128.0f;

    //------------------------------------------------------------------------------------
	//-------------------- 14- 2nd sphere on 3rd column, cyan plastic ------------------------------
    mArray[13].MaterialAmbient[0] = 0.0f;
    mArray[13].MaterialAmbient[1] = 0.1f;
    mArray[13].MaterialAmbient[2] = 0.06f;
    mArray[13].MaterialAmbient[3] = 1.0f;
    mArray[13].MaterialDiffuse[0] = 0.01f;
    mArray[13].MaterialDiffuse[1] = 0.50980392f;
    mArray[13].MaterialDiffuse[2] = 0.50980392f;
    mArray[13].MaterialDiffuse[3] = 1.0f;
    mArray[13].MaterialSpecular[0] = 0.50196078f;
    mArray[13].MaterialSpecular[1] = 0.50196078f;
    mArray[13].MaterialSpecular[2] = 0.50196078f;
    mArray[13].MaterialSpecular[3] = 1.0f;
    mArray[13].MaterialShininess = 0.25f * 128.0f;

    //------------------------------------------------------------------------------------
	//-------------------- 15- 3rd sphere on 2nd column, green plastic------------------------------

    mArray[14].MaterialAmbient[0] = 0.0f;
    mArray[14].MaterialAmbient[1] = 0.0f;
    mArray[14].MaterialAmbient[2] = 0.0f;
    mArray[14].MaterialAmbient[3] = 1.0f;
    mArray[14].MaterialDiffuse[0] = 0.1f;
    mArray[14].MaterialDiffuse[1] = 0.35f;
    mArray[14].MaterialDiffuse[2] = 0.1f;
    mArray[14].MaterialDiffuse[3] = 1.0f;
    mArray[14].MaterialSpecular[0] = 0.45f;
    mArray[14].MaterialSpecular[1] = 0.55f;
    mArray[14].MaterialSpecular[2] = 0.45f;
    mArray[14].MaterialSpecular[3] = 1.0f;
    mArray[14].MaterialShininess = 0.25f * 128.0f;

    //------------------------------------------------------------------------------------
	//-------------------- 16- 4th sphere on 3rd column, red plastic------------------------------
    mArray[15].MaterialAmbient[0] = 0.0f;
    mArray[15].MaterialAmbient[1] = 0.0f;
    mArray[15].MaterialAmbient[2] = 0.0f;
    mArray[15].MaterialAmbient[3] = 1.0f;
    mArray[15].MaterialDiffuse[0] = 0.5f;
    mArray[15].MaterialDiffuse[1] = 0.0f;
    mArray[15].MaterialDiffuse[2] = 0.0f;
    mArray[15].MaterialDiffuse[3] = 1.0f;
    mArray[15].MaterialSpecular[0] = 0.7f;
    mArray[15].MaterialSpecular[1] = 0.6f;
    mArray[15].MaterialSpecular[2] = 0.6f;
    mArray[15].MaterialSpecular[3] = 1.0f;
    mArray[15].MaterialShininess = 0.25f * 128.0f;

    //------------------------------------------------------------------------------------
	//-------------------- 17- 5th sphere on 3rd column, white plastic------------------------------

    mArray[16].MaterialAmbient[0] = 0.0f;
    mArray[16].MaterialAmbient[1] = 0.0f;
    mArray[16].MaterialAmbient[2] = 0.0f;
    mArray[16].MaterialAmbient[3] = 1.0f;
    mArray[16].MaterialDiffuse[0] = 0.55f;
    mArray[16].MaterialDiffuse[1] = 0.55f;
    mArray[16].MaterialDiffuse[2] = 0.55f;
    mArray[16].MaterialDiffuse[3] = 1.0f;
    mArray[16].MaterialSpecular[0] = 0.70f;
    mArray[16].MaterialSpecular[1] = 0.70f;
    mArray[16].MaterialSpecular[2] = 0.70f;
    mArray[16].MaterialSpecular[3] = 1.0f;
    mArray[16].MaterialShininess = 0.25f * 128.0f;

    //------------------------------------------------------------------------------------
	//-------------------- 18- 6th sphere on 3rd column, yellow plastic ------------------------------

    mArray[17].MaterialAmbient[0] = 0.0f;
    mArray[17].MaterialAmbient[1] = 0.0f;
    mArray[17].MaterialAmbient[2] = 0.0f;
    mArray[17].MaterialAmbient[3] = 1.0f;
    mArray[17].MaterialDiffuse[0] = 0.5f;
    mArray[17].MaterialDiffuse[1] = 0.5f;
    mArray[17].MaterialDiffuse[2] = 0.0f;
    mArray[17].MaterialDiffuse[3] = 1.0f;
    mArray[17].MaterialSpecular[0] = 0.60f;
    mArray[17].MaterialSpecular[1] = 0.60f;
    mArray[17].MaterialSpecular[2] = 0.50f;
    mArray[17].MaterialSpecular[3] = 1.0f;
    mArray[17].MaterialShininess = 0.25f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 19- 1st sphere on 4th column, black rubber ------------------------------

    mArray[18].MaterialAmbient[0] = 0.02f;
    mArray[18].MaterialAmbient[1] = 0.02f;
    mArray[18].MaterialAmbient[2] = 0.02f;
    mArray[18].MaterialAmbient[3] = 1.0f;
    mArray[18].MaterialDiffuse[0] = 0.01f;
    mArray[18].MaterialDiffuse[1] = 0.01f;
    mArray[18].MaterialDiffuse[2] = 0.01f;
    mArray[18].MaterialDiffuse[3] = 1.0f;
    mArray[18].MaterialSpecular[0] = 0.4f;
    mArray[18].MaterialSpecular[1] = 0.4f;
    mArray[18].MaterialSpecular[2] = 0.4f;
    mArray[18].MaterialSpecular[3] = 1.0f;
    mArray[18].MaterialShininess = 0.078125f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 20- 2nd sphere on 4th column, cyan rubber ------------------------------

    mArray[19].MaterialAmbient[0] = 0.0f;
    mArray[19].MaterialAmbient[1] = 0.05f;
    mArray[19].MaterialAmbient[2] = 0.05f;
    mArray[19].MaterialAmbient[3] = 1.0f;
    mArray[19].MaterialDiffuse[0] = 0.4f;
    mArray[19].MaterialDiffuse[1] = 0.5f;
    mArray[19].MaterialDiffuse[2] = 0.5f;
    mArray[19].MaterialDiffuse[3] = 1.0f;
    mArray[19].MaterialSpecular[0] = 0.04f;
    mArray[19].MaterialSpecular[1] = 0.7f;
    mArray[19].MaterialSpecular[2] = 0.7f;
    mArray[19].MaterialSpecular[3] = 1.0f;
    mArray[19].MaterialShininess = 0.078125f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 21- 3rd sphere on 4th column, green rubber ------------------------------

    mArray[20].MaterialAmbient[0] = 0.0f;
    mArray[20].MaterialAmbient[1] = 0.05f;
    mArray[20].MaterialAmbient[2] = 0.0f;
    mArray[20].MaterialAmbient[3] = 1.0f;
    mArray[20].MaterialDiffuse[0] = 0.4f;
    mArray[20].MaterialDiffuse[1] = 0.5f;
    mArray[20].MaterialDiffuse[2] = 0.4f;
    mArray[20].MaterialDiffuse[3] = 1.0f;
    mArray[20].MaterialSpecular[0] = 0.04f;
    mArray[20].MaterialSpecular[1] = 0.7f;
    mArray[20].MaterialSpecular[2] = 0.04f;
    mArray[20].MaterialSpecular[3] = 1.0f;
    mArray[20].MaterialShininess = 0.078125f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 22- 4th sphere on 4th column, red rubber------------------------------

    mArray[21].MaterialAmbient[0] = 0.05f;
    mArray[21].MaterialAmbient[1] = 0.0f;
    mArray[21].MaterialAmbient[2] = 0.0f;
    mArray[21].MaterialAmbient[3] = 1.0f;
    mArray[21].MaterialDiffuse[0] = 0.5f;
    mArray[21].MaterialDiffuse[1] = 0.4f;
    mArray[21].MaterialDiffuse[2] = 0.4f;
    mArray[21].MaterialDiffuse[3] = 1.0f;
    mArray[21].MaterialSpecular[0] = 0.7f;
    mArray[21].MaterialSpecular[1] = 0.04f;
    mArray[21].MaterialSpecular[2] = 0.04f;
    mArray[21].MaterialSpecular[3] = 1.0f;
    mArray[21].MaterialShininess = 0.078125f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 23- 5th sphere on 4th column, white ------------------------------

    mArray[22].MaterialAmbient[0] = 0.05f;
    mArray[22].MaterialAmbient[1] = 0.05f;
    mArray[22].MaterialAmbient[2] = 0.05f;
    mArray[22].MaterialAmbient[3] = 1.0f;
    mArray[22].MaterialDiffuse[0] = 0.5f;
    mArray[22].MaterialDiffuse[1] = 0.5f;
    mArray[22].MaterialDiffuse[2] = 0.5f;
    mArray[22].MaterialDiffuse[3] = 1.0f;
    mArray[22].MaterialSpecular[0] = 0.7f;
    mArray[22].MaterialSpecular[1] = 0.7f;
    mArray[22].MaterialSpecular[2] = 0.7f;
    mArray[22].MaterialSpecular[3] = 1.0f;
    mArray[22].MaterialShininess = 0.078125f * 128.0f;
    
    //------------------------------------------------------------------------------------
	//-------------------- 24- 6th sphere on 4th column, yellow rubber ------------------------------

    mArray[23].MaterialAmbient[0] = 0.05f;
    mArray[23].MaterialAmbient[1] = 0.05f;
    mArray[23].MaterialAmbient[2] = 0.0f;
    mArray[23].MaterialAmbient[3] = 1.0f;
    mArray[23].MaterialDiffuse[0] = 0.5f;
    mArray[23].MaterialDiffuse[1] = 0.5f;
    mArray[23].MaterialDiffuse[2] = 0.4f;
    mArray[23].MaterialDiffuse[3] = 1.0f;
    mArray[23].MaterialSpecular[0] = 0.7f;
    mArray[23].MaterialSpecular[1] = 0.7f;
    mArray[23].MaterialSpecular[2] = 0.04f;
    mArray[23].MaterialSpecular[3] = 1.0f;
    mArray[23].MaterialShininess = 0.078125f * 128.0f;
}


-(void)drawView:(id)sender
{
    //code
    [EAGLContext setCurrentContext:eaglContext];
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glUseProgram(gShaderProgramObject);
    
    [self Draw24Spheres];
    
    //unuse program
    glUseProgram(0);

    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];
    
    lightAngle = lightAngle + 0.005f;
    if (lightAngle >= 360)
    {
        lightAngle = 0.0f;
    }
}

-(void)layoutSubviews
{
    //code
    GLint width;
    GLint height;
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)self.layer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    
    giWindowWidth = width;
    giWindowHeight = height;
    
    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
    
    glViewport(0, 0, width, height);
    
    GLfloat fwidth = (GLfloat)width;
    GLfloat fheight = (GLfloat)height;
    
    perspectiveProjectionMatrix = vmath::perspective(45.0f, ((GLfloat)fwidth / (GLfloat)fheight), 0.1f, 100.0f);

    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Failed To Create Complete Framebuffer Object %x\n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
    
    [self drawView:nil];    //repaint
}

-(void)startAnimation
{
    if(!isAnimating)
    {
        displayLink=[NSClassFromString(@"CADisplayLink")
                     displayLinkWithTarget:self selector:@selector(drawView:)];
        [displayLink setPreferredFramesPerSecond:animationFrameInterval];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        
        isAnimating = YES;
    }
}

-(void)stopAnimation
{
    if(isAnimating)
    {
        [displayLink invalidate];
        displayLink=nil;
        
        isAnimating = NO;
    }
}

//to become first responder
- (BOOL)acceptsFirstResponder
{
    //code
    return (YES);
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    //code
}

-(void)onSingleTap:(UITapGestureRecognizer *)gr
{
    //code
    number = number + 1;
    if(number > 3)
    {
        number = 0;
    }
}

-(void)onDoubleTap:(UITapGestureRecognizer *)gr
{
    //code
    if (bLight == FALSE)
    {
        bLight = TRUE;
    }
    else
    {
        bLight = FALSE;
    }
}

-(void)onSwipe:(UISwipeGestureRecognizer *)gr
{
    //code
    [self release];
    exit(0);
}

-(void)onLongPress:(UILongPressGestureRecognizer *)gr
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
        
        //ask program how many shaders are attached
        glGetProgramiv(gShaderProgramObject, GL_ATTACHED_SHADERS, &shaderCount);
        
        GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);
        
        if (pShaders)
        {
            glGetAttachedShaders(gShaderProgramObject, shaderCount, &shaderCount, pShaders);
            
            for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
            {
                //detach shader
                glDetachShader(gShaderProgramObject, pShaders[shaderNumber]);
                //delete shader
                glDeleteShader(pShaders[shaderNumber]);
                pShaders[shaderNumber] = 0;
            }
            free(pShaders);
        }
        glDeleteProgram(gShaderProgramObject);
        gShaderProgramObject = 0;
        glUseProgram(0);
    }
    
    if(depthRenderbuffer)
    {
        glDeleteRenderbuffers(1, &depthRenderbuffer);
        depthRenderbuffer=0;
    }
    if(colorRenderbuffer)
    {
        glDeleteRenderbuffers(1, &colorRenderbuffer);
        colorRenderbuffer=0;
    }
    if(defaultFramebuffer)
    {
        glDeleteFramebuffers(1, &defaultFramebuffer);
        defaultFramebuffer=0;
    }
    
    if([EAGLContext currentContext]==eaglContext)
    {
        [EAGLContext setCurrentContext:nil];
    }
    [EAGLContext release];
    eaglContext=nil;
    
    [super dealloc];
}
@end
