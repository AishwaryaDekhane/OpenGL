package com.AMD.render_texture;

//packages added by me

import android.opengl.GLSurfaceView;
import android.opengl.GLES32;	//opengl 3.2 version

import javax.microedition.khronos.opengles.GL10;//opengl extension starts desktop 1.0 , J2ME = Java to microedition
import javax.microedition.khronos.egl.EGLConfig;//Embedded graphic Library

import android.content.Context;// for drawing context related
import android.view.Gravity;//for Gravity	
import android.graphics.Color;// for "Color" class
import android.view.MotionEvent;// for "MotionEvent"
import android.view.GestureDetector;// for GestureDetector
import android.view.GestureDetector.OnGestureListener;// for OnGestureListener
import android.view.GestureDetector.OnDoubleTapListener;// OnDoubleTapListener

//packages added for ortho triangle - for OpenGL Buffers
import java.nio.ByteBuffer;	//native i/o / non-blocking i/o
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;

import android.graphics.BitmapFactory;
import android.graphics.BitmapFactory.Options;
import android.graphics.Bitmap;
import android.opengl.GLUtils;//for textImage2D	

import android.opengl.Matrix;	//for matrix math


public class GLESView extends GLSurfaceView 
		implements GLSurfaceView.Renderer,OnGestureListener, OnDoubleTapListener
{
	private final Context context;
	private GestureDetector gestureDetector;	
	
	//declare variables for shaders
	private int vertexShaderObject;
	private int fragmentShaderObject;
	private int shaderProgramObject;//java did'nt have GLuint/uint
	
		//we did'nt have address in java so pass array of one
	private int[] vao_cube = new int[1];
	private int[] vbo_position_cube = new int[1];
	private int[] vbo_texture_cube = new int[1];
	private int[] marble_texture = new int[1];
	
		//as java did'nt have address operator we pass it as array of one
	private int[] fbo = new int[1];
	private int[] depth_texture = new int[1];;
	private int[] color_texture = new int[1];
	private IntBuffer drawBuffer;
	int drawFrameBuffers[] = new int[]{GLES32.GL_COLOR_ATTACHMENT0};
	private int mvpUniform;
	private int samplerUniform;
	
	private int gwidth, gheight;
	
	private float[] perspectiveProjectionMatrix = new float[16];
	float angle_cube_inner = 0.0f;
	float angle_cube_outer = 360.0f; 

	GLESView(Context drawingContext)
	{
		super(drawingContext);
		context = drawingContext;
		
		setEGLContextClientVersion(3);
		setRenderer(this);
		setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
		gestureDetector = new GestureDetector(drawingContext, this,null,false);
	}
	    // Handling 'onTouchEvent' Is The Most IMPORTANT,
    // Because It Triggers All Gesture And Tap Events

	 // abstract method from OnDoubleTapListener so must be implemented
	@Override
	public boolean onTouchEvent(MotionEvent event)
	{
		if(!gestureDetector.onTouchEvent(event))
		{
			super.onTouchEvent(event);
		}
		return (true);
	}
	
	 // abstract method from OnDoubleTapListener so must be implemented
	@Override
	public boolean onDoubleTap(MotionEvent e)
	{
		return (true);
	}
	
	 // abstract method from OnDoubleTapListener so must be implemented
	@Override
	public boolean onDoubleTapEvent(MotionEvent e)
	{
		return  (true);
	}
	
	 // abstract method from OnDoubleTapListener so must be implemented
	@Override
	public boolean onSingleTapConfirmed(MotionEvent event)
	{
		return (true);
	}
	
	    // abstract method from OnGestureListener so must be implemented
	@Override
	public boolean onDown(MotionEvent e)
	{
		return (true);
	}

	    // abstract method from OnGestureListener so must be implemented
	@Override
	public boolean onFling(MotionEvent e1,MotionEvent e2, float velocityX, float velocityY)
	{
		return (true);
	}
	
	    // abstract method from OnGestureListener so must be implemented
	@Override
	public void onLongPress(MotionEvent e)
	{
		//setText("Long Press");
	}
	
	    // abstract method from OnGestureListener so must be implemented
	@Override
	public boolean onScroll(MotionEvent e1,MotionEvent e2, float distanceX, float distanceY)
	{
		//setTextColor(255,128,128);
		System.exit(0);
		return (true);
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
		return (true);
	}
	
	// abstract method from OnGestureListener so must be implemented
	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{
		initialize();
	}
	
	// abstract method from OnGestureListener so must be implemented
	@Override
	public void onSurfaceChanged(GL10 unused, int width, int height)
	{
		gwidth = width;
		gheight = height;
		resize(width, height);
	}
	
	// abstract method from OnGestureListener so must be implemented
	@Override
	public void onDrawFrame(GL10 unused)
	{
		angle_cube_outer = angle_cube_outer - 1.0f;
		if (angle_cube_outer == 0.0f)
		{
			angle_cube_outer = 360.0f;
		}
		
		angle_cube_inner = angle_cube_inner + 1.0f;
		if (angle_cube_inner == 360.0f)
		{
			angle_cube_inner = 0.0f;
		}
			display();
	}

		//our custom methods
	private void initialize()
	{
		//this line internally calls NDK functions
		//specify above source code to vertex shader object
		vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

		final String vertexShaderSourceCode = String.format
		(
			"#version 320 es" + 
			"\n" +
			"in vec4 vPosition;" +
			"in vec2 vTexCoord;" +
			"uniform mat4 u_mvp_matrix;" +
			"out vec2 out_texcoord;"+
			"void main(void)" +
			"{"+
				"gl_Position = u_mvp_matrix * vPosition;"+
				"out_texcoord = vTexCoord;" +
			"}"
		);

		
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
		
		GLES32.glShaderSource(vertexShaderObject, vertexShaderSourceCode);

		GLES32.glCompileShader(vertexShaderObject);
		
		int[] iShaderCompileStatus = new int[1];
		int[] iInfoLogLength = new int[1];
		String szInfoLog = null;
		
		GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_COMPILE_STATUS,iShaderCompileStatus,0);
		if(iShaderCompileStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_INFO_LOG_LENGTH,iInfoLogLength, 0);
			if(iInfoLogLength[0]>0)
			{
				szInfoLog = GLES32.glGetShaderInfoLog(vertexShaderObject);
				System.out.println("RTR:" + "Vertex shader compile log"+szInfoLog);
				uninitialize();
				System.exit(0);
			}
		}
		else
		{
			System.out.println("RTR: Vertex Shader Compilation Log");
		}

		//Fragment shader
		
		fragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);
		final String fragmentShaderSourceCode = String.format
		(
			"#version 320 es" +
			"\n" +
			"precision highp float;" +
			"in vec2 out_texcoord;"  +
			"out vec4 FragColor;" +
			"uniform sampler2D u_sampler;"+
			"void main(void)" +
			"{" +
			"FragColor = texture(u_sampler,out_texcoord);"  +
			"}"
		);

		//this line internally calls NDK functions
		//specify above source code to fragment shader object
		GLES32.glShaderSource(fragmentShaderObject, fragmentShaderSourceCode);

		GLES32.glCompileShader(fragmentShaderObject);

		iShaderCompileStatus[0] = 0;
		iInfoLogLength[0] = 0;
		szInfoLog = null;

		GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus, 0);
		if(iShaderCompileStatus[0]== GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_INFO_LOG_LENGTH,iInfoLogLength, 0);
			if(iInfoLogLength[0]>0)
			{
				szInfoLog = GLES32.glGetShaderInfoLog(fragmentShaderObject);
				System.out.println("RTR: "+ "Fragment shader Compilation log :"+ szInfoLog);
				uninitialize();
				System.exit(0);
			}
		}
		else
		{
			System.out.println("RTR: "+ "Fragment : Compilation is successful");
		}


		//shader program object
		shaderProgramObject = GLES32.glCreateProgram();

		GLES32.glAttachShader(shaderProgramObject, vertexShaderObject);
		GLES32.glAttachShader(shaderProgramObject, fragmentShaderObject);
		
		//Pre linking
		GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
		GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_TEXCOORD0, "vTexCoord");

		//LINK
		GLES32.glLinkProgram(shaderProgramObject);
		
		//error checking for linking
		int[] iProgramLinkStatus = new int[1];
		iInfoLogLength[0] = 0;
		szInfoLog = null;
		GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_LINK_STATUS, iProgramLinkStatus, 0);
		if(iProgramLinkStatus[0]== GLES32.GL_FALSE)
		{
			GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
			if(iInfoLogLength[0]>0)
			{
				szInfoLog = GLES32.glGetProgramInfoLog(shaderProgramObject);
				System.out.println("RTR: "+ "Linking shader object log :"+ szInfoLog);
				uninitialize();
				System.exit(0);
			}
		}

		//post linking Get uniform location 
		mvpUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_mvp_matrix");
		samplerUniform=GLES32.glGetUniformLocation(shaderProgramObject,"u_sampler");
		
		System.out.println("RTR: "+ "Start vao creation");
		
		
		//Cube
		final float cubeVertices[] = new float[] {
		//Top square
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		//Front square
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		//back square
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		//right sqaure
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,

		//left square
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f
	};
	
	final float[] cubeTex= new float[]{
		//Top square
		0.0f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f,
		1.0f,1.0f,

		//bottom square
		1.0f,1.0f,
		0.0f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f,

		//front square
		0.0f,0.0f,
		1.0f,0.0f,
		1.0f,1.0f,
		0.0f,1.0f,

		//back square
		1.0f,0.0f,
		1.0f,1.0f,
		0.0f,1.0f,
		0.0f,0.0f,

		//right square
		1.0f,0.0f,
		1.0f,1.0f,
		0.0f,1.0f,
		0.0f,0.0f,

		//left sqaure
		0.0f,0.0f,
		1.0f,0.0f,
		1.0f,1.0f,
		0.0f,1.0f,

		//Top square
		0.0f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f,
		1.0f,1.0f,

		//bottom square
		1.0f,1.0f,
		0.0f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f,

		//front square
		0.0f,0.0f,
		1.0f,0.0f,
		1.0f,1.0f,
		0.0f,1.0f,

		//back square
		1.0f,0.0f,
		1.0f,1.0f,
		0.0f,1.0f,
		0.0f,0.0f,

		//right square
		1.0f,0.0f,
		1.0f,1.0f,
		0.0f,1.0f,
		0.0f,0.0f,

		//left sqaure
		0.0f,0.0f,
		1.0f,0.0f,
		1.0f,1.0f,
		0.0f,1.0f
	};
	GLES32.glGenVertexArrays(1,vao_cube,0);
	GLES32.glBindVertexArray(vao_cube[0]);
	GLES32.glGenBuffers(1,vbo_position_cube,0);
	GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_position_cube[0]);
	
	//BIND buffer for cubeVertices
	ByteBuffer byteBuffer_rect = ByteBuffer.allocateDirect(cubeVertices.length*4);
	byteBuffer_rect.order(ByteOrder.nativeOrder());
	
	FloatBuffer positionBuffer_rect = byteBuffer_rect.asFloatBuffer();
	
	positionBuffer_rect.put(cubeVertices);
	
	positionBuffer_rect.position(0);
	
	GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,cubeVertices.length*4,positionBuffer_rect,GLES32.GL_STATIC_DRAW);
	
	GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION,3,GLES32.GL_FLOAT,false,0,0);
	
	GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
	
	//unbind
	GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,0);

	//-----------------------------------------------------------------------------------

	//create and bind vbo
	GLES32.glGenBuffers(1, vbo_texture_cube, 0);
	GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_texture_cube[0]);
	
	//Allocate the buffer directly from the native memory(not VM memory)
	//(managed not) So we are using NIO native io		
	ByteBuffer byteBuffer = ByteBuffer.allocateDirect(cubeTex.length* 4);
	
	//Arrange the buffer in byte order of buffer natively
	byteBuffer.order(ByteOrder.nativeOrder());
	
	//Create the float type buffer and convert our byte type buffer 
	FloatBuffer colorBuffer = byteBuffer.asFloatBuffer();
	
	//Now put your array into the "cooked buffer."
	colorBuffer.put(cubeTex);
	
	//Set the array at the 0th position the buffer
	colorBuffer.position(0);

	//To convert the array to such buffer which is capable of to create a buffer data array
	//Allowcate the buffer directly from the native memory(not from VM)
	GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubeTex.length*4,colorBuffer,GLES32.GL_STATIC_DRAW);
	
	//give strides to vPosition
	GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_TEXCOORD0, 2, GLES32.GL_FLOAT, false, 0, 0);
	
	//enable array
	GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_TEXCOORD0);
	
	//unbind buffer
	GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
	
	//unbind vao
	GLES32.glBindVertexArray(0);
	
	//----------------------------------------------------------------------------------------------------FRAMEBUFFER
	
	// First, generate and bind our framebuffer object
	GLES32.glGenFramebuffers(1, fbo,0);
	GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, fbo[0]);

	// Generate  texture names
	GLES32.glGenTextures(1, color_texture,0);

	// Bind and allocate storage for it
	GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, color_texture[0]);
	GLES32.glTexStorage2D(GLES32.GL_TEXTURE_2D, 1, GLES32.GL_RGBA16F,512, 512);
	
	// Set its default filter parameters
	GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D,GLES32.GL_TEXTURE_MIN_FILTER, GLES32.GL_LINEAR);
	GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D,GLES32.GL_TEXTURE_MAG_FILTER, GLES32.GL_LINEAR);

	// Attach it to our framebuffer object as color attachments
	GLES32.glFramebufferTexture(GLES32.GL_FRAMEBUFFER,GLES32.GL_COLOR_ATTACHMENT0,color_texture[0], 0);
	GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, 0);

	// Now create a depth texture
	GLES32.glGenTextures(1, depth_texture,0);
	GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, depth_texture[0]);
	GLES32.glTexStorage2D(GLES32.GL_TEXTURE_2D, 1, GLES32.GL_DEPTH_COMPONENT32F, 512, 512);

	// Attach the depth texture to the framebuffer
	GLES32.glFramebufferTexture(GLES32.GL_FRAMEBUFFER, GLES32.GL_DEPTH_ATTACHMENT,depth_texture[0], 0);

	GLES32.glDrawBuffers(1, drawFrameBuffers,0);

		if (GLES32.glCheckFramebufferStatus(GLES32.GL_FRAMEBUFFER) != GLES32.GL_FRAMEBUFFER_COMPLETE)
		{
			System.out.println("RTR: "+ "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
		}
		else
		{
			System.out.println("RTR: "+ "FRAMEBUFFER:: Framebuffer is complete!\n");
		}

		//for texture 
		GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, 0);

		//to bind the buffer
		GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, 0);
		
		Matrix.setIdentityM(perspectiveProjectionMatrix,0);

		//for depth
		GLES32.glEnable(GLES32.GL_DEPTH_TEST);	//for 3D geometry enable depth test i.e z axis 
		GLES32.glDepthFunc(GLES32.GL_LEQUAL);	//LEQUAL = less than or equal to test compared to the max value which is set to 1.0f
		GLES32.glHint(GL10.GL_PERSPECTIVE_CORRECTION_HINT, GL10.GL_NICEST);	//give existance to depth buffer
		
		//to enable the texture
		GLES32.glEnable(GLES32.GL_TEXTURE_2D);
		marble_texture[0] = loadTexture(R.raw.marble);
		
		//for background color
		GLES32.glClearColor(0.0f,0.0f,0.0f,0.0f);
	}
	
	private void resize(int width,int height)
	{
		if(height <0)
		{
			height = 1;
		}
		GLES32.glViewport(0,0,width,height);

		//perspectiveM(float[] m, int offset, float fovy, float aspect, float zNear, float zFar)
		//Defines a projection matrix in terms of a field of view angle, an aspect ratio, and z clip planes.

		
		 Matrix.perspectiveM(perspectiveProjectionMatrix,0,45.0f, (float)width /(float) height, 0.1f, 100.f);
	}
	
	private void display()
	{
		//binding
		GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
		
		// OpenGL Drawing
		//declaration of matrices
		float[] modelViewMatrix = new float[16];
		float[] modelViewProjectionMatrix = new float[16];
		float[] rotationMatrix = new float[16];
		float[] translationMatrix = new float[16];

		//cube color
		float[] colorBufferData = {0.0f, 0.0f, 0.5f};

		float depth[] = { 1.0f };		//for depth of inner cube

		GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, fbo[0]);	
		GLES32.glClearBufferfv(GLES32.GL_COLOR, 0, colorBufferData,0);
		GLES32.glClearBufferfv(GLES32.GL_DEPTH, 0, depth,0);

		//initialize above matrices to identity
		GLES32.glUseProgram(shaderProgramObject);

		Matrix.setIdentityM(modelViewProjectionMatrix, 0);
		Matrix.setIdentityM(modelViewMatrix, 0);
		Matrix.setIdentityM(rotationMatrix, 0);
		Matrix.setIdentityM(translationMatrix, 0);

		GLES32.glViewport(0,0,512,256);

		//do necessary transformation if ant required
		//translateM(float[] m, int mOffset, float x, float y, float z)
		//Translates matrix m by x, y, and z in place.
		//translationMatrix = translate(-1.5f, 0.0f, -6.0f);
		Matrix.translateM(translationMatrix,0,1.0f,0.0f,-4.0f);

		//ratationMatrix = rotate(angle_tri, 0.0f, 1.0f, 0.0f);
		//static void	setRotateM(float[] rm, int rmOffset, float a, float x, float y, float z)
		//Creates a matrix for rotation by angle a (in degrees) around the axis (x, y, z).
		Matrix.setRotateM(rotationMatrix,0,angle_cube_inner,0.0f, 1.0f, 0.0f);

		//do necessary matrix multiplication
		//modelViewMatrix = translationMatrix * ratationMatrix;
		//modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
		Matrix.multiplyMM(modelViewMatrix, 0, translationMatrix, 0,rotationMatrix,0);
		Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0,modelViewMatrix,0);


		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(mvpUniform,1,false, modelViewProjectionMatrix,0);
		//GL_FALSE = internally we are not transposing = row -> col / col -> row
		//OpenGL is col major while DirectX is row major

		//bind with vao(this will avoid many repetitive binding with vbo)

		GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
		GLES32.glBindTexture(GLES32.GL_TEXTURE_2D,marble_texture[0]);

		GLES32.glUniform1i(samplerUniform,0);

		//bind with vao 
		GLES32.glBindVertexArray(vao_cube[0]);

		//draw
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN,0,4);
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN,4,4);
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN,8,4);
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN,12,4);
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN,16,4);
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN,20,4);
		
		//unbind with vao
		GLES32.glBindVertexArray(0);
		GLES32.glUseProgram(0);

		//RECTANGLE
		GLES32.glUseProgram(shaderProgramObject);
		GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER,0);

		//-----------------------------------------------------------------------------------------
		
		//initialize above matrices to identity
		Matrix.setIdentityM(modelViewProjectionMatrix, 0);
		Matrix.setIdentityM(modelViewMatrix, 0);
		Matrix.setIdentityM(rotationMatrix, 0);
		Matrix.setIdentityM(translationMatrix, 0);
		
		//do necessary transformation if ant required
		//translateM(float[] m, int mOffset, float x, float y, float z)
		//Translates matrix m by x, y, and z in place.
		//translationMatrix = translate(-1.5f, 0.0f, -6.0f);
		 Matrix.translateM(translationMatrix,0,0.0f,0.0f,-6.0f);

		 //ratationMatrix = rotate(angle_tri, 0.0f, 1.0f, 0.0f);
		//static void	setRotateM(float[] rm, int rmOffset, float a, float x, float y, float z)
		//Creates a matrix for rotation by angle a (in degrees) around the axis (x, y, z).
		 Matrix.setRotateM(rotationMatrix,0,angle_cube_outer,1.0f, 1.0f, 1.0f);
		 //GL_FALSE = internally we are not transposing = row -> col / col -> row
		//OpenGL is col major while DirectX is row major

		//bind with vao(this will avoid many repetitive binding with vbo)
		
		//bind with vao 
		GLES32.glBindVertexArray(vao_cube[0]);

		//do necessary matrix multiplication
		//modelViewMatrix = translationMatrix * ratationMatrix;
		//modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
		Matrix.multiplyMM(modelViewMatrix, 0, translationMatrix, 0,rotationMatrix,0);
		Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0,modelViewMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(mvpUniform,1,false, modelViewProjectionMatrix,0);

		//to change the viewport according to inner cube
		GLES32.glViewport(0,0,gwidth,gheight);

		//bind texture
		GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
		GLES32.glBindTexture(GLES32.GL_TEXTURE_2D,color_texture[0]);

		GLES32.glUniform1i(samplerUniform,0);

		//draw
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN,0,4);
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN,4,4);
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN,8,4);
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN,12,4);
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN,16,4);
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN,20,4);
		
		//unbind with vao
		GLES32.glBindVertexArray(0);

		//unuse
		GLES32.glUseProgram(0);

		//swapbuffer
		requestRender();
	}
	
	private int loadTexture(int imageFileResourceId)
	{
		//to load the texture
		int[] texture = new int[1];
		BitmapFactory.Options  options  = new BitmapFactory.Options();
		options.inScaled = false;
		
		Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(),imageFileResourceId,options);

		GLES32.glGenTextures(1,texture,0);
		GLES32.glBindTexture(GLES32.GL_TEXTURE_2D,texture[0]);

		GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D,GLES32.GL_TEXTURE_MIN_FILTER,GLES32.GL_LINEAR_MIPMAP_LINEAR);
		GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D,GLES32.GL_TEXTURE_MAG_FILTER,GLES32.GL_LINEAR_MIPMAP_LINEAR);
		
		GLUtils.texImage2D(GLES32.GL_TEXTURE_2D, 0, bitmap,0);
		GLES32.glGenerateMipmap(GLES32.GL_TEXTURE_2D);
		
		return (texture[0]);
	}
	private void uninitialize()
	{
		if(vbo_texture_cube[0] != 0)
		{
			GLES32.glDeleteBuffers(1, vbo_texture_cube,0);
			vbo_texture_cube[0] = 0;
		}

		if(vbo_position_cube[0] != 0)
		{
			GLES32.glDeleteBuffers(1, vbo_position_cube,0);
			vbo_position_cube[0] = 0;
		}

		if(vao_cube[0] != 0)
		{
			GLES32.glDeleteVertexArrays(1, vao_cube,0);
			vao_cube[0] = 0;
		}
		
		//safe release
		GLES32.glUseProgram(shaderProgramObject);
		GLES32.glDetachShader(shaderProgramObject, fragmentShaderObject);
		GLES32.glDetachShader(shaderProgramObject, vertexShaderObject);
		GLES32.glDeleteShader(fragmentShaderObject);
		GLES32.glDeleteShader(vertexShaderObject);
		GLES32.glDeleteProgram(shaderProgramObject);
		GLES32.glUseProgram(0);
	}
}