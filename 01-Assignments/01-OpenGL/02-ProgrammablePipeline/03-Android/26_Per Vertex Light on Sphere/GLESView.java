package com.AMD.per_vertex;

//packages added by me

//packages added for PP Blue Window Code
import android.opengl.GLSurfaceView;
import android.opengl.GLES32;				//opengl 3.2 version
import javax.microedition.khronos.opengles.GL10;		//opengl extension starts desktop 1.0 , J2ME = Java to microedition
import javax.microedition.khronos.egl.EGLConfig;		//Embedded graphic Library
import java.nio.ShortBuffer; //for sphere
		
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
	private int [] vao_sphere = new int [1];
	private int[] vbo_sphere_position = new int[1];
    private int[] vbo_sphere_normal = new int[1];
    private int[] vbo_sphere_element = new int[1];

	private int ModelMatrixUniform;
	private int ViewMatrixUniform;
	private int ProjectionMatrixUniform;
	private int LaUniform;
	private int KaUniform;
	private int LdUniform;
	private int KdUniform;
	private int LsUniform;
	private int KsUniform;
	private int LightPositionUniform;
	private int MaterialShininessUniform;
	private int singleTap;

	private float [] LightAmbient = { 0.0f,0.0f,0.0f,0.0f };
	private float [] LightDiffuse = { 1.0f,1.0f,1.0f,1.0f };
	private float [] LightSpecular = { 1.0f,1.0f,1.0f,1.0f };
	private float [] LightPosition = { 100.0f,100.0f,100.0f,1.0f };

	private float [] MaterialAmbient = { 0.0f,0.0f,0.0f,0.0f };
	private float [] MaterialDiffuse = { 1.0f,1.0f,1.0f,1.0f };
	private float [] MaterialSpecular = { 1.0f,1.0f,1.0f,1.0f };
	private float [] MaterialShininess = { 128.0f };

	private float angle_sphere = 0.0f;

	private float[] perspectiveProjectionMatrix = new float[16];		//4*4 matrix

	boolean gbAnimate = false;
	boolean gbLight = false;

	private int numElements;
	private int numVertices;

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
        //setText("Double Tap")

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
			/*if (gbAnimate == false)
			{
				gbAnimate = true;
			}
			else
			{
				gbAnimate = false;
			}*/

			if (gbLight == false)
			{
				gbLight = true;
			}
			else
			{
				gbLight = false;
			}
			
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
		/*if(gbAnimate == true)
		{
				update();
		}*/
		update();			
        display();
    }


	//our custom methods
	private void initialize()
	{
		//function declaration

		//VERTEX SHADER
		vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

		final String vertexShaderSourceCode = String.format
		(
			"#version 320 es" +
				"\n" +
				"precision mediump int;" +
				"in vec4 vPosition;" +
				"in vec3 vNormal;" +
				"uniform mat4 u_model_matrix;" +
				"uniform mat4 u_projection_matrix;" +
				"uniform mat4 u_view_matrix;" +
				"uniform int u_LKeyPressed;" +
				"uniform vec3 u_La;" +
				"uniform vec3 u_Ld;" +
				"uniform vec3 u_Ls;" +
				"uniform vec3 u_Ka;" +
				"uniform vec3 u_Kd;" +
				"uniform vec3 u_Ks;" +
				"uniform float u_matrial_shininess;" +
				"uniform vec4 u_light_position;" +
				"out vec3 phong_ADS_light;" +
				"void main(void)" +
				"{" +
				"if (u_LKeyPressed == 1)" +
				"{" +
				"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;" +
				"vec3 tNorm = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);" +
				"vec3 light_direction = normalize(vec3(u_light_position - eyeCoordinates));" +
				"float tn_dot_Ld = max(dot(light_direction, tNorm), 0.0);" +
				"vec3 reflection_vector = reflect(-light_direction, tNorm);" +
				"vec3 viewer_vector = normalize(vec3(-eyeCoordinates.xyz));" +
				"vec3 ambient = u_La * u_Ka;" +
				"vec3 diffuse = u_Ld * u_Kd * tn_dot_Ld;" +
				"vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, viewer_vector), 0.0), u_matrial_shininess);" +
				"phong_ADS_light = ambient + diffuse + specular;" +
				"}" +
				"else" +
				"{" +
				"phong_ADS_light = vec3(1.0, 1.0, 1.0);" +
				"}" +
				"gl_Position = u_projection_matrix * u_model_matrix * u_view_matrix * vPosition;" +
				"}"
		);

		//this line internally calls NDK functions
		//specify above source code to vertex shader object
		GLES32.glShaderSource(vertexShaderObject,vertexShaderSourceCode);

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
		String szInfoLog = null;

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
			"in vec3 phong_ADS_light;" +
			"out vec4 FragColor;" +
			"uniform int u_LKeyPressed;" +
			"void main(void)" +
			"{" +
			"vec4 color;" +
			"if (u_LKeyPressed == 1)" +
			"{" +
			"color = vec4(phong_ADS_light,1.0);" +
			"}" +
			"else" +
			"{" +
			"color = vec4(1.0, 1.0, 1.0, 1.0);" +
			"}" +
			"FragColor = color;" +
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
		GLES32.glAttachShader(shaderProgramObject, fragmentShaderObject);

		//Prelinking binding to vertex attributes
		GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
		GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

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

					System.out.println("RTR: Fragemnt Shader Compilation Log" + szInfoLog);
			
					uninitialize();
				
					System.exit(0);		
			}
		}

		//--------------------------------------------------------------------------------------

		//Postlinking retriving uniform locations

		ModelMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_model_matrix");
		ProjectionMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_projection_matrix");
		ViewMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_view_matrix");

		singleTap = GLES32.glGetUniformLocation(shaderProgramObject, "u_LKeyPressed");
		MaterialShininessUniform = GLES32.glGetUniformLocation(shaderProgramObject,"u_matrial_shininess");

		LaUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_La");
		KaUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ka");

		LdUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ld");
		KdUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Kd");

		LsUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ls");
		KsUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ks");
		LightPositionUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_light_position");

		
	//------------------------------------------------------------------------------------------------------------------------------------------
	
	initializeSphere();
	
	//-------------------------------------------------------------------------------------------------------------
		//for background color
		GLES32.glClearColor(0.0f,0.0f,0.0f,1.0f);

		//For Depth

		//GLES32.glClearDepth(1.0f);	//give existance to depth buffer

		GLES32.glEnable(GLES32.GL_DEPTH_TEST);	//for 3D geometry enable depth test i.e z axis 

		GLES32.glDepthFunc(GLES32.GL_LEQUAL);		//LEQUAL = less than or equal to test compared to the max value which is set to 1.0f

		//identity() is in mat4
		Matrix.setIdentityM(perspectiveProjectionMatrix, 0);

		//gbAnimate = false;
		//gbLight = false;

		//no warmup call to resize as we are already inn fullscreen

		//for texture
		//GLES32.glEnable(GLES32.GL_CULL_FACE);

	}

	private void update()
	{
		angle_sphere = angle_sphere + 0.5f;
		if (angle_sphere >= 360.0f)
			angle_sphere = angle_sphere - 360.0f;
	}


	private void initializeSphere()
	{
		Sphere sphere=new Sphere();
        float sphere_vertices[]=new float[1146];
        float sphere_normals[]=new float[1146];
        float sphere_textures[]=new float[764];
        short sphere_elements[]=new short[2280];
        sphere.getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
        numVertices = sphere.getNumberOfSphereVertices();
        numElements = sphere.getNumberOfSphereElements();

		// vao
        GLES32.glGenVertexArrays(1,vao_sphere,0);
        GLES32.glBindVertexArray(vao_sphere[0]);
        
        // position vbo
        GLES32.glGenBuffers(1,vbo_sphere_position,0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,vbo_sphere_position[0]);
        
        ByteBuffer byteBuffer=ByteBuffer.allocateDirect(sphere_vertices.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        FloatBuffer verticesBuffer=byteBuffer.asFloatBuffer();
        verticesBuffer.put(sphere_vertices);
        verticesBuffer.position(0);
        
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                            sphere_vertices.length * 4,
                            verticesBuffer,
                            GLES32.GL_STATIC_DRAW);
        
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION,
                                     3,
                                     GLES32.GL_FLOAT,
                                     false,0,0);
        
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,0);
        
        // normal vbo
        GLES32.glGenBuffers(1,vbo_sphere_normal,0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,vbo_sphere_normal[0]);
        
        byteBuffer=ByteBuffer.allocateDirect(sphere_normals.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        verticesBuffer=byteBuffer.asFloatBuffer();
        verticesBuffer.put(sphere_normals);
        verticesBuffer.position(0);
        
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                            sphere_normals.length * 4,
                            verticesBuffer,
                            GLES32.GL_STATIC_DRAW);
        
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_NORMAL,
                                     3,
                                     GLES32.GL_FLOAT,
                                     false,0,0);
        
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_NORMAL);
        
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,0);
        
        // element vbo
        GLES32.glGenBuffers(1,vbo_sphere_element,0);
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER,vbo_sphere_element[0]);
        
        byteBuffer=ByteBuffer.allocateDirect(sphere_elements.length * 2);
        byteBuffer.order(ByteOrder.nativeOrder());
        ShortBuffer elementsBuffer=byteBuffer.asShortBuffer();
        elementsBuffer.put(sphere_elements);
        elementsBuffer.position(0);
        
        GLES32.glBufferData(GLES32.GL_ELEMENT_ARRAY_BUFFER,
                            sphere_elements.length * 2,
                            elementsBuffer,
                            GLES32.GL_STATIC_DRAW);
        
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER,0);

        GLES32.glBindVertexArray(0);
	}


	private void resize(int width, int height)
	{
		//code
		if (height <= 0)
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

		GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT |GLES32.GL_DEPTH_BUFFER_BIT);

		//binding
		GLES32.glUseProgram(shaderProgramObject);

		//----------------------------------------------------------------------------------------------------------

		// OpenGL Drawing
		//declaration of matrices
		float [] modelMatrix = new float[16];
		float [] viewMatrix = new float[16];
		float [] translationMatrix = new float[16];
		float [] rotationMatrix = new float[16];
		float [] projectionMatrix = new float[16];

		//--------------------------------------------------------------------------------------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);


		//do necessary transformation if ant required
		//translateM(float[] m, int mOffset, float x, float y, float z)
		//Translates matrix m by x, y, and z in place.
		//translationMatrix = translate(-1.5f, 0.0f, -6.0f);
		Matrix.translateM(translationMatrix, 0, 0.0f, 0.0f, -4.0f);

		//ratationMatrix = rotate(angle_sphere, 1.0f, 1.0f, 1.0f);
		//static void	setRotateM(float[] rm, int rmOffset, float a, float x, float y, float z)
		//Creates a matrix for rotation by angle a (in degrees) around the axis (x, y, z).
		//Matrix.setRotateM(rotationMatrix , 0, angle_sphere, 1.0f, 0.0f, 0.0f);

		//do necessary matrix multiplication
		//modelViewMatrix = translationMatrix * ratationMatrix;
		//modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		/*Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 */ 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform, 1,false, modelMatrix, 0);

		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform, 1, false, projectionMatrix, 0);
		//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

		GLES32.glUniformMatrix4fv(ViewMatrixUniform, 1, false, viewMatrix, 0);
		//GL_FALSE = internally we are not transposing = row -> col / col -> row
		//OpenGL is col major while DirectX is row major

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform,  128.0f );

			GLES32.glUniform3f(LaUniform, 0.0f,0.0f,0.0f);
			GLES32.glUniform3f(KaUniform, 0.0f,0.0f,0.0f);

			GLES32.glUniform3f(LdUniform, 1.0f,1.0f,1.0f);
			GLES32.glUniform3f(KdUniform,  1.0f,1.0f,1.0f);

			GLES32.glUniform3f(LsUniform, 1.0f,1.0f,1.0f);
			GLES32.glUniform3f(KsUniform,  1.0f,1.0f,1.0f);

			GLES32.glUniform4f(LightPositionUniform,100.0f,100.0f,100.0f,1.0f);
			}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}


		//bind with vao(this will avoid many repetitive binding with vbo)
		 // bind vao
		 GLES32.glBindVertexArray(vao_sphere[0]);

		//similarly bind the textures if any
		
		//draw necessary scene

		 // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------
	
		//unbinding - unused program
		GLES32.glUseProgram(0);

		requestRender();		//SwapBuffers()
	}	

	void uninitialize()
	{
		//code
		
		 // destroy vao
		 if(vao_sphere[0] != 0)
		 {
			 GLES32.glDeleteVertexArrays(1, vao_sphere, 0);
			 vao_sphere[0]=0;
		 }
		 
		 // destroy position vbo
		 if(vbo_sphere_position[0] != 0)
		 {
			 GLES32.glDeleteBuffers(1, vbo_sphere_position, 0);
			 vbo_sphere_position[0]=0;
		 }
		 
		 // destroy normal vbo
		 if(vbo_sphere_normal[0] != 0)
		 {
			 GLES32.glDeleteBuffers(1, vbo_sphere_normal, 0);
			 vbo_sphere_normal[0]=0;
		 }

		 // destroy element vbo
		 if(vbo_sphere_element[0] != 0)
		 {
			 GLES32.glDeleteBuffers(1, vbo_sphere_element, 0);
			 vbo_sphere_element[0]=0;
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
