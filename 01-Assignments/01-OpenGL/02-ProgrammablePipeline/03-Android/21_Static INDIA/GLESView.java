package com.AMD.static_india;

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

import java.lang.Math;

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
	private int [] vao_first_I = new int [1];
	private int [] vbo_first_I_pos = new int [1];
	private int [] vbo_first_I_col = new int [1];

	private int [] vao_N = new int [1];
	private int [] vbo_N_pos = new int [1];
	private int [] vbo_N_col = new int [1];

	private int [] vao_D = new int [1];
	private int [] vbo_D_pos = new int [1];
	private int [] vbo_D_col = new int [1];

	private int [] vao_second_I = new int [1];
	private int [] vbo_second_I_pos = new int [1];
	private int [] vbo_second_I_col = new int [1];

	private int [] vao_A = new int [1];
	private int [] vbo_A_pos = new int [1];
	private int [] vbo_A_col = new int [1];

	
	private int mvp_Uniform;

	private int samplerUniform;

	private float[] perspectiveProjectionMatrix = new float[16];		//4*4 matrix


    GLESView(Context drawingContext)
    {
        super(drawingContext);

		context= drawingContext;

		setEGLContextClientVersion(3);				//want 3.x

		setRenderer(this);

		setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

     //   gestureDetector = new GestureDetector(drawingContext, this, null, false); // this means 'handler' i.e. who is going to handle

     //   gestureDetector.setOnDoubleTapListener(this); // this means 'handler' i.e. who is going to handle

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
		//update();
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
			"in vec4 vColor;" +
			"out vec4 out_color;" +
			"uniform mat4 u_mvp_matrix;" +
			"void main(void)" +
			"{" +
			"gl_Position = u_mvp_matrix * vPosition;" +
			"out_color = vColor;" +
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
			"out vec4 FragColor;" +
			"in vec4 out_color;" +
			"void main(void)" +
			"{" +
			"FragColor = out_color;" +
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
		GLES31.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_COLOR, "vColor");

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

		
	//------------------------------------------------------------------------------------------------------------------------------------------
	
	initializeD();
	initializeFirstI();
	initializeN();
	initializeSecondI();
	initializeA();
	
	//-------------------------------------------------------------------------------------------------------------



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

	}

	private void initializeFirstI()
	{
	final float [] first_I_pos = new float []	
	{
		-0.56f, 0.5f, 0.0f,
		-0.56f, -0.5f, 0.0f,
		-0.65f, 0.5f, 0.0f,
		-0.47f, 0.5f, 0.0f,
		-0.65f, -0.5f, 0.0f,
		-0.47f, -0.5f, 0.0f,
	};

	final float [] first_I_col = new float []
	{
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		0.0705882353f, 0.53333f, 0.0274509804f,
	};

	//for first I

		//for position

		//vao = VertexArrayObject
		GLES31.glGenVertexArrays(1, vao_first_I, 0);

		//bind array with vao
		GLES31.glBindVertexArray(vao_first_I[0]);

		//create vbo
		GLES31.glGenBuffers(1, vbo_first_I_pos, 0);

		//target vbo <-> GL_ARRAY_BUFFER
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_first_I_pos[0]);

		//To convert the array to such buffer which is capable of to create a buffer data array

		//step 1
		//Allowcate the buffer directly from the native memory(not from VM)

		ByteBuffer byteBufferFIPos = ByteBuffer.allocateDirect(first_I_pos.length * 4);	

		//step 2
		//Arrange the buffer in the byte order of buffer natively(big/little endian)
		
		byteBufferFIPos.order(ByteOrder.nativeOrder());

		//step 3
		//Create the float type buffer & convert our byte type buffer to float type buffer

		FloatBuffer positionBufferFI = byteBufferFIPos.asFloatBuffer();

		//step 4
		//Now put your array into the cooked buffer

		positionBufferFI.put(first_I_pos);

		//step 5
		//Not necessary but useful for large array -> interleaved array.
		//Set the array at the 0th position

		positionBufferFI.position(0);

		//fill the buffer data statically
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, first_I_pos.length * 4, positionBufferFI, GLES31.GL_STATIC_DRAW);

		//give strides to vPosition
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES31.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or

		//enable array
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp

		//unbind buffer vbo
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);



		//for color

		//create vbo
		GLES31.glGenBuffers(1, vbo_first_I_col, 0);

		//target vbo <-> GL_ARRAY_BUFFER
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_first_I_col[0]);

		//To convert the array to such buffer which is capable of to create a buffer data array

		//step 1
		//Allowcate the buffer directly from the native memory(not from VM)

		ByteBuffer byteBufferFIColor = ByteBuffer.allocateDirect(first_I_col.length * 4);	

		//step 2
		//Arrange the buffer in the byte order of buffer natively(big/little endian)
		
		byteBufferFIColor.order(ByteOrder.nativeOrder());

		//step 3
		//Create the float type buffer & convert our byte type buffer to float type buffer

		FloatBuffer colorBufferFI = byteBufferFIColor.asFloatBuffer();

		//step 4
		//Now put your array into the cooked buffer

		colorBufferFI.put(first_I_col);

		//step 5
		//Not necessary but useful for large array -> interleaved array.
		//Set the array at the 0th position

		colorBufferFI.position(0);

		//fill the buffer data statically
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, first_I_col.length * 4, colorBufferFI, GLES31.GL_STATIC_DRAW);

		//give strides to vPosition
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES31.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or

		//enable array
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);	//AMC = AstroMediComp

		//unbind buffer
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);

		//unbind array
		GLES31.glBindVertexArray(0);
	}

	private void initializeN()
	{
		final float [] lineNpos = new float []
		{
			-0.37f, 0.5f, 0.0f,
		-0.37f, -0.5f, 0.0f,
		-0.19f, 0.5f, 0.0f,
		-0.19f, -0.5f, 0.0f,
		-0.37f, 0.5f, 0.0f,
		-0.19f, -0.5f, 0.0f,
		};
	
		final float [] lineNCol = new float []
		{
			1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		};

		//for line

		//for position

		//vao = VertexArrayObject
		GLES31.glGenVertexArrays(1, vao_N, 0);

		//bind array with vao
		GLES31.glBindVertexArray (vao_N[0]);

		//create vbo
		GLES31.glGenBuffers(1, vbo_N_pos, 0);

		//target vbo <-> GL_ARRAY_BUFFER
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_N_pos[0]);

		//To convert the array to such buffer which is capable of to create a buffer data array

		//step 1
		//Allowcate the buffer directly from the native memory(not from VM)

		ByteBuffer byteBufferRecPos = ByteBuffer.allocateDirect(lineNpos.length * 4);	

		//step 2
		//Arrange the buffer in the byte order of buffer natively(big/little endian)
		
		byteBufferRecPos.order(ByteOrder.nativeOrder());

		//step 3
		//Create the float type buffer & convert our byte type buffer to float type buffer

		FloatBuffer positionBufferRec = byteBufferRecPos.asFloatBuffer();

		//step 4
		//Now put your array into the cooked buffer

		positionBufferRec.put(lineNpos);

		//step 5
		//Not necessary but useful for large array -> interleaved array.
		//Set the array at the 0th position

		positionBufferRec.position(0);

		//fill the buffer data statically
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, lineNpos.length * 4, positionBufferRec, GLES31.GL_STATIC_DRAW);

		//give strides to vPosition
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES31.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or

		//enable array
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp

		//unbind buffer vbo
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);



		//for color

		//create vbo
		GLES31.glGenBuffers(1, vbo_N_col, 0);

		//target vbo <-> GL_ARRAY_BUFFER
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_N_col[0]);

		//To convert the array to such buffer which is capable of to create a buffer data array

		//step 1
		//Allowcate the buffer directly from the native memory(not from VM)

		ByteBuffer byteBufferRecColor = ByteBuffer.allocateDirect(lineNCol.length * 4);	

		//step 2
		//Arrange the buffer in the byte order of buffer natively(big/little endian)
		
		byteBufferRecColor.order(ByteOrder.nativeOrder());

		//step 3
		//Create the float type buffer & convert our byte type buffer to float type buffer

		FloatBuffer colorBufferRec = byteBufferRecColor.asFloatBuffer();

		//step 4
		//Now put your array into the cooked buffer

		colorBufferRec.put(lineNCol);

		//step 5
		//Not necessary but useful for large array -> interleaved array.
		//Set the array at the 0th position

		colorBufferRec.position(0);

		//fill the buffer data statically
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, lineNCol.length * 4, colorBufferRec, GLES31.GL_STATIC_DRAW);

		//give strides to vPosition
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES31.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or

		//enable array
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);	//AMC = AstroMediComp

		//unbind buffer
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);

		//unbind array
		GLES31.glBindVertexArray(0);
	}

	private void initializeD()
	{
		final float [] Dpos = new float []
		{
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
	
		final float [] DCol = new float []
		{
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

		//for D line

		//for position
		GLES31.glGenVertexArrays(1, vao_D, 0);
		GLES31.glBindVertexArray(vao_D[0]);

		GLES31.glGenBuffers(1, vbo_D_pos, 0);
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_D_pos[0]);
		ByteBuffer byteBufferICPos = ByteBuffer.allocateDirect(Dpos.length * 4);
		byteBufferICPos.order(ByteOrder.nativeOrder());
		FloatBuffer positionBufferIC = byteBufferICPos.asFloatBuffer();
		positionBufferIC.put(Dpos);
		positionBufferIC.position(0);
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, Dpos.length * 4, positionBufferIC, GLES31.GL_STATIC_DRAW);
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES31.GL_FLOAT, false, 0, 0);
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);
		
		//for color
		GLES31.glGenBuffers(1, vbo_D_col, 0);
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_D_col[0]);
		ByteBuffer byteBufferICColor = ByteBuffer.allocateDirect(DCol.length * 4);	
		byteBufferICColor.order(ByteOrder.nativeOrder());
		FloatBuffer colorBufferIC = byteBufferICColor.asFloatBuffer();
		colorBufferIC.put(DCol);
		colorBufferIC.position(0);
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, DCol.length * 4, colorBufferIC, GLES31.GL_STATIC_DRAW);
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES31.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);	//AMC = AstroMediComp
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);
		GLES31.glBindVertexArray(0);
	}

	private void initializeSecondI()
	{
		final float [] second_I_pos = new float []
		{
			0.28f, 0.5f, 0.0f,
		0.28f, -0.5f, 0.0f,
		0.19f, 0.5f, 0.0f,
		0.37f, 0.5f, 0.0f,
		0.19f, -0.5f, 0.0f,
		0.37f, -0.5f, 0.0f,
		};
	
		final float [] second_I_col = new float []
		{
			1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		1.0f, 0.6f, 0.2f,
		1.0f, 0.6f, 0.2f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		0.0705882353f, 0.53333f, 0.0274509804f,
		};

		//for I 2nd line

		//for position
		GLES31.glGenVertexArrays(1, vao_second_I, 0);
		GLES31.glBindVertexArray(vao_second_I[0]);

		GLES31.glGenBuffers(1, vbo_second_I_pos, 0);
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_second_I_pos[0]);
		ByteBuffer byteBufferISPos = ByteBuffer.allocateDirect(second_I_pos.length * 4);
		byteBufferISPos.order(ByteOrder.nativeOrder());
		FloatBuffer positionBufferIS = byteBufferISPos.asFloatBuffer();
		positionBufferIS.put(second_I_pos);
		positionBufferIS.position(0);
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, second_I_pos.length * 4, positionBufferIS, GLES31.GL_STATIC_DRAW);
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES31.GL_FLOAT, false, 0, 0);
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);
		
		//for color
		GLES31.glGenBuffers(1, vbo_second_I_col, 0);
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_second_I_col[0]);
		ByteBuffer byteBufferISColor = ByteBuffer.allocateDirect(second_I_col.length * 4);	
		byteBufferISColor.order(ByteOrder.nativeOrder());
		FloatBuffer colorBufferIS = byteBufferISColor.asFloatBuffer();
		colorBufferIS.put(second_I_col);
		colorBufferIS.position(0);
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, second_I_col.length * 4, colorBufferIS, GLES31.GL_STATIC_DRAW);
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES31.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);	//AMC = AstroMediComp
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);
		GLES31.glBindVertexArray(0);
	}

	private void initializeA()
	{
		final float [] A_pos = new float []
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
	
		final float [] A_col = new float []
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

		//for A line

		//for position
		GLES31.glGenVertexArrays(1, vao_A, 0);
		GLES31.glBindVertexArray(vao_A[0]);

		GLES31.glGenBuffers(1, vbo_A_pos, 0);
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_A_pos[0]);
		ByteBuffer byteBufferAPos = ByteBuffer.allocateDirect(A_pos.length * 4);
		byteBufferAPos.order(ByteOrder.nativeOrder());
		FloatBuffer positionBufferA = byteBufferAPos.asFloatBuffer();
		positionBufferA.put(A_pos);
		positionBufferA.position(0);
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, A_pos.length * 4, positionBufferA, GLES31.GL_STATIC_DRAW);
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES31.GL_FLOAT, false, 0, 0);
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);
		
		//for color
		GLES31.glGenBuffers(1, vbo_A_col, 0);
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_A_col[0]);
		ByteBuffer byteBufferAColor = ByteBuffer.allocateDirect(A_col.length * 4);	
		byteBufferAColor.order(ByteOrder.nativeOrder());
		FloatBuffer colorBufferA = byteBufferAColor.asFloatBuffer();
		colorBufferA.put(A_col);
		colorBufferA.position(0);
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, A_col.length * 4, colorBufferA, GLES31.GL_STATIC_DRAW);
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES31.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);	//AMC = AstroMediComp
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);
		GLES31.glBindVertexArray(0);
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

	private void display()
	{

		GLES31.glClear(GLES31.GL_COLOR_BUFFER_BIT |GLES31.GL_DEPTH_BUFFER_BIT);

		//binding
		GLES31.glUseProgram(shaderProgramObject);

		// OpenGL Drawing
		//declaration of matrices
		float [] modelViewMatrix = new float[16];
		float [] modelViewProjectionMatrix = new float[16];
		float [] translationMatrix = new float[16];
		float [] rotationMatrix = new float[16];

		//--------------------------------------------------------------------------------------------------------------
		//for first I

		//initialize above matrices to identity
		Matrix.setIdentityM(modelViewMatrix,0);
		Matrix.setIdentityM(modelViewProjectionMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		//do necessary transformation if ant required
		//translateM(float[] m, int mOffset, float x, float y, float z)
		//Translates matrix m by x, y, and z in place.
		//translationMatrix = translate(-1.5f, 0.0f, -6.0f);
		Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -2.0f);

		//ratationMatrix = rotate(angle_tri, 0.0f, 1.0f, 0.0f);
		//static void	setRotateM(float[] rm, int rmOffset, float a, float x, float y, float z)
		//Creates a matrix for rotation by angle a (in degrees) around the axis (x, y, z).
		//Matrix.setRotateM(rotationMatrix , 0, angle_Tri, 0.0f, 1.0f, 0.0f);

		//do necessary matrix multiplication
		//modelViewMatrix = translationMatrix * ratationMatrix;
		//modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix

		Matrix.multiplyMM(modelViewMatrix,0,
						  modelViewMatrix,0,
						  rotationMatrix,0);

		Matrix.multiplyMM(modelViewMatrix,0,
						  modelViewMatrix,0,
						  translationMatrix,0);				  

		Matrix.multiplyMM(modelViewProjectionMatrix,0,
						  perspectiveProjectionMatrix,0,
						  modelViewMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES31.glUniformMatrix4fv(mvp_Uniform, 1, false, modelViewProjectionMatrix, 0);
		//GL_FALSE = internally we are not transposing = row -> col / col -> row
		//OpenGL is col major while DirectX is row major

		//bind with vao(this will avoid many repetitive binding with vbo)
		GLES31.glBindVertexArray(vao_first_I[0]);

		//similarly bind the textures if any
		
		GLES31.glLineWidth(20.0f);
		//draw necessary scene
		GLES31.glDrawArrays(GLES31.GL_LINES, 0, 2);
		GLES31.glDrawArrays(GLES31.GL_LINES, 2, 2);
		GLES31.glDrawArrays(GLES31.GL_LINES, 4, 2);
		
		//unbind va0
		GLES31.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------
			//for N

		//initialize above matrices to identity
		Matrix.setIdentityM(modelViewMatrix,0);
		Matrix.setIdentityM(modelViewProjectionMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		//do necessary transformation if ant required
		//translateM(float[] m, int mOffset, float x, float y, float z)
		//Translates matrix m by x, y, and z in place.
		//translationMatrix = translate(-1.5f, 0.0f, -6.0f);
		Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -2.0f);

		//ratationMatrix = rotate(angle_tri, 0.0f, 1.0f, 0.0f);
		//static void	setRotateM(float[] rm, int rmOffset, float a, float x, float y, float z)
		//Creates a matrix for rotation by angle a (in degrees) around the axis (x, y, z).
		//Matrix.setRotateM(rotationMatrix , 0, angle_Tri, 0.0f, 1.0f, 0.0f);

		//do necessary matrix multiplication
		//modelViewMatrix = translationMatrix * ratationMatrix;
		//modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix

		Matrix.multiplyMM(modelViewMatrix,0,
						  modelViewMatrix,0,
						  rotationMatrix,0);

		Matrix.multiplyMM(modelViewMatrix,0,
						  modelViewMatrix,0,
						  translationMatrix,0);				  

		Matrix.multiplyMM(modelViewProjectionMatrix,0,
						  perspectiveProjectionMatrix,0,
						  modelViewMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES31.glUniformMatrix4fv(mvp_Uniform, 1, false, modelViewProjectionMatrix, 0);
		//GL_FALSE = internally we are not transposing = row -> col / col -> row
		//OpenGL is col major while DirectX is row major

		//bind with vao(this will avoid many repetitive binding with vbo)
		GLES31.glBindVertexArray(vao_N[0]);

		//similarly bind the textures if any

		//draw necessary scene
		GLES31.glDrawArrays(GLES31.GL_LINES, 0, 2);
		GLES31.glDrawArrays(GLES31.GL_LINES, 2, 2);
		GLES31.glDrawArrays(GLES31.GL_LINES, 4, 2);
		
		//unbind va0
		GLES31.glBindVertexArray(0);
	
		//----------------------------------------------------------------------------------------------------------------
			//for D

		//initialize above matrices to identity
		Matrix.setIdentityM(modelViewMatrix,0);
		Matrix.setIdentityM(modelViewProjectionMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		//do necessary transformation if ant required
		//translateM(float[] m, int mOffset, float x, float y, float z)
		//Translates matrix m by x, y, and z in place.
		//translationMatrix = translate(-1.5f, 0.0f, -6.0f);
		Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -2.0f);

		//ratationMatrix = rotate(angle_tri, 0.0f, 1.0f, 0.0f);
		//static void	setRotateM(float[] rm, int rmOffset, float a, float x, float y, float z)
		//Creates a matrix for rotation by angle a (in degrees) around the axis (x, y, z).
		//Matrix.setRotateM(rotationMatrix , 0, angle_Tri, 0.0f, 1.0f, 0.0f);

		//do necessary matrix multiplication
		//modelViewMatrix = translationMatrix * ratationMatrix;
		//modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix

		Matrix.multiplyMM(modelViewMatrix,0,
						  modelViewMatrix,0,
						  rotationMatrix,0);

		Matrix.multiplyMM(modelViewMatrix,0,
						  modelViewMatrix,0,
						  translationMatrix,0);				  

		Matrix.multiplyMM(modelViewProjectionMatrix,0,
						  perspectiveProjectionMatrix,0,
						  modelViewMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES31.glUniformMatrix4fv(mvp_Uniform, 1, false, modelViewProjectionMatrix, 0);
		//GL_FALSE = internally we are not transposing = row -> col / col -> row
		//OpenGL is col major while DirectX is row major

		//bind with vao(this will avoid many repetitive binding with vbo)
		GLES31.glBindVertexArray(vao_D[0]);

		//similarly bind the textures if any

		//draw necessary scene
		GLES31.glDrawArrays(GLES31.GL_LINES, 0, 2);
		GLES31.glDrawArrays(GLES31.GL_LINES, 2, 2);
		GLES31.glDrawArrays(GLES31.GL_LINES, 4, 2);
		GLES31.glDrawArrays(GLES31.GL_LINES, 6, 2);
		GLES31.glDrawArrays(GLES31.GL_LINES, 8, 2);
		GLES31.glDrawArrays(GLES31.GL_LINES, 10, 2);
		
		//unbind va0
		GLES31.glBindVertexArray(0);
	//----------------------------------------------------------------------------------------------------------------------------------
			//for 2ND I

		//initialize above matrices to identity
		Matrix.setIdentityM(modelViewMatrix,0);
		Matrix.setIdentityM(modelViewProjectionMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		//do necessary transformation if ant required
		//translateM(float[] m, int mOffset, float x, float y, float z)
		//Translates matrix m by x, y, and z in place.
		//translationMatrix = translate(-1.5f, 0.0f, -6.0f);
		Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -2.0f);

		//ratationMatrix = rotate(angle_tri, 0.0f, 1.0f, 0.0f);
		//static void	setRotateM(float[] rm, int rmOffset, float a, float x, float y, float z)
		//Creates a matrix for rotation by angle a (in degrees) around the axis (x, y, z).
		//Matrix.setRotateM(rotationMatrix , 0, angle_Tri, 0.0f, 1.0f, 0.0f);

		//do necessary matrix multiplication
		//modelViewMatrix = translationMatrix * ratationMatrix;
		//modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix

		Matrix.multiplyMM(modelViewMatrix,0,
						  modelViewMatrix,0,
						  rotationMatrix,0);

		Matrix.multiplyMM(modelViewMatrix,0,
						  modelViewMatrix,0,
						  translationMatrix,0);				  

		Matrix.multiplyMM(modelViewProjectionMatrix,0,
						  perspectiveProjectionMatrix,0,
						  modelViewMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES31.glUniformMatrix4fv(mvp_Uniform, 1, false, modelViewProjectionMatrix, 0);
		//GL_FALSE = internally we are not transposing = row -> col / col -> row
		//OpenGL is col major while DirectX is row major

		//bind with vao(this will avoid many repetitive binding with vbo)
		GLES31.glBindVertexArray(vao_second_I[0]);

		//similarly bind the textures if any

		//draw necessary scene
		GLES31.glDrawArrays(GLES31.GL_LINES, 0, 2);
		GLES31.glDrawArrays(GLES31.GL_LINES, 2, 2);
		GLES31.glDrawArrays(GLES31.GL_LINES, 4, 2);
		
		//unbind va0
		GLES31.glBindVertexArray(0);
	//----------------------------------------------------------------------------------------------------------------------------------
			//for A

		//initialize above matrices to identity
		Matrix.setIdentityM(modelViewMatrix,0);
		Matrix.setIdentityM(modelViewProjectionMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		//do necessary transformation if ant required
		//translateM(float[] m, int mOffset, float x, float y, float z)
		//Translates matrix m by x, y, and z in place.
		//translationMatrix = translate(-1.5f, 0.0f, -6.0f);
		Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -2.0f);

		//ratationMatrix = rotate(angle_tri, 0.0f, 1.0f, 0.0f);
		//static void	setRotateM(float[] rm, int rmOffset, float a, float x, float y, float z)
		//Creates a matrix for rotation by angle a (in degrees) around the axis (x, y, z).
		//Matrix.setRotateM(rotationMatrix , 0, angle_Tri, 0.0f, 1.0f, 0.0f);

		//do necessary matrix multiplication
		//modelViewMatrix = translationMatrix * ratationMatrix;
		//modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix

		Matrix.multiplyMM(modelViewMatrix,0,
						  modelViewMatrix,0,
						  rotationMatrix,0);

		Matrix.multiplyMM(modelViewMatrix,0,
						  modelViewMatrix,0,
						  translationMatrix,0);				  

		Matrix.multiplyMM(modelViewProjectionMatrix,0,
						  perspectiveProjectionMatrix,0,
						  modelViewMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES31.glUniformMatrix4fv(mvp_Uniform, 1, false, modelViewProjectionMatrix, 0);
		//GL_FALSE = internally we are not transposing = row -> col / col -> row
		//OpenGL is col major while DirectX is row major

		//bind with vao(this will avoid many repetitive binding with vbo)
		GLES31.glBindVertexArray(vao_A[0]);

		//similarly bind the textures if any

		//draw necessary scene
		GLES31.glDrawArrays(GLES31.GL_LINES, 0, 2);
		GLES31.glDrawArrays(GLES31.GL_LINES, 2, 2);
		GLES31.glDrawArrays(GLES31.GL_LINES, 4, 2);
		GLES31.glDrawArrays(GLES31.GL_LINES, 6, 2);
		GLES31.glDrawArrays(GLES31.GL_LINES, 8, 2);
		
		//unbind va0
		GLES31.glBindVertexArray(0);
	//-------------------------------------------------------------------------------------------------------------------------------

		//unbinding - unused program
		GLES31.glUseProgram(0);

		requestRender();		//SwapBuffers()
	}	

	void uninitialize()
	{
		//code
		
		if (vao_first_I[0] != 0)
		{
			GLES31.glDeleteVertexArrays(1, vao_first_I,0);
			vao_first_I[0] = 0;
		}

		if (vbo_first_I_pos[0] != 0)
		{
			GLES31.glDeleteVertexArrays(1, vbo_first_I_pos,0);
			vbo_first_I_pos[0] = 0;
		}

		if (vbo_first_I_col[0] != 0)
		{
			GLES31.glDeleteVertexArrays(1, vbo_first_I_col,0);
			vbo_first_I_col[0] = 0;
		}

		if (vao_D[0] != 0)
		{
			GLES31.glDeleteVertexArrays(1, vao_D,0);
			vao_D[0] = 0;
		}

		if (vbo_D_pos[0] != 0)
		{
			GLES31.glDeleteVertexArrays(1, vbo_D_pos,0);
			vbo_D_pos[0] = 0;
		}

		if (vbo_D_col[0] != 0)
		{
			GLES31.glDeleteVertexArrays(1, vbo_D_col,0);
			vbo_D_col[0] = 0;
		}

		if (vao_N[0] != 0)
		{
			GLES31.glDeleteVertexArrays(1, vao_N,0);
			vao_N[0] = 0;
		}

		if (vbo_N_pos[0] != 0)
		{
			GLES31.glDeleteVertexArrays(1, vbo_N_pos,0);
			vbo_N_pos[0] = 0;
		}

		if (vbo_N_col[0] != 0)
		{
			GLES31.glDeleteVertexArrays(1, vbo_N_col,0);
			vbo_N_col[0] = 0;
		}

		if (vao_second_I[0] != 0)
		{
			GLES31.glDeleteVertexArrays(1, vao_second_I,0);
			vao_second_I[0] = 0;
		}

		if (vbo_second_I_pos[0] != 0)
		{
			GLES31.glDeleteVertexArrays(1, vbo_second_I_pos,0);
			vbo_second_I_pos[0] = 0;
		}

		if (vbo_second_I_col[0] != 0)
		{
			GLES31.glDeleteVertexArrays(1, vbo_second_I_col,0);
			vbo_second_I_col[0] = 0;
		}

		if (vao_A[0] != 0)
		{
			GLES31.glDeleteVertexArrays(1, vao_A,0);
			vao_A[0] = 0;
		}

		if (vbo_A_pos[0] != 0)
		{
			GLES31.glDeleteVertexArrays(1, vbo_A_pos,0);
			vbo_A_pos[0] = 0;
		}

		if (vbo_A_col[0] != 0)
		{
			GLES31.glDeleteVertexArrays(1, vbo_A_col,0);
			vbo_A_col[0] = 0;
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
