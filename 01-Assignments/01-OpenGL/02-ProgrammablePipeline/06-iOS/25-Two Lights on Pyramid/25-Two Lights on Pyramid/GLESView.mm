#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import "vmath.h"
#import "GLESView.h"
#import "sphere.h"

enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOODR_0
};

//global variable
FILE *gpFile = NULL;

GLuint ModelMatrixUniform;
GLuint ViewMatrixUniform;
GLuint ProjectionMatrixUniform;
GLuint LaUniform_red;
GLuint KaUniform;
GLuint LdUniform_red;
GLuint KdUniform;
GLuint LsUniform_red;
GLuint KsUniform;
GLuint LightPositionUniform_red;
GLuint MaterialShininessUniform;
GLuint LKeyPressedUniform;

GLuint ModelMatrixUniform_blue;
GLuint ViewMatrixUniform_blue;
GLuint ProjectionMatrixUniform_blue;
GLuint LaUniform_blue;
GLuint LdUniform_blue;
GLuint LsUniform_blue;
GLuint KaUniform_blue;
GLuint KdUniform_blue;
GLuint KsUniform_blue;
GLuint LightPositionUniform_blue;
GLuint MaterialShininessUniform_blue;
GLuint LKeyPressedUniform_blue;

float LightAngle = 0.0f;
//sphere related variables
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_texture[764];
short sphere_elements[2280];
unsigned int gNumVertices;
unsigned int gNumElements;

//light values
float LightAmbientZero[4] = { 0.0f,0.0f,0.0f,1.0f };
float LightDiffuseZero[4] = { 1.0f,0.0f,0.0f,1.0f };
float LightSpecularZero[4] = { 1.0f,0.0f,0.0f,1.0f };	//providing same values for diffuse and specular not compulsory but it looks good calculation is done using maths
float LightPositionZero[4] = { -2.0f,0.0f,0.0f,1.0f };

float LightAmbientOne[4] = { 0.0f,0.0f,0.0f,1.0f };
float LightDiffuseOne[4] = { 0.0f,0.0f,1.0f,1.0f };
float LightSpecularOne[4] = { 0.0f,0.0f,1.0f,1.0f };
float LightPositionOne[4] = { 2.0f,0.0f,0.0f,1.0f };

float MaterialAmbient[4] = { 0.0f,0.0f,0.0f,1.0f };
float MaterialDiffuse[4] = { 1.0f,1.0f,1.0f,1.0f };
float MaterialSpecular[4] = { 1.0f,1.0f,1.0f,1.0f };
float MaterialShininess = 128.f ;

//flags
BOOL bLight = NO;

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
    
    GLuint vao_pyramid;
	GLuint vbo_pyramid_position;
	GLuint vbo_pyramid_normal;
	GLuint vbo_pyramid_element;
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
        animationFrameInterval = 60;    //default since iOS 8.2
        
        /* Vertex Shader */
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
		"uniform mat4 u_projection_matrix;" \
		"uniform mat4 u_view_matrix;" \
		"uniform int u_LKeyPressed;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform float u_matrial_shininess;"
		"out vec3 phong_ADS_light;" \

		"uniform vec3 u_La_red;" \
		"uniform vec3 u_Ld_red;" \
		"uniform vec3 u_Ls_red;" \
		"uniform vec4 u_light_position_red;" \

		"uniform vec3 u_La_blue;" \
		"uniform vec3 u_Ld_blue;" \
		"uniform vec3 u_Ls_blue;" \
		"uniform vec4 u_light_position_blue;" \


		"void main(void)" \
		"{" \
		"if (u_LKeyPressed == 1)" \
		"{" \
		"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;" \
		"vec3 tNorm = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);" \

		"vec3 light_direction_red = normalize(vec3(u_light_position_red - eyeCoordinates));" \
		"float tn_dot_Ld_red = max(dot(light_direction_red, tNorm), 0.0);" \
		"vec3 reflection_vector_red = reflect(-light_direction_red, tNorm);" \
		"vec3 viewer_vector_red = normalize(vec3(-eyeCoordinates.xyz));" \

		"vec3 light_direction_blue = normalize(vec3(u_light_position_blue - eyeCoordinates));" \
		"float tn_dot_Ld_blue = max(dot(light_direction_blue, tNorm), 0.0);" \
		"vec3 reflection_vector_blue = reflect(-light_direction_blue, tNorm);" \
		"vec3 viewer_vector_blue = normalize(vec3(-eyeCoordinates.xyz));" \

		"vec3 ambient = u_La_red * u_Ka + u_La_blue * u_Ka;" \
		"vec3 diffuse = u_Ld_red * u_Kd * tn_dot_Ld_red + u_Ld_blue * u_Kd * tn_dot_Ld_blue;" \
		"vec3 specular = u_Ls_red * u_Ks * pow(max(dot(reflection_vector_red, viewer_vector_red), 0.0), u_matrial_shininess) +  u_Ls_blue * u_Ks * pow(max(dot(reflection_vector_blue, viewer_vector_blue), 0.0), u_matrial_shininess);" \
		"phong_ADS_light = ambient + diffuse + specular;" \

	
		"}" \
		"else" \
		"{" \
		"phong_ADS_light = vec3(1.0, 1.0, 1.0);" \
		"}" \
		"gl_Position = u_projection_matrix * u_model_matrix * u_view_matrix * vPosition;" \
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
        /* Fragment Shader Code */
        
        //define fragment shader object
        gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
        
        //write shader code
        const GLchar *fragmentShaderSourceCode =
        "#version 300 es" \
        "\n" \
        "precision highp float;" \
        "\n" \
		"in vec3 phong_ADS_light;" \
		"out vec4 FragColor;" \
		"uniform int u_LKeyPressed;" \
		"void main(void)" \
		"{" \
		"if (u_LKeyPressed == 1)" \
		"{" \
		"FragColor = vec4(phong_ADS_light,1.0);" \
		"}" \
		"else" \
		"{" \
		"FragColor = vec4(1.0, 1.0, 1.0, 1.0);" \
		"}" \
		"}";

        //specify above shader code to fragment shader object
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
      
        ModelMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	ProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
	ViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");

	LKeyPressedUniform = glGetUniformLocation(gShaderProgramObject, "u_LKeyPressed");
	MaterialShininessUniform = glGetUniformLocation(gShaderProgramObject, "u_matrial_shininess");

	LaUniform_red = glGetUniformLocation(gShaderProgramObject, "u_La_red");
	LaUniform_blue = glGetUniformLocation(gShaderProgramObject, "u_La_blue");
	KaUniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");

	LdUniform_red = glGetUniformLocation(gShaderProgramObject, "u_Ld_red");
	LdUniform_blue = glGetUniformLocation(gShaderProgramObject, "u_Ld_blue");
	KdUniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");

	LsUniform_red = glGetUniformLocation(gShaderProgramObject, "u_Ls_red");
	LsUniform_blue = glGetUniformLocation(gShaderProgramObject, "u_Ls_blue");
	KsUniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");

	LightPositionUniform_red = glGetUniformLocation(gShaderProgramObject, "u_light_position_red");
	LightPositionUniform_blue = glGetUniformLocation(gShaderProgramObject, "u_light_position_blue");

	//declare vertex3f coordinates here

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
		-1.0f, -1.0f, 1.0f,
	};

	const GLfloat pyramidNormals[] = 
	{
		0.0f,0.447214f,0.894427f,	//front-top
		0.0f,0.447214f,0.894427f,	//front-left
		0.0f,0.447214f,0.894427f,	//front-right

		0.894427f, 0.447214f, 0.0f,	//right-top
		0.894427f, 0.447214f, 0.0f,	//right-left
		0.894427f, 0.447214f, 0.0f,	//right-right

		0.0f, 0.447214f, -0.894427f,	//back-top
		0.0f, 0.447214f, -0.894427f,	//back-left
		0.0f, 0.447214f, -0.894427f,	//back-right

		-0.894427f, 0.447214f, 0.0f,	//left-top
		-0.894427f, 0.447214f, 0.0f,	//left-left
		-0.894427f, 0.447214f, 0.0f,	//left-right

	};


	//--------------------------------------------------------------------------------------------------------------

	//For sphere

	//vao = VertexArrayObject
	glGenVertexArrays(1, &vao_pyramid);

	//bind array with vao
	glBindVertexArray(vao_pyramid);

	//-------------------------------------------------------------------------------------------------------

	//For position

	//create vbo
	glGenBuffers(1, &vbo_pyramid_position);

	//target vbo <-> GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_position);

	//fill the buffer data statically
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

	//give strides to vPosition
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);  //GL_FALSE = we are not sending normalized co-or

																					//enable array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp

														//unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	//-------------------------------------------------------------------------------------------------------


	//For Normals

	//create vbo
	glGenBuffers(1, &vbo_pyramid_normal);

	//target vbo <-> GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_normal);

	//fill the buffer data statically
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNormals), pyramidNormals, GL_STATIC_DRAW);

	//give strides to vPosition
	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);  //GL_FALSE = we are not sending normalized co-or

																				  //enable array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);	//AMC = AstroMediComp

														//unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	//unbind vao
	glBindVertexArray(0);
	//----------------------------------------------------------------------------------------------------------
        
        //depth
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        
        //make orthograhic projection matrix a identity matrix
        perspectiveProjectionMatrix = vmath::mat4::identity();
        
        //clear color
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        
        //GESTURE RECOGNITION
        //Tap Gesture Code
        UITapGestureRecognizer *singleTapGestureRecognizer=[[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(onSingleTap:)];
        [singleTapGestureRecognizer setNumberOfTapsRequired:1];
        [singleTapGestureRecognizer setNumberOfTouchesRequired:1];  //touch of 1 finger
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

-(void)drawView:(id)sender
{
    //code
    [EAGLContext setCurrentContext:eaglContext];
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glUseProgram(gShaderProgramObject);
    
  //for lights
	if (bLight == true)
	{
		//for red light

		glUniform1i(LKeyPressedUniform, 1);
		glUniform1f(MaterialShininessUniform, 1);

		glUniform3fv(LaUniform_red, 1, LightAmbientZero);
		glUniform3fv(KaUniform, 1, MaterialAmbient);

		glUniform3fv(LdUniform_red, 1, LightDiffuseZero);
		glUniform3fv(KdUniform, 1, MaterialDiffuse);

		glUniform3fv(LsUniform_red, 1, LightSpecularZero);
		glUniform3fv(KsUniform, 1, MaterialSpecular);

		float lightPosition_red[] = { -2.0f,0.0f,0.0f,1.0f };
		glUniform4fv(LightPositionUniform_red, 1, (GLfloat *)lightPosition_red);

		//for blue light

		glUniform3fv(LaUniform_blue, 1, LightAmbientOne);
		
		glUniform3fv(LdUniform_blue, 1, LightDiffuseOne);
		
		glUniform3fv(LsUniform_blue, 1, LightSpecularOne);
		
		float lightPosition_blue[] = { 2.0f,0.0f,0.0f,1.0f };
		glUniform4fv(LightPositionUniform_blue, 1, (GLfloat *)lightPosition_blue);
	}
	else
	{
		glUniform1i(LKeyPressedUniform, 0);
	}

	// OpenGL Drawing
	//declaration of matrices
	vmath::mat4 modelMatrix;
	vmath::mat4 viewMatrix;
	vmath::mat4 projectionMatrix;

	//for rotation
	vmath::mat4 ratationMatrix;
	vmath::mat4 translationMatrix;


	//For rectangle
	//initialize above matrices to identity
	modelMatrix = vmath::mat4::identity();
	viewMatrix = vmath::mat4::identity();
	projectionMatrix = vmath::mat4::identity();

	ratationMatrix = vmath::mat4::identity();
	translationMatrix = vmath::mat4::identity();

	//do necessary transformation if ant required
	translationMatrix = vmath::translate(0.0f, 0.0f, -5.0f);

	ratationMatrix = vmath::rotate(0.0f, LightAngle, 0.0f);

	//do necessary matrix multiplication

	modelMatrix = translationMatrix * ratationMatrix;

	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix;

	//viewMatrix = perspectiveProjectionMatrix * modelMatrix;
	//this is internally done  by gluOrho2D/ glOrtho

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(ModelMatrixUniform, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, projectionMatrix);
	//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniformMatrix4fv(ViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	//GL_FALSE = internally we are not transposing = row -> col / col -> row
	//OpenGL is col major while DirectX is row major

	//bind with vao(this will avoid many repetitive binding with vbo)
	glBindVertexArray(vao_pyramid);

	//similarly bind the textures if any

	//draw necessary scene
	//glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	// glDrawArrays(GLenum, GLint , GLsizei)
	// (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

	//unbind va0
	glBindVertexArray(0);

    
    //unuse program
    glUseProgram(0);

    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];
    
    LightAngle = LightAngle + 0.5f;
    if (LightAngle >= 360)
    {
        LightAngle = 0.0f;
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
