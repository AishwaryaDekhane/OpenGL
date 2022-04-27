package com.AMD.checkerboard;

//packages added by me

//packages added for PP Blue Window Code
import android.opengl.GLSurfaceView;
import android.opengl.GLES31;				//opengl 3.2 version
import javax.microedition.khronos.opengles.GL10;		//opengl extension starts desktop 1.0 , J2ME = Java to microedition
import javax.microedition.khronos.egl.EGLConfig;		//Embedded graphic Library
		
import android.content.Context; // for drawing context related
import android.graphics.Color; // for "Color" class
import android.view.MotionEvent; // for "MotionEvent"
import android.view.GestureDetector; // for GestureDetector
import android.view.GestureDetector.OnGestureListener; // for OnGestureListener
import android.view.GestureDetector.OnDoubleTapListener; // OnDoubleTapListener
import android.view.Gravity;						//for Gravity	
import android.graphics.Color;						//for Color

//packages added for ortho triangle - for OpenGL Buffers
import java.nio.ByteBuffer;		//native i/o / non-blocking i/o
import java.nio.ByteOrder;		
import java.nio.FloatBuffer;

import android.opengl.Matrix;		//for matrix math

import android.graphics.BitmapFactory;
import android.graphics.Bitmap;
import android.opengl.GLUtils;		//for TexImage2D()


public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener
{
    private GestureDetector gestureDetector;
    private final Context context;

	//declare variables for shaders
	private int vertexShaderObject;
	private int fragmentShaderObject;
	private int shaderProgramObject;		//java did'nt have GLuint/uint

	//we did'nt have address in java so pass array of one
	private int [] vao_quad = new int [1];

	//as java did'nt have address operator we pass it as array of one
	private int [] vbo_position_quad = new int [1];

	private int [] vbo_texture_quad = new int [1];

	private int mvp_Uniform;

	private int samplerUniform;

	private int [] TexImage = new int [1];

	private float[] perspectiveProjectionMatrix = new float[16];		//4*4 matrix

	private int [] Texture_Smiley = new int [1];
	private float [] quadTexture = new float [8];

	private int CheckImageWidth = 64;
	private int CheckImageHeight = 64;

	private byte [] CheckImage = new byte [CheckImageHeight * CheckImageWidth * 4];

	private float [] quadPosition1 = new float [12];
	private float [] quadPosition2 = new float [12];

    GLESView(Context drawingContext)
    {
        super(drawingContext);

		context= drawingContext;

		setEGLContextClientVersion(3);				//want 3.x

		setRenderer(this);

		setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        gestureDetector = new GestureDetector(drawingContext, this, null, false); // this means 'handler' i.e. who is going to handle

        gestureDetector.setOnDoubleTapListener(this); // this means 'handler' i.e. who is going to handle

    }
    
    // Handling 'onTouchEvent' Is The Most IMPORTANT,
    // Because It Triggers All Gesture And Tap Events
    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        // code

        int eventaction = event.getAction();
        if(!gestureDetector.onTouchEvent(event))
            super.onTouchEvent(event);
        return(true);

    }
    
    // abstract method from OnDoubleTapListener so must be implemented
    @Override
    public boolean onDoubleTap(MotionEvent e)
    {
        //setText("Double Tap");
        return(true);
    }
    
    // abstract method from OnDoubleTapListener so must be implemented
    @Override
    public boolean onDoubleTapEvent(MotionEvent e)
    {
        // Do Not Write Any code Here Because Already Written 'onDoubleTap'

        return(true);
    }
    
    // abstract method from OnDoubleTapListener so must be implemented
    @Override
    public boolean onSingleTapConfirmed(MotionEvent e)
    {
        //setText("Single Tap");
		
        return(true);
    }
    
    // abstract method from OnGestureListener so must be implemented
    @Override
    public boolean onDown(MotionEvent e)
    {

        // Do Not Write Any code Here Because Already Written 'onSingleTapConfirmed'
        return(true);
    }
    
    // abstract method from OnGestureListener so must be implemented
    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY)
    {

        return(true);
    }
    
    // abstract method from OnGestureListener so must be implemented
    @Override
    public void onLongPress(MotionEvent e)
    {

        //setText("Long Press");
    }
    
    // abstract method from OnGestureListener so must be implemented
    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY)
    {
       // setText("Scroll");
		uninitialize();
	   System.exit(0);
		return(true);
    }
    
    // abstract method from OnGestureListener so must be implemented
    @Override
    public void onShowPress(MotionEvent e)
    {
    }
    
    // abstract method from OnGestureListener so must be implemented
    @Override
    public boolean onSingleTapUp(MotionEvent e)
    {
        return(true);
    }

	// implement GLSurface.viewRender methods
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {

        String version = gl.glGetString(GL10.GL_VERSION);
		String shadingLanguageVersion = gl.glGetString(GLES31.GL_SHADING_LANGUAGE_VERSION);

		System.out.println("RTR: version"+version);
		System.out.println("RTR: ShadingLanguageVersion: "+shadingLanguageVersion);
		initialize();
    }


    @Override
    public void onSurfaceChanged(GL10 unused, int width, int height)
    {
        resize(width, height);
    }


    @Override
    public void onDrawFrame(GL10 unused)
    {
        display();
    }


	//our custom methods
	private void initialize()
	{
		//function declaration
		//private int load_Textures(int);

		//VERTEX SHADER
		vertexShaderObject = GLES31.glCreateShader(GLES31.GL_VERTEX_SHADER);

		final String vertexShaderSourceCode = String.format
		(
			"#version 310 es" +
			"\n" +
			"in vec4 vPosition;" +
			"in vec2 vTexture0_Coord;" +
			"out vec2 out_texture0_coord;" +
			"uniform mat4 u_mvp_matrix;" +
			"void main(void)" +
			"{" +
			"gl_Position = u_mvp_matrix * vPosition;" +
			"out_texture0_coord = vTexture0_Coord;" +
			"}"
		);

		//this line internally calls NDK functions
		//specify above source code to vertex shader object
		GLES31.glShaderSource(vertexShaderObject,vertexShaderSourceCode);

		//compile the vertex shader
		GLES31.glCompileShader(vertexShaderObject);

			/***Steps For Error Checking***/
		/*
		1.	Call glGetShaderiv(), and get the compile status of that object.
		2.	check that compile status, if it is GL_FALSE then shader has compilation error.
		3.	if(GL_FALSE) call again the glGetShaderiv() function and get the
		infoLogLength.
		4.	if(infoLogLength > 0) then call glGetShaderInfoLog() function to get the error
		information.
		5.	Print that obtained logs in file.
		*/

		//Error checking for VS

		int [] iShaderCompileStatus = new int [1];
		int [] iInfoLogLength = new int [1];
		String szInfoLog = null;

		//step 1
		GLES31.glGetShaderiv(vertexShaderObject, GLES31.GL_COMPILE_STATUS, iShaderCompileStatus, 0);


		//step 2
		if (iShaderCompileStatus[0] == GLES31.GL_FALSE)
		{
			GLES31.glGetShaderiv(vertexShaderObject, GLES31.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);

			if (iInfoLogLength[0] > 0)
			{
				szInfoLog = GLES31.glGetShaderInfoLog(vertexShaderObject);

				System.out.println("RTR: Vertex Shader Compilation Log" + szInfoLog);
			
				uninitialize();
				
				System.exit(0);
			}
		}


		//FRAGMENT SHADER

		iShaderCompileStatus[0] = 0;
		iInfoLogLength[0] = 0;
		szInfoLog = null;

		fragmentShaderObject = GLES31.glCreateShader(GLES31.GL_FRAGMENT_SHADER);

		final String fragmentShaderSourceCode = String.format
		(
			"#version 310 es" +
			"\n" +
			"precision highp float;" +
			"in vec2 out_texture0_coord;" +
			"out vec4 FragColor;" +
			"uniform highp sampler2D u_texture0_sampler;" +
			"void main(void)" +
			"{" +
			"FragColor = texture(u_texture0_sampler, out_texture0_coord);" +
			"}"
		);

		//this line internally calls NDK functions
		//specify above source code to fragment shader object
		GLES31.glShaderSource(fragmentShaderObject,fragmentShaderSourceCode);

		//compile the vertex shader
		GLES31.glCompileShader(fragmentShaderObject);

		//Error checking for FS

		//step 1
		GLES31.glGetShaderiv(fragmentShaderObject, GLES31.GL_COMPILE_STATUS, iShaderCompileStatus, 0);


		//step 2
		if (iShaderCompileStatus[0] == GLES31.GL_FALSE)
		{
			GLES31.glGetShaderiv(fragmentShaderObject, GLES31.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);

			if (iInfoLogLength[0] > 0)
			{
				szInfoLog = GLES31.glGetShaderInfoLog(fragmentShaderObject);

				System.out.println("RTR: Fragemnt Shader Compilation Log" + szInfoLog);
			
				uninitialize();
				
				System.exit(0);
			}
		}

		
		//SHADER PROGRAM

		//create obj
		shaderProgramObject = GLES31.glCreateProgram();

		//add vertex shader
		GLES31.glAttachShader(shaderProgramObject, vertexShaderObject);

		//add fragment shader
		GLES31.glAttachShader(shaderProgramObject, fragmentShaderObject);

		//Prelinking binding to vertex attributes
		GLES31.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
		GLES31.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_TEXCOORD0, "vTexture0_Coord");

		GLES31.glLinkProgram(shaderProgramObject);

		//Error checking for shader

		iInfoLogLength[0] = 0;
		szInfoLog = null;
		int[] iProgramLinkStatus = new int[1];

		//step 1
		GLES31.glGetProgramiv(shaderProgramObject, GLES31.GL_LINK_STATUS, iProgramLinkStatus, 0);

		//step 2
		if (iProgramLinkStatus[0] == GLES31.GL_FALSE)
		{
			GLES31.glGetProgramiv(shaderProgramObject, GLES31.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);

			if (iInfoLogLength[0] > 0)
			{
					szInfoLog = GLES31.glGetShaderInfoLog(fragmentShaderObject);

					System.out.println("RTR: Fragemnt Shader Compilation Log" + szInfoLog);
			
					uninitialize();
				
					System.exit(0);		
			}
		}

		//--------------------------------------------------------------------------------------

		//Postlinking retriving uniform locations
		mvp_Uniform = GLES31.glGetUniformLocation(shaderProgramObject, "u_mvp_matrix");
		samplerUniform = GLES31.glGetUniformLocation(shaderProgramObject, "u_texture0_sampler");

		//declare vertex3f coordinates here
		final float [] quadTexture = new float []	
		{ 
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
		};


		//-------------------------------------------------------

		//for quad

		//for position

		//vao = VertexArrayObject
		GLES31.glGenVertexArrays(1, vao_quad, 0);

		//bind array with vao
		GLES31.glBindVertexArray(vao_quad[0]);

		//create vbo
		GLES31.glGenBuffers(1, vbo_position_quad, 0);

		//target vbo <-> GL_ARRAY_BUFFER
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_position_quad[0]);

		//fill the buffer data statically
		//glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, 4 * 3 * 4, null, GLES31.GL_DYNAMIC_DRAW);

		//give strides to vPosition
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES31.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or

		//enable array
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp

		//unbind buffer vbo
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);



		//for texture

		//create vbo
		GLES31.glGenBuffers(1, vbo_texture_quad, 0);

		//target vbo <-> GL_ARRAY_BUFFER
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_texture_quad[0]);

		//To convert the array to such buffer which is capable of to create a buffer data array

		//step 1
		//Allowcate the buffer directly from the native memory(not from VM)

		ByteBuffer byteBufferTexture = ByteBuffer.allocateDirect(quadTexture.length * 4);	

		//step 2
		//Arrange the buffer in the byte order of buffer natively(big/little endian)
		
		byteBufferTexture.order(ByteOrder.nativeOrder());

		//step 3
		//Create the float type buffer & convert our byte type buffer to float type buffer

		FloatBuffer textureBuffer = byteBufferTexture.asFloatBuffer();

		//step 4
		//Now put your array into the cooked buffer

		textureBuffer.put(quadTexture);

		//step 5
		//Not necessary but useful for large array -> interleaved array.
		//Set the array at the 0th position

		textureBuffer.position(0);

		//fill the buffer data statically
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, quadTexture.length * 4, textureBuffer, GLES31.GL_STATIC_DRAW);


		//give strides to vPosition
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_TEXCOORD0, 2, GLES31.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or

		//enable array
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_TEXCOORD0);	//AMC = AstroMediComp

		//unbind buffer
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);



		//unbind array
		GLES31.glBindVertexArray(0);


	//-------------------------------------------------------------------------------------------------------------------------------------------

		//for background color
		GLES31.glClearColor(0.0f,0.0f,0.0f,0.0f);

		//For Depth

		//GLES31.glClearDepth(1.0f);	//give existance to depth buffer

		GLES31.glEnable(GLES31.GL_DEPTH_TEST);	//for 3D geometry enable depth test i.e z axis 

		GLES31.glDepthFunc(GLES31.GL_LEQUAL);		//LEQUAL = less than or equal to test compared to the max value which is set to 1.0f

		//identity() is in mat4
		Matrix.setIdentityM(perspectiveProjectionMatrix, 0);

		//no warmup call to resize as we are already inn fullscreen

		//for texture
		//GLES31.glEnable(GLES31.GL_CULL_FACE);

		GLES31.glEnable(GLES31.GL_TEXTURE_2D);

		load_Textures();
	}

	private void resize(int width, int height)
	{
		//code
		if (height <= 0)
		{
			height = 1;
		}
		
		GLES31.glViewport(0, 0, width, height);


	//perspectiveM(float[] m, int offset, float fovy, float aspect, float zNear, float zFar)
	//Defines a projection matrix in terms of a field of view angle, an aspect ratio, and z clip planes.

		Matrix.perspectiveM(perspectiveProjectionMatrix, 0 , 45.0f, ((float)width /(float) height), 0.1f, 100.0f);
	}

	private void MakeCheckImage()
	{
		int i, j, c;

		for (i = 0;i < CheckImageHeight;i++)
		{
			for (j = 0;j < CheckImageWidth;j++)
			{
				c = ((i & 8) ^ (j & 8)) * 255;
				CheckImage[(i * 64 + j) * 4 + 0] = (byte)c;
				CheckImage[(i * 64 + j) * 4 + 1] = (byte)c;
				CheckImage[(i * 64 + j) * 4 + 2] = (byte)c;
				CheckImage[(i * 64 + j) * 4 + 3] = (byte)0xff;
			}
		}
	}

	/*for (i = 0;i < CHECK_IMAGE_HEIGHT;i++)
		{
			for (j = 0;j < CHECK_IMAGE_WIDTH;j++)
			{
				c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0)) * 255;

				Check_Image[i][j][0] = (GLubyte)c;
				Check_Image[i][j][1] = (GLubyte)c;
				Check_Image[i][j][2] = (GLubyte)c;
				Check_Image[i][j][3] = (GLubyte)255;
			}
		}*/

	private int load_Textures()
	{
		//fuction call
		MakeCheckImage();

		int [] texture = new int [1];

		//step 1
		ByteBuffer byteBuffer_check_image = ByteBuffer.allocateDirect(CheckImageWidth * CheckImageHeight * 4);

		//step2
		byteBuffer_check_image.order(ByteOrder.nativeOrder());

		//step 4
		byteBuffer_check_image.put(CheckImage);

		//step 5
		byteBuffer_check_image.position(0);


		Bitmap bitmap = Bitmap.createBitmap(CheckImageWidth,CheckImageHeight,Bitmap.Config.ARGB_8888);
		bitmap.copyPixelsFromBuffer(byteBuffer_check_image);

		GLES31.glPixelStorei(GLES31.GL_UNPACK_ALIGNMENT, 4);	//4=RGBA

		GLES31.glGenTextures(1, TexImage, 0);

		GLES31.glBindTexture(GLES31.GL_TEXTURE_2D, TexImage[0]);

		//mipmap = to reduce load time and memory of image we create multiple images of given render - optimization
		//GL_LINEAR is used for good quality it calculates average and then map out texel center to the pixel center
		GLES31.glTexParameteri(GLES31.GL_TEXTURE_2D, GLES31.GL_TEXTURE_WRAP_S, GLES31.GL_REPEAT);
		GLES31.glTexParameteri(GLES31.GL_TEXTURE_2D, GLES31.GL_TEXTURE_WRAP_T, GLES31.GL_REPEAT);

		GLES31.glTexParameteri(GLES31.GL_TEXTURE_2D, GLES31.GL_TEXTURE_MAG_FILTER, GLES31.GL_NEAREST);	//MAG= magnification of image

		GLES31.glTexParameteri(GLES31.GL_TEXTURE_2D, GLES31.GL_TEXTURE_MIN_FILTER, GLES31.GL_NEAREST);	//MIN= minification of image

		//instead of gluBuild2DMipmaps we use these two below fuctions
		GLUtils.texImage2D(GLES31.GL_TEXTURE_2D,			//target
			0,								//mipmap level
			bitmap,
			0);								//border width

		GLES31.glGenerateMipmap(GLES31.GL_TEXTURE_2D);

		//unbind
		GLES31.glBindTexture(GLES31.GL_TEXTURE_2D, 0);

		return(texture[0]);
	}

	private void display()
	{
		GLES31.glClear(GLES31.GL_COLOR_BUFFER_BIT |GLES31.GL_DEPTH_BUFFER_BIT);

		//binding
		GLES31.glUseProgram(shaderProgramObject);

		// OpenGL Drawing
		//declaration of matrices
		float [] modelViewMatrix = new float[16];
		float [] modelViewProjectionMatrix = new float[16];
		

		//for triangle

		//initialize above matrices to identity
		Matrix.setIdentityM(modelViewMatrix,0);
		Matrix.setIdentityM(modelViewProjectionMatrix,0);
	

		//do necessary transformation if ant required
		//translateM(float[] m, int mOffset, float x, float y, float z)
		//Translates matrix m by x, y, and z in place.
		//translationMatrix = translate(-1.5f, 0.0f, -6.0f);
		Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -3.0f);

		//do necessary matrix multiplication
		//modelViewMatrix = translationMatrix * ratationMatrix;
		//modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix			  

		Matrix.multiplyMM(modelViewProjectionMatrix,0,
						  perspectiveProjectionMatrix,0,
						  modelViewMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES31.glUniformMatrix4fv(mvp_Uniform, 1, false, modelViewProjectionMatrix, 0);
		//GL_FALSE = internally we are not transposing = row -> col / col -> row
		//OpenGL is col major while DirectX is row major

		//for texture
		GLES31.glActiveTexture(GLES31.GL_TEXTURE0);
		GLES31.glBindTexture(GLES31.GL_TEXTURE_2D, TexImage[0]);
		GLES31.glUniform1i(samplerUniform, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		GLES31.glBindVertexArray(vao_quad[0]);


		//similarly bind the textures if any


		//draw necessary scene

		quadPosition1[0] = -2.0f;
		quadPosition1[1] = -1.0f;
		quadPosition1[2] = 0.0f;
		quadPosition1[3] = -2.0f;
		quadPosition1[4] = 1.0f;
		quadPosition1[5] = 0.0f;
		quadPosition1[6] = 0.0f;
		quadPosition1[7] = 1.0f;
		quadPosition1[8] = 0.0f;
		quadPosition1[9] = 0.0f;
		quadPosition1[10] = -1.0f;
		quadPosition1[11] = 0.0f;

	

		//target vbo <-> GL_ARRAY_BUFFER
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_position_quad[0]);

		//To convert the array to such buffer which is capable of to create a buffer data array

		//step 1
		//Allowcate the buffer directly from the native memory(not from VM)

		ByteBuffer byteBufferPosition1 = ByteBuffer.allocateDirect(quadPosition1.length * 4);	

		//step 2
		//Arrange the buffer in the byte order of buffer natively(big/little endian)
		
		byteBufferPosition1.order(ByteOrder.nativeOrder());

		//step 3
		//Create the float type buffer & convert our byte type buffer to float type buffer

		FloatBuffer positionBuffer1 = byteBufferPosition1.asFloatBuffer();

		//step 4
		//Now put your array into the cooked buffer

		positionBuffer1.put(quadPosition1);

		//step 5
		//Not necessary but useful for large array -> interleaved array.
		//Set the array at the 0th position

		positionBuffer1.position(0);

		//fill the buffer data statically
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER,
			quadPosition1.length * 4,		//quad- 4 coord, texccord - 2 coord, float
			positionBuffer1,							//no array
			GLES31.GL_DYNAMIC_DRAW);				//draw dynamically

		//give strides to vPosition
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES31.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or

		//enable array
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp

		
		//similarly bind the textures if any

		//draw necessary scene
		GLES31.glDrawArrays(GLES31.GL_TRIANGLE_FAN, 0, 4);
		// glDrawArrays(GLenum, GLint , GLsizei)
		// (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

		//unbind buffer
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);


		//unbind va0
		GLES31.glBindVertexArray(0);
	
	//----------------------------------------------------------------------------------------------------------------

		
		

		//bind with vao(this will avoid many repetitive binding with vbo)
		GLES31.glBindVertexArray(vao_quad[0]);

		//target vbo <-> GL_ARRAY_BUFFER
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_position_quad[0]);

		//To convert the array to such buffer which is capable of to create a buffer data array

		quadPosition2[0] = 1.0f;
		quadPosition2[1] = -1.0f;
		quadPosition2[2] = 0.0f;
		quadPosition2[3] = 1.0f;
		quadPosition2[4] = 1.0f;
		quadPosition2[5] = 0.0f;
		quadPosition2[6] = 2.41421f;
		quadPosition2[7] = 1.0f;
		quadPosition2[8] = -1.41421f;
		quadPosition2[9] = 2.41421f;
		quadPosition2[10] = -1.0f;
		quadPosition2[11] = -1.41421f;

		//step 1
		//Allowcate the buffer directly from the native memory(not from VM)

		ByteBuffer byteBufferPosition2 = ByteBuffer.allocateDirect(quadPosition2.length * 4);	

		//step 2
		//Arrange the buffer in the byte order of buffer natively(big/little endian)
		
		byteBufferPosition2.order(ByteOrder.nativeOrder());

		//step 3
		//Create the float type buffer & convert our byte type buffer to float type buffer

		FloatBuffer positionBuffer2 = byteBufferPosition2.asFloatBuffer();

		//step 4
		//Now put your array into the cooked buffer

		positionBuffer2.put(quadPosition2);

		//step 5
		//Not necessary but useful for large array -> interleaved array.
		//Set the array at the 0th position

		positionBuffer2.position(0);

		//fill the buffer data statically
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER,
			quadPosition1.length * 4,		//quad- 4 coord, texccord - 2 coord, float
			positionBuffer2,							//no array
			GLES31.GL_DYNAMIC_DRAW);				//draw dynamically

		//give strides to vPosition
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES31.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or

		//enable array
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp


		//similarly bind the textures if any

		//draw necessary scene
		GLES31.glDrawArrays(GLES31.GL_TRIANGLE_FAN, 0, 4);
		// glDrawArrays(GLenum, GLint , GLsizei)
		// (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

		//unbind buffer
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);


		//unbind va0
		GLES31.glBindVertexArray(0);

		
		//----------------------------------------------------------------------------------------------------------------

		//unbinding - unused program
		GLES31.glUseProgram(0);

		requestRender();		//SwapBuffers()

}

void uninitialize()
{
	//code

	if (vao_quad[0] != 0)
	{
		GLES31.glDeleteBuffers(1, vao_quad, 0);
		vao_quad[0] = 0;
	}

	if (vbo_position_quad[0] != 0)
	{
		GLES31.glDeleteBuffers(1, vbo_position_quad, 0);
		vbo_position_quad[0] = 0;
	}

	if (vbo_texture_quad[0] != 0)
	{
		GLES31.glDeleteBuffers(1, vbo_texture_quad, 0);
		vbo_texture_quad[0] = 0;
	}

	if (Texture_Smiley[0] != 0)
	{
		GLES31.glDeleteTextures(1, Texture_Smiley, 0);
		Texture_Smiley[0] = 0;
	}

	//safe release

	if(shaderProgramObject != 0)
		{
			int[] shaderCount = new int[1];
			int shaderNumber;

			GLES31.glUseProgram(shaderProgramObject);

			//ask program how many shaders are attached
			GLES31.glGetProgramiv(	shaderProgramObject,
									GLES31.GL_ATTACHED_SHADERS,
									shaderCount, 0);

			int[] shaders = new int[shaderCount[0]];

			if(shaderCount[0] != 0)
			{
				GLES31.glGetAttachedShaders(	shaderProgramObject,
												shaderCount[0],
												shaderCount, 0,
												shaders, 0);

				for(shaderNumber = 0; shaderNumber < shaderCount[0]; shaderNumber++)
				{
					//detach shaders
					GLES31.glDetachShader(	shaderProgramObject,
											shaders[shaderNumber]);

					//delete shaders
					GLES31.glDeleteShader(shaders[shaderNumber]);

					shaders[shaderNumber] = 0;
				}
			} 
			GLES31.glDeleteProgram(shaderProgramObject);
			shaderProgramObject = 0;
			GLES31.glUseProgram(0);
		}
}
}
