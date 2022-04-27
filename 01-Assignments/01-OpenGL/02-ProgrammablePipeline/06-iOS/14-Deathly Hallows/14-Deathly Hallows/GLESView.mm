#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import "vmath.h"
#import <math.h>
#import "GLESView.h"
#define PI 3.141592653589793238463
enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOODR_0
};

float angle_Tri = 0.0f;
float angle_Circle = 0.0f;
float angle_line = 0.0f;

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
 
    GLuint vao_triangle;
    GLuint vbo_triangle_pos;
    GLuint vbo_triangle_col;
    
    GLuint vao_line;
    GLuint vbo_line_pos;
    GLuint vbo_line_col;
    
    GLuint vao_inner_circle;
    GLuint vbo_inner_circle_pos;
    GLuint vbo_inner_circle_col;
    
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
        
        /* Vertex Shader */
        //define vertex shader object
        gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
        
        //write vertex shader code
        const GLchar *vertexShaderSourceCode =
        "#version 300 es" \
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
        
        /*

    *   void glShaderSource(    GLuint shader,
    *   GLsizei count,
    *   const GLchar **string,
    *   const GLint *length);

    *   Parameters
    *   shader
    *   Specifies the handle of the shader object whose source code is to be replaced.

    *   count
    *   Specifies the number of elements in the string and length arrays.

    *   string
    *   Specifies an array of pointers to strings containing the source code to be loaded into the shader.

    *   length
    *   Specifies an array of string lengths.
    */

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
        "out vec4 FragColor;" \
        "in vec4 out_color;" \
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
        
        [self initializeInnerCircle];
        [self initializeTriangle];
        [self initializeLine];
        
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

-(void) initializeTriangle
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
    vmath::mat4 ratationMatrix;

    //-------------------------------------------------------------------------------------------------------------------

    //for triangle

    //initialize above matrices to identity
    modelViewMatrix = vmath::mat4::identity();
    modelViewProjectionMatrix = vmath::mat4::identity();
    ratationMatrix = vmath::mat4::identity();

    //do necessary transformation if ant required
    modelViewMatrix = vmath::translate(0.0f, 0.001f, -2.0f);

    ratationMatrix = vmath::rotate(angle_Tri, 0.0f, 1.0f, 0.0f);

    //do necessary matrix multiplication
    modelViewMatrix = modelViewMatrix * ratationMatrix;

    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major

    //bind with vao
    glBindVertexArray(vao_triangle);

    //draw scene
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 2, 2);
    glDrawArrays(GL_LINES, 4, 2);

    //unbind vao
    glBindVertexArray(0);
    //------------------------------------------------------------------------------------------------------------------

    //for line

    //initialize above matrices to identity
    modelViewMatrix = vmath::mat4::identity();
    modelViewProjectionMatrix = vmath::mat4::identity();
    ratationMatrix = vmath::mat4::identity();

    //do necessary transformation if ant required
    modelViewMatrix = vmath::translate(0.0f, 0.0f, -2.0f);

    ratationMatrix = vmath::rotate(angle_line, 0.0f, 1.0f, 0.0f);

    //do necessary matrix multiplication
    modelViewMatrix = modelViewMatrix * ratationMatrix;

    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major

    //bind with vao
    glBindVertexArray(vao_line);

    //draw scene
    //glLineWidth(2.0f);

    glDrawArrays(GL_LINES, 0, 2);

    //unbind vao
    glBindVertexArray(0);
    //------------------------------------------------------------------------------------------------------------------

    //for inner circle

    GLfloat innerCircleVertex[10];
    GLfloat radius_inner = 0.308;

    //initialize above matrices to identity
    modelViewMatrix = vmath::mat4::identity();
    modelViewProjectionMatrix = vmath::mat4::identity();
    ratationMatrix = vmath::mat4::identity();

    //do necessary transformation if ant required
    modelViewMatrix = vmath::translate(0.0f, -0.19f, -2.0f);

    ratationMatrix = vmath::rotate(angle_Circle, 0.0f, 1.0f, 0.0f);

    //do necessary matrix multiplication
    modelViewMatrix = modelViewMatrix * ratationMatrix;

    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
    //this is internally done  by gluOrho2D/ glOrtho

    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major

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
        //glPointSize(2.0f);
        glDrawArrays(GL_POINTS, 0, 1);
    }

    //unbind vao
    glBindVertexArray(0);

    //--------------------------------------------------------------------------------------------------------------------

   
    //unuse program
    glUseProgram(0);
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];

    angle_Tri = angle_Tri + 0.05f;
    if (angle_Tri >= 360.0f)
        angle_Tri = angle_Tri - 360.0f;

    angle_Circle = angle_Circle + 0.05f;
    if (angle_Circle >= 360.0f)
        angle_Circle = angle_Circle - 360.0f;

    angle_line = angle_line + 0.05f;
    if (angle_line >= 360.0f)
        angle_line = angle_line - 360.0f;
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
{    //safe release
    
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
