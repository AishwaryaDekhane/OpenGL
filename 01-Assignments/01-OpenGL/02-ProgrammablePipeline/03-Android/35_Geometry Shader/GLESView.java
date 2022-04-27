package com.AMD.geometry_shader;

//packages added by me

//packages added for PP Blue Window Code
import android.opengl.GLSurfaceView;
import android.opengl.GLES32;				//opengl 3.2 version
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
	private int gGeometryShaderObject;;

	//we did'nt have address in java so pass array of one
	private int [] vao = new int [1];

	//as java did'nt have address operator we pass it as array of one
	private int [] vbo = new int [1];

	private int mvp_Uniform;
	String szInfoLog = null;
	private float[] perspectiveProjectionMatrix = new float[16];		//4*4 matrix

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
		String shadingLanguageVersion = gl.glGetString(GLES32.GL_SHADING_LANGUAGE_VERSION);

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

		//VERTEX SHADER
		vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

		final String vertexShaderSourceCode = String.format
		(
			"#version 320 es" +
			"\n" +
			"precision mediump int;" +
			"in vec4 vPosition;" +
			"in vec4 vColor;" +
			"uniform mat4 u_mvp_matrix;" +
			"out vec4 out_color;" +
			"void main(void)" +
			"{" +
			"out_color = vColor;" +
			"gl_Position = u_mvp_matrix * vPosition;" +
			"}"
		);

		//this line internally calls NDK functions
		//specify above source code to vertex shader object
		GLES32.glShaderSource(vertexShaderObject,vertexShaderSourceCode);

		/*

		*	void glShaderSource(	GLuint shader,
		*	GLsizei count,
		*	const GLchar **string,
		*	const GLint *length);

		*	Parameters
		*	shader
		*	Specifies the handle of the shader object whose source code is to be replaced.

		*	count
		*	Specifies the number of elements in the string and length arrays.

		*	string
		*	Specifies an array of pointers to strings containing the source code to be loaded into the shader.

		*	length
		*	Specifies an array of string lengths.
		*/

		//compile the vertex shader
		GLES32.glCompileShader(vertexShaderObject);

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
		szInfoLog = null;

		//step 1
		GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus, 0);


		//step 2
		if (iShaderCompileStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);

			if (iInfoLogLength[0] > 0)
			{
				szInfoLog = GLES32.glGetShaderInfoLog(vertexShaderObject);

				System.out.println("RTR: Vertex Shader Compilation Log" + szInfoLog);
			
				uninitialize();
				
				System.exit(0);
			}
		}

		// *** GEOMETRY SHADER ***

	// create shader
	gGeometryShaderObject = GLES32.glCreateShader(GLES32.GL_GEOMETRY_SHADER);

	// provide source code to shader
	final String geometryShaderSourceCode = String.format
	(
		"#version 320 es" +
		"\n" +
		"layout(triangles)in;" +
		"layout(triangle_strip, max_vertices = 9)out;" +
		"uniform mat4 u_mvp_matrix;" +
		"in vec4 out_color[];" +
		"out vec4 glColor;" +
		"void main(void)" +
		"{" +
		"for(int vertex = 0; vertex < 3; vertex++)" +
		"{" +

		"gl_Position = u_mvp_matrix * (gl_in[vertex].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));" +
		"glColor = out_color[vertex];" +
		"EmitVertex();" +

		"gl_Position = u_mvp_matrix * (gl_in[vertex].gl_Position + vec4(-1.0, -1.0, 0.0, 0.0));" +
		"glColor = out_color[vertex];" +
		"EmitVertex();" +

		"gl_Position = u_mvp_matrix * (gl_in[vertex].gl_Position + vec4(1.0, -1.0, 0.0, 0.0));" +
		"glColor = out_color[vertex];" +
		"EmitVertex();" +

		"EndPrimitive();" +
		"}" +
		"}"
	);

	GLES32.glShaderSource(gGeometryShaderObject,geometryShaderSourceCode);
	//compile the vertex shader
	GLES32.glCompileShader(gGeometryShaderObject);
	//Error checking for FS
	//step 1
	GLES32.glGetShaderiv(gGeometryShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus, 0);
	//step 2
	if (iShaderCompileStatus[0] == GLES32.GL_FALSE)
	{
		GLES32.glGetShaderiv(gGeometryShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);

		if (iInfoLogLength[0] > 0)
		{
			szInfoLog = GLES32.glGetShaderInfoLog(gGeometryShaderObject);

			System.out.println("RTR: Geometry Shader Compilation Log" + szInfoLog);
		
			uninitialize();
			
			System.exit(0);
		}
	}


		//FRAGMENT SHADER

		iShaderCompileStatus[0] = 0;
		iInfoLogLength[0] = 0;
		szInfoLog = null;

		fragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);

		final String fragmentShaderSourceCode = String.format
		(
			"#version 320 es" +
			"\n" +
			"precision highp float;" +
			"out vec4 FragColor;" +
			"in vec4 glColor;" +
			"void main(void)" +
			"{" +
			"FragColor = glColor;" +
			"}"
		);

		//this line internally calls NDK functions
		//specify above source code to fragment shader object
		GLES32.glShaderSource(fragmentShaderObject,fragmentShaderSourceCode);

		//compile the vertex shader
		GLES32.glCompileShader(fragmentShaderObject);

		//Error checking for FS

		//step 1
		GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus, 0);


		//step 2
		if (iShaderCompileStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);

			if (iInfoLogLength[0] > 0)
			{
				szInfoLog = GLES32.glGetShaderInfoLog(fragmentShaderObject);

				System.out.println("RTR: Fragemnt Shader Compilation Log" + szInfoLog);
			
				uninitialize();
				
				System.exit(0);
			}
		}

		
		//SHADER PROGRAM

		//create obj
		shaderProgramObject = GLES32.glCreateProgram();

		//add vertex shader
		GLES32.glAttachShader(shaderProgramObject, vertexShaderObject);

		//add fragment shader
		GLES32.glAttachShader(shaderProgramObject, gGeometryShaderObject);

		//add fragment shader
		GLES32.glAttachShader(shaderProgramObject, fragmentShaderObject);

		//Prelinking binding to vertex attributes
		GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
		//GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_COLOR, "vColor");

		GLES32.glLinkProgram(shaderProgramObject);

		//Error checking for shader

		iInfoLogLength[0] = 0;
		szInfoLog = null;
		int[] iProgramLinkStatus = new int[1];

		//step 1
		GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_LINK_STATUS, iProgramLinkStatus, 0);

		//step 2
		if (iProgramLinkStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);

			if (iInfoLogLength[0] > 0)
			{
					szInfoLog = GLES32.glGetShaderInfoLog(fragmentShaderObject);

					System.out.println("RTR: Program Shader Compilation Log" + szInfoLog);
			
					uninitialize();
				
					System.exit(0);		
			}
		}

		//--------------------------------------------------------------------------------------

		//Postlinking retriving uniform locations
		mvp_Uniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_mvp_matrix");

	
		final float [] verticesPosition= new float []
		{ 0.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f };

		final float [] verticesColor= new float []
		{ 1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f };

		//-------------------------------------------------------
		//for position

		//vao = VertexArrayObject
		GLES32.glGenVertexArrays(1, vao, 0);

		//bind array with vao
		GLES32.glBindVertexArray(vao[0]);

		System.out.println("RTR: vao created" + szInfoLog);

		//create vbo
		GLES32.glGenBuffers(1, vbo, 0);

		//target vbo <-> GL_ARRAY_BUFFER
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo[0]);

		System.out.println("RTR: vbo position created" + szInfoLog);

		//To convert the array to such buffer which is capable of to create a buffer data array

		//step 1
		//Allowcate the buffer directly from the native memory(not from VM)

		ByteBuffer byteBufferTriPos = ByteBuffer.allocateDirect(verticesPosition.length * 4);	

		//step 2
		//Arrange the buffer in the byte order of buffer natively(big/little endian)
		
		byteBufferTriPos.order(ByteOrder.nativeOrder());

		//step 3
		//Create the float type buffer & convert our byte type buffer to float type buffer

		FloatBuffer positionBufferTriPos = byteBufferTriPos.asFloatBuffer();

		//step 4
		//Now put your array into the cooked buffer

		positionBufferTriPos.put(verticesPosition);

		//step 5
		//Not necessary but useful for large array -> interleaved array.
		//Set the array at the 0th position

		positionBufferTriPos.position(0);

		//fill the buffer data statically
		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, verticesPosition.length * 4, positionBufferTriPos, GLES32.GL_STATIC_DRAW);

		//give strides to vPosition
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION,3 , GLES32.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or

		//enable array
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);	//AMC = AstroMediComp

		//unbind buffer vbo
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);


		//for color

		//create vbo
		GLES32.glGenBuffers(1, vbo, 0);

		//target vbo <-> GL_ARRAY_BUFFER
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo[0]);

		System.out.println("RTR: vbo color created" + szInfoLog);

		//To convert the array to such buffer which is capable of to create a buffer data array

		//step 1
		//Allowcate the buffer directly from the native memory(not from VM)

		ByteBuffer byteBufferTriColor = ByteBuffer.allocateDirect(verticesColor.length * 4);	

		//step 2
		//Arrange the buffer in the byte order of buffer natively(big/little endian)
		
		byteBufferTriColor.order(ByteOrder.nativeOrder());

		//step 3
		//Create the float type buffer & convert our byte type buffer to float type buffer

		FloatBuffer colorBufferTri = byteBufferTriColor.asFloatBuffer();

		//step 4
		//Now put your array into the cooked buffer

		colorBufferTri.put(verticesColor);

		//step 5
		//Not necessary but useful for large array -> interleaved array.
		//Set the array at the 0th position

		colorBufferTri.position(0);

		//fill the buffer data statically
		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, verticesColor.length * 4, colorBufferTri, GLES32.GL_STATIC_DRAW);

		//give strides to vPosition
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);  //false = we are not sending normalized co-or

		//enable array
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);	//AMC = AstroMediComp

		//unbind buffer
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);


		//unbind array
		GLES32.glBindVertexArray(0);

	//-------------------------------------------------------------------------------------------------------------------------------------------

		//for background color
		GLES32.glClearColor(0.0f,0.0f,0.0f,1.0f);

		//For Depth

		//GLES32.glClearDepth(1.0f);	//give existance to depth buffer

		GLES32.glEnable(GLES32.GL_DEPTH_TEST);	//for 3D geometry enable depth test i.e z axis 

		GLES32.glDepthFunc(GLES32.GL_LEQUAL);		//LEQUAL = less than or equal to test compared to the max value which is set to 1.0f

		//identity() is in mat4
		Matrix.setIdentityM(perspectiveProjectionMatrix, 0);

		//no warmup call to resize as we are already inn fullscreen

	}

	private void resize(int width, int height)
	{
		System.out.println("RTR: In resize" + szInfoLog); 

		//code
		if (height == 0)
		{
			height = 1;
		}
		
		GLES32.glViewport(0, 0, width, height);


	//perspectiveM(float[] m, int offset, float fovy, float aspect, float zNear, float zFar)
	//Defines a projection matrix in terms of a field of view angle, an aspect ratio, and z clip planes.

		Matrix.perspectiveM(perspectiveProjectionMatrix, 0 , 45.0f, ((float)width /(float) height), 0.1f, 100.0f);
	}

	private void display()
	{
		System.out.println("RTR: In display" + szInfoLog);

		GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT |GLES32.GL_DEPTH_BUFFER_BIT);

		//binding
		GLES32.glUseProgram(shaderProgramObject);

		// OpenGL Drawing
		//declaration of matrices
		float [] modelViewMatrix = new float[16];
		float [] modelViewProjectionMatrix = new float[16];
		float [] translationMatrix = new float[16];

		//initialize above matrices to identity
		Matrix.setIdentityM(modelViewMatrix,0);
		Matrix.setIdentityM(modelViewProjectionMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);

		//do necessary transformation if ant required
		//translateM(float[] m, int mOffset, float x, float y, float z)
		//Translates matrix m by x, y, and z in place.
		//translationMatrix = translate(-1.5f, 0.0f, -6.0f);
		Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -4.0f);

		//do necessary matrix multiplication
		//modelViewMatrix = translationMatrix * ratationMatrix;
		//modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
		Matrix.multiplyMM(modelViewMatrix,0,
						  modelViewMatrix,0,
						  translationMatrix,0);				  

		Matrix.multiplyMM(modelViewProjectionMatrix,0,
						  perspectiveProjectionMatrix,0,
						  modelViewMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(mvp_Uniform, 1, false, modelViewProjectionMatrix, 0);
		//GL_FALSE = internally we are not transposing = row -> col / col -> row
		//OpenGL is col major while DirectX is row major

		//bind with vao(this will avoid many repetitive binding with vbo)
		GLES32.glBindVertexArray(vao[0]);

		//similarly bind the textures if any

		//draw necessary scene
		GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 3);
		// glDrawArrays(GLenum, GLint , GLsizei)
		// (pass primitive inside glBegin(), start from 0th row 0th index, triangle vertices)

		//unbind va0
		GLES32.glBindVertexArray(0);

		//----------------------------------------------------------------------------------------------------------------

		//unbinding - unused program
		GLES32.glUseProgram(0);

		requestRender();		//SwapBuffers()

}

void uninitialize()
{
	System.out.println("RTR: In uninitialize" + szInfoLog);

	//code

	if (vao[0] != 0)
	{
		GLES32.glDeleteBuffers(1, vao, 0);
		vao[0] = 0;
	}

	if (vbo[0] != 0)
	{
		GLES32.glDeleteBuffers(1, vbo, 0);
		vbo[0] = 0;
	}

	//safe release

	if(shaderProgramObject != 0)
		{
			int[] shaderCount = new int[1];
			int shaderNumber;

			GLES32.glUseProgram(shaderProgramObject);

			//ask program how many shaders are attached
			GLES32.glGetProgramiv(	shaderProgramObject,
									GLES32.GL_ATTACHED_SHADERS,
									shaderCount, 0);

			int[] shaders = new int[shaderCount[0]];

			if(shaderCount[0] != 0)
			{
				GLES32.glGetAttachedShaders(	shaderProgramObject,
												shaderCount[0],
												shaderCount, 0,
												shaders, 0);

				for(shaderNumber = 0; shaderNumber < shaderCount[0]; shaderNumber++)
				{
					//detach shaders
					GLES32.glDetachShader(	shaderProgramObject,
											shaders[shaderNumber]);

					//delete shaders
					GLES32.glDeleteShader(shaders[shaderNumber]);

					shaders[shaderNumber] = 0;
				}
			} 
			GLES32.glDeleteProgram(shaderProgramObject);
			shaderProgramObject = 0;
			GLES32.glUseProgram(0);
		}
}
}
