#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import "vmath.h"
#import <math.h>
#import "GLESView.h"

enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOODR_0
};


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
    
    GLuint mvpUniform;
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
        "in vec4 vPosition;" \
        "in vec4 vColor;" \
        "out vec4 out_color;"
        "uniform mat4 u_mvp_matrix;" \
        "void main(void)" \
        "{" \
        "gl_Position = u_mvp_matrix * vPosition;" \
        "out_color = vColor;" \
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
        "in vec4 out_color;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
        "FragColor = out_color;" \
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
        glBindAttribLocation(gShaderProgramObject, AMC_ATTRIBUTE_COLOR, "vColor");
        
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
        mvpUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");
        
        [self initializeD ];
        [self initializeA ];
        [self initializeN ];
        [self initializeFirstI ];
        [self initializeSecondI ];
        //clear the window
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        
        //depth
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        
        //make orthograhic projection matrix a identity matrix
        perspectiveProjectionMatrix = vmath::mat4::identity();
        
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

-(void)drawView:(id)sender
{
    //code
    [EAGLContext setCurrentContext:eaglContext];
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glUseProgram(gShaderProgramObject);
    
   // OpenGL Drawing
	//declaration of matrices
    vmath::mat4 modelViewMatrix;
	vmath::mat4 modelViewProjectionMatrix;

	//-------------------------------------------------------------------------------------------------------------------

	//for first I

	//initialize above matrices to identity
	modelViewMatrix = vmath::mat4::identity();
	modelViewProjectionMatrix = vmath::mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = vmath::translate(0.0f, 0.0f, -2.0f);

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
	modelViewMatrix = vmath::mat4::identity();
	modelViewProjectionMatrix = vmath::mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = vmath::translate(0.0f, 0.0f, -2.0f);

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
	modelViewMatrix = vmath::mat4::identity();
	modelViewProjectionMatrix = vmath::mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = vmath::translate(0.0f, 0.0f, -2.0f);

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
	modelViewMatrix = vmath::mat4::identity();
	modelViewProjectionMatrix = vmath::mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = vmath::translate(0.0f, 0.0f, -2.0f);

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
	modelViewMatrix = vmath::mat4::identity();
	modelViewProjectionMatrix = vmath::mat4::identity();

	//do necessary transformation if ant required
	modelViewMatrix = vmath::translate(0.0f, 0.0f, -2.0f);

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
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];
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
