package com.AMD.tri_rec_colored;

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


public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener
{
    private GestureDetector gestureDetector;
    private final Context context;

	//declare variables for shaders
	private int vertexShaderObject;
	private int fragmentShaderObject;
	private int shaderProgramObject;		//java did'nt have GLuint/uint

	//we did'nt have address in java so pass array of one
	private int [] vao_triangle = new int [1];
	private int [] vao_rectangle = new int [1];

	//as java did'nt have address operator we pass it as array of one
	private int [] vbo_position_triangle = new int [1];
	private int [] vbo_position_rectangle = new int [1];

	private int [] vbo_color_triangle = new int [1];
	private int [] vbo_color_rectangle = new int [1];

	private int mvp_Uniform;

	private float[] perspectiveProjectionMatrix = new float[16];		//4*4 matrix

	private float angle_tri = 0.0f;
	private float angle_rec = 0.0f;

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
		update();
        display();
    }


	//our custom methods
	private void initialize()
	{

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

		//declare vertex3f coordinates here
		final float [] triangleVertices = new float []
		{ 
			0.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f
		};

		final float [] rectangleVertices = new float []
		{
			 1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f
		};

		final float [] triangleColor = new float []
		{ 
			1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f
		};

		final float [] rectangleColor = new float []
		{ 
			0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f
		};

		//-------------------------------------------------------

		//for triangle

		//for position

		//vao = VertexArrayObject
		GLES31.glGenVertexArrays(1, vao_triangle, 0);

		//bind array with vao
		GLES31.glBindVertexArray(vao_triangle[0]);

		//create vbo
		GLES31.glGenBuffers(1, vbo_position_triangle, 0);

		//target vbo <-> GL_ARRAY_BUFFER
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_position_triangle[0]);

		//To convert the array to such buffer which is capable of to create a buffer data array

		//step 1
		//Allowcate the buffer directly from the native memory(not from VM)

		ByteBuffer byteBufferTriPos = ByteBuffer.allocateDirect(triangleVertices.length * 4);	

		//step 2
		//Arrange the buffer in the byte order of buffer natively(big/little endian)
		
		byteBufferTriPos.order(ByteOrder.nativeOrder());

		//step 3
		//Create the float type buffer & convert our byte type buffer to float type buffer

		FloatBuffer positionBufferTriPos = byteBufferTriPos.asFloatBuffer();

		//step 4
		//Now put your array into the cooked buffer

		positionBufferTriPos.put(triangleVertices);

		//step 5
		//Not necessary but useful for large array -> interleaved array.
		//Set the array at the 0th position

		positionBufferTriPos.position(0);

		//fill the buffer data statically
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, triangleVertices.length * 4, positionBufferTriPos, GLES31.GL_STATIC_DRAW);

		//give strides to vPosition
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES31.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or

		//enable array
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp

		//unbind buffer vbo
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);



		//for color

		//create vbo
		GLES31.glGenBuffers(1, vbo_color_triangle, 0);

		//target vbo <-> GL_ARRAY_BUFFER
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_color_triangle[0]);

		//To convert the array to such buffer which is capable of to create a buffer data array

		//step 1
		//Allowcate the buffer directly from the native memory(not from VM)

		ByteBuffer byteBufferTriColor = ByteBuffer.allocateDirect(triangleColor.length * 4);	

		//step 2
		//Arrange the buffer in the byte order of buffer natively(big/little endian)
		
		byteBufferTriColor.order(ByteOrder.nativeOrder());

		//step 3
		//Create the float type buffer & convert our byte type buffer to float type buffer

		FloatBuffer colorBufferTri = byteBufferTriColor.asFloatBuffer();

		//step 4
		//Now put your array into the cooked buffer

		colorBufferTri.put(triangleColor);

		//step 5
		//Not necessary but useful for large array -> interleaved array.
		//Set the array at the 0th position

		colorBufferTri.position(0);

		//fill the buffer data statically
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, triangleColor.length * 4, colorBufferTri, GLES31.GL_STATIC_DRAW);

		//give strides to vPosition
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES31.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or

		//enable array
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);	//AMC = AstroMediComp

		//unbind buffer
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);



		//unbind array
		GLES31.glBindVertexArray(0);

	//--------------------------------------------------------------------------------------------------------------

		//for rectangle

		//for position

		//vao = VertexArrayObject
		GLES31.glGenVertexArrays(1, vao_rectangle, 0);

		//bind array with vao
		GLES31.glBindVertexArray(vao_rectangle[0]);

		//create vbo
		GLES31.glGenBuffers(1, vbo_position_rectangle, 0);

		//target vbo <-> GL_ARRAY_BUFFER
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_position_rectangle[0]);

		//To convert the array to such buffer which is capable of to create a buffer data array

		//step 1
		//Allowcate the buffer directly from the native memory(not from VM)

		ByteBuffer byteBufferRecPos = ByteBuffer.allocateDirect(rectangleVertices.length * 4);	

		//step 2
		//Arrange the buffer in the byte order of buffer natively(big/little endian)
		
		byteBufferRecPos.order(ByteOrder.nativeOrder());

		//step 3
		//Create the float type buffer & convert our byte type buffer to float type buffer

		FloatBuffer positionBufferRecPos = byteBufferRecPos.asFloatBuffer();

		//step 4
		//Now put your array into the cooked buffer

		positionBufferRecPos.put(rectangleVertices);

		//step 5
		//Not necessary but useful for large array -> interleaved array.
		//Set the array at the 0th position

		positionBufferRecPos.position(0);

		//fill the buffer data statically
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, rectangleVertices.length * 4, positionBufferRecPos, GLES31.GL_STATIC_DRAW);

		//give strides to vPosition
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES31.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or

		//enable array
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp

		//unbind buffer
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, 0);


		//for color

		//create vbo
		GLES31.glGenBuffers(1, vbo_color_rectangle, 0);

		//target vbo <-> GL_ARRAY_BUFFER
		GLES31.glBindBuffer(GLES31.GL_ARRAY_BUFFER, vbo_color_rectangle[0]);

		//To convert the array to such buffer which is capable of to create a buffer data array

		//step 1
		//Allowcate the buffer directly from the native memory(not from VM)

		ByteBuffer byteBufferRecColor = ByteBuffer.allocateDirect(rectangleColor.length * 4);	

		//step 2
		//Arrange the buffer in the byte order of buffer natively(big/little endian)
		
		byteBufferRecColor.order(ByteOrder.nativeOrder());

		//step 3
		//Create the float type buffer & convert our byte type buffer to float type buffer

		FloatBuffer colorBufferRec = byteBufferRecColor.asFloatBuffer();

		//step 4
		//Now put your array into the cooked buffer

		colorBufferRec.put(rectangleColor);

		//step 5
		//Not necessary but useful for large array -> interleaved array.
		//Set the array at the 0th position

		colorBufferRec.position(0);

		//fill the buffer data statically
		GLES31.glBufferData(GLES31.GL_ARRAY_BUFFER, rectangleColor.length * 4, colorBufferRec, GLES31.GL_STATIC_DRAW);

		//give strides to vPosition
		GLES31.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES31.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or

		//enable array
		GLES31.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);	//AMC = AstroMediComp

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

	}

	private void update()
	{
		angle_tri = angle_tri + 0.5f;
		if (angle_tri >= 360.0f)
			angle_tri = angle_tri - 360.0f;

		angle_rec = angle_rec + 0.5f;
		if (angle_rec >= 360.0f)
			angle_rec = angle_rec - 360.0f;

	}

	private void resize(int width, int height)
	{
		//code
		if (height == 0)
		{
			height = 1;
		}
		
		GLES31.glViewport(0, 0, width, height);


	//perspectiveM(float[] m, int offset, float fovy, float aspect, float zNear, float zFar)
	//Defines a projection matrix in terms of a field of view angle, an aspect ratio, and z clip planes.

		Matrix.perspectiveM(perspectiveProjectionMatrix, 0 , 45.0f, (width / height), 0.1f, 100.0f);
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

		//for triangle

		//initialize above matrices to identity
		Matrix.setIdentityM(modelViewMatrix,0);
		Matrix.setIdentityM(modelViewProjectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);

		//do necessary transformation if ant required
		//translateM(float[] m, int mOffset, float x, float y, float z)
		//Translates matrix m by x, y, and z in place.
		//translationMatrix = translate(-1.5f, 0.0f, -6.0f);
		Matrix.translateM(modelViewMatrix, 0, -1.5f, 0.0f, -7.0f);

		//ratationMatrix = rotate(angle_tri, 0.0f, 1.0f, 0.0f);
		//static void	setRotateM(float[] rm, int rmOffset, float a, float x, float y, float z)
		//Creates a matrix for rotation by angle a (in degrees) around the axis (x, y, z).
		Matrix.setRotateM(rotationMatrix , 0, angle_tri, 0.0f, 1.0f, 0.0f);

		//do necessary matrix multiplication
		//modelViewMatrix = translationMatrix * ratationMatrix;
		//modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
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
		GLES31.glBindVertexArray(vao_triangle[0]);

		//similarly bind the textures if any

		//draw necessary scene
		GLES31.glDrawArrays(GLES31.GL_TRIANGLES, 0, 3);
		// glDrawArrays(GLenum, GLint , GLsizei)
		// (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

		//unbind va0
		GLES31.glBindVertexArray(0);

		//-----------------------------------------------------------------------------------------
		//for rectangle

		//initialize above matrices to identity
		Matrix.setIdentityM(modelViewMatrix,0);
		Matrix.setIdentityM(modelViewProjectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);

		//do necessary transformation if ant required
		//translateM(float[] m, int mOffset, float x, float y, float z)
		//Translates matrix m by x, y, and z in place.
		//modelViewMatrix = translate(1.5f, 0.0f, -6.0f);
		Matrix.translateM(modelViewMatrix, 0, 1.5f, 0.0f, -7.0f);

		//ratationMatrix = rotate(angle_rec, 0.0f, 1.0f, 0.0f);
		//static void	setRotateM(float[] rm, int rmOffset, float a, float x, float y, float z)
		//Creates a matrix for rotation by angle a (in degrees) around the axis (x, y, z).
		Matrix.setRotateM(rotationMatrix, 0, angle_rec, 0.0f, 1.0f, 0.0f);

		//do necessary matrix multiplication
		//modelViewMatrix = translationMatrix * ratationMatrix;
		//modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
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
		GLES31.glBindVertexArray(vao_rectangle[0]);

		//similarly bind the textures if any

		//draw necessary scene
		GLES31.glDrawArrays(GLES31.GL_TRIANGLE_FAN, 0, 4);
		// glDrawArrays(GLenum, GLint , GLsizei)
		// (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

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

	if (vao_triangle[0] != 0)
	{
		GLES31.glDeleteBuffers(1, vao_triangle, 0);
		vao_triangle[0] = 0;
	}

	if (vao_rectangle[0] != 0)
	{
		GLES31.glDeleteBuffers(1, vao_rectangle, 0);
		vao_rectangle[0] = 0;
	}

	if (vbo_position_triangle[0] != 0)
	{
		GLES31.glDeleteBuffers(1, vbo_position_triangle, 0);
		vbo_position_triangle[0] = 0;
	}

	if (vbo_position_rectangle[0] != 0)
	{
		GLES31.glDeleteBuffers(1, vbo_position_rectangle, 0);
		vbo_position_rectangle[0] = 0;
	}

	if (vbo_color_triangle[0] != 0)
	{
		GLES31.glDeleteBuffers(1, vbo_color_triangle, 0);
		vbo_color_triangle[0] = 0;
	}

	if (vbo_color_rectangle[0] != 0)
	{
		GLES31.glDeleteBuffers(1, vbo_color_rectangle, 0);
		vbo_color_rectangle[0] = 0;
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
