package com.AMD.rlight;

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
	private int shaderProgramObject;		//java did'nt have private int/uint

	//we did'nt have address in java so pass array of one
	private int [] vao_sphere = new int [1];
	private int[] vbo_sphere_position = new int[1];
    private int[] vbo_sphere_normal = new int[1];
    private int[] vbo_sphere_element = new int[1];

	private int ModelMatrixUniform_pf;
	private int ViewMatrixUniform_pf;
	private int ProjectionMatrixUniform_pf;
	private int MaterialShininessUniform_pf;
	private int LKeyPressedUniform_pf;
	private int KaUniform_pf;
	private int KdUniform_pf;
	private int KsUniform_pf;

	private int LaUniform_pf_zero;
	private int LdUniform_pf_zero;
	private int LsUniform_pf_zero;
	private int LightPositionUniform_pf_zero;

	private int LaUniform_pf_one;
	private int LdUniform_pf_one;
	private int LsUniform_pf_one;
	private int LightPositionUniform_pf_one;

	private int LaUniform_pf_two;
	private int LdUniform_pf_two;
	private int LsUniform_pf_two;
	private int LightPositionUniform_pf_two;

	//LIGHT0
	private float [] LightAmbientZero = { 0.0f,0.0f,0.0f,1.0f };
	private float [] LightDiffuseZero = { 1.0f,0.0f,0.0f,1.0f };
	private float [] LightSpecularZero = { 1.0f,0.0f,0.0f,1.0f };
	private float [] LightPositionZero = { 100.0f,0.0f,0.0f,1.0f };

	//LIGHT1
	private float [] LightAmbientOne = { 0.0f,0.0f,0.0f,1.0f };
	private float [] LightDiffuseOne = { 0.0f,1.0f,0.0f,1.0f };
	private float [] LightSpecularOne = { 0.0f,1.0f,0.0f,1.0f };
	private float [] LightPositionOne = { 0.0f,-100.0f,0.0f,1.0f };

	//LIGHT2
	private float [] LightAmbientTwo = { 0.0f,0.0f,0.0f,1.0f };
	private float [] LightDiffuseTwo = { 0.0f,0.0f,1.0f,1.0f };
	private float [] LightSpecularTwo = { 0.0f,0.0f,1.0f,1.0f };
	private float [] LightPositionTwo = { 0.0f,0.0f,100.0f,1.0f };

	private float [] MaterialAmbient = { 0.0f,0.0f,0.0f,1.0f };
	private float [] MaterialDiffuse = { 1.0f,1.0f,1.0f,1.0f };
	private float [] MaterialSpecular = { 1.0f,1.0f,1.0f,1.0f };
	private float [] MaterialShininess = { 128.0f };

	private float LightAngleZero = 0.0f;
	private float LightAngleOne = 0.0f;
	private float LightAngleTwo = 0.0f;

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
				"out vec3 phong_ADS_light;" +
				"out vec3 tNorm;" +
				"out vec3 view_vector;" +

				"uniform vec4 u_light_position_zero;" +
				"out vec3 light_direction_zero;" +

				"uniform vec4 u_light_position_one;" +
				"out vec3 light_direction_one;" +

				"uniform vec4 u_light_position_two;" +
				"out vec3 light_direction_two;" +
				

				"void main(void)" +
				"{" +
				"if (u_LKeyPressed == 1)" +
				"{" +
				"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;" +
				"tNorm = mat3(u_view_matrix * u_model_matrix) * vNormal;" +

				"light_direction_zero = vec3(u_light_position_zero - eyeCoordinates);" +
				"light_direction_one = vec3(u_light_position_one - eyeCoordinates);" +
				"light_direction_two = vec3(u_light_position_two - eyeCoordinates);" +

				"vec3 viewer_vector = vec3(-eyeCoordinates.xyz);" +

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
				"uniform vec3 u_Ka;" +
				"uniform vec3 u_Kd;" +
				"uniform vec3 u_Ks;" +
				"uniform float u_material_shininess;" +
				"in vec3 phong_ADS_light;" +
				"in vec3 tNorm;" +
				"out vec4 FragColor;" +
				"uniform int u_LKeyPressed;" +
				"in vec3 view_vector;" +

				"uniform vec3 u_La_zero;" +
				"uniform vec3 u_Ld_zero;" +
				"uniform vec3 u_Ls_zero;" +
				"in vec3 light_direction_zero;" +

				"uniform vec3 u_La_one;" +
				"uniform vec3 u_Ld_one;" +
				"uniform vec3 u_Ls_one;" +
				"in vec3 light_direction_one;" +

				"uniform vec3 u_La_two;" +
				"uniform vec3 u_Ld_two;" +
				"uniform vec3 u_Ls_two;" +
				"in vec3 light_direction_two;" +
				

				"void main(void)" +
				"{" +
				"vec4 color;" +
				"if (u_LKeyPressed == 1)" +
				"{" +
				"vec3 normalized_tNorm = normalize(tNorm);" +
				"vec3 normalized_view_vector = normalize(view_vector);" +

				"vec3 normalized_light_direction_zero = normalize(light_direction_zero);" +
				"vec3 reflection_vector_zero = reflect(-normalized_light_direction_zero,normalized_tNorm);" +
				"float tn_dot_ld_zero = max(dot(normalized_light_direction_zero.xyz,normalized_tNorm),0.0f);" +

				"vec3 normalized_light_direction_one = normalize(light_direction_one);" +
				"vec3 reflection_vector_one = reflect(-normalized_light_direction_one,normalized_tNorm);" +
				"float tn_dot_ld_one = max(dot(normalized_light_direction_one.xyz,normalized_tNorm),0.0f);" +

				"vec3 normalized_light_direction_two = normalize(light_direction_two);" +
				"vec3 reflection_vector_two = reflect(-normalized_light_direction_two,normalized_tNorm);" +
				"float tn_dot_ld_two = max(dot(normalized_light_direction_two.xyz,normalized_tNorm),0.0f);" +

				"vec3 ambient = u_La_zero * u_Ka + u_La_one * u_Ka + u_La_two * u_Ka;" +
				"vec3 diffuse = u_Ld_zero  * u_Kd * tn_dot_ld_zero + u_Ld_one  * u_Kd * tn_dot_ld_one + u_Ld_two  * u_Kd * tn_dot_ld_two;" +
				"vec3 specular = u_Ls_zero  * u_Ks * pow(max(dot(reflection_vector_zero,normalized_view_vector),0.0f),u_material_shininess) + u_Ls_one  * u_Ks * pow(max(dot(reflection_vector_one,normalized_view_vector),0.0f),u_material_shininess) + u_Ls_two  * u_Ks * pow(max(dot(reflection_vector_two,normalized_view_vector),0.0f),u_material_shininess);" +
				"vec3 phong_ADS_light = ambient + diffuse + specular;" +

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

		ModelMatrixUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject, "u_model_matrix");
		ProjectionMatrixUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject, "u_projection_matrix");
		ViewMatrixUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject, "u_view_matrix");
	
		LKeyPressedUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject, "u_LKeyPressed");
		MaterialShininessUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject, "u_material_shininess");
	
		LaUniform_pf_zero = GLES32.glGetUniformLocation(shaderProgramObject, "u_La_zero");
		LaUniform_pf_one = GLES32.glGetUniformLocation(shaderProgramObject, "u_La_one");
		LaUniform_pf_two = GLES32.glGetUniformLocation(shaderProgramObject, "u_La_two");
		KaUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ka");
	
		LdUniform_pf_zero = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ld_zero");
		LdUniform_pf_one = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ld_one");
		LdUniform_pf_two = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ld_two");
		KdUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject, "u_Kd");
	
		LsUniform_pf_zero = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ls_zero");
		LsUniform_pf_one = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ls_one");
		LsUniform_pf_two = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ls_two");
		KsUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ks");
	
		LightPositionUniform_pf_zero = GLES32.glGetUniformLocation(shaderProgramObject, "u_light_position_zero");
		LightPositionUniform_pf_one = GLES32.glGetUniformLocation(shaderProgramObject, "u_light_position_one");
		LightPositionUniform_pf_two = GLES32.glGetUniformLocation(shaderProgramObject, "u_light_position_two");
	

		
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
		LightAngleZero = LightAngleZero + 0.005f;
	if (LightAngleZero >= 360.0f)
		LightAngleZero = LightAngleZero - 360.0f;

	LightAngleOne = LightAngleOne + 0.005f;
	if (LightAngleOne >= 360.0f)
		LightAngleOne = LightAngleOne - 360.0f;

	LightAngleTwo = LightAngleTwo + 0.005f;
	if (LightAngleTwo >= 360.0f)
		LightAngleTwo = LightAngleTwo - 360.0f;
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
		int radius = 100;

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
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);

		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		//glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);
		//GL_FALSE = internally we are not transposing = row -> col / col -> row
		//OpenGL is col major while DirectX is row major


		if (gbLight == true)
		{
			GLES32.glUniform1i(LKeyPressedUniform_pf, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf, 128.0f);

			GLES32.glUniform3f(LaUniform_pf_zero, 0.0f,0.0f,0.0f);
			GLES32.glUniform3f(LaUniform_pf_one, 0.0f,0.0f,0.0f);
			GLES32.glUniform3f(LaUniform_pf_two, 0.0f,0.0f,0.0f);
			GLES32.glUniform3f(KaUniform_pf, 0.0f,0.0f,0.0f);

			GLES32.glUniform3f(LdUniform_pf_zero, 1.0f,0.0f,0.0f);
			GLES32.glUniform3f(LdUniform_pf_one, 0.0f,1.0f,0.0f);
			GLES32.glUniform3f(LdUniform_pf_two,  0.0f,0.0f,1.0f);
			GLES32.glUniform3f(KdUniform_pf,  1.0f,1.0f,1.0f);

			GLES32.glUniform3f(LsUniform_pf_zero, 1.0f,0.0f,0.0f);
			GLES32.glUniform3f(LsUniform_pf_one, 0.0f,1.0f,0.0f);
			GLES32.glUniform3f(LsUniform_pf_two,  0.0f,0.0f,1.0f);
			GLES32.glUniform3f(KsUniform_pf, 1.0f,1.0f,1.0f);

			/*LightPositionZero[0] = 0.0f;
			LightPositionZero[1] = Math.cos(LightAngleZero) * radius;
			LightPositionZero[2] = Math.sin(LightAngleZero) * radius;
			LightPositionZero[3] = 0.0f;

			LightPositionOne[0] = Math.cos(LightAngleOne) * radius;
			LightPositionOne[1] = 0.0f;
			LightPositionOne[2] = Math.sin(LightAngleOne) * radius;
			LightPositionOne[3] = 0.0f;

			LightPositionTwo[0] = 0.0f;
			LightPositionTwo[1] = Math.sin(LightAngleTwo) * radius;
			LightPositionTwo[2] = Math.cos(LightAngleTwo) * radius;
			LightPositionTwo[3] = 0.0f;*/

			GLES32.glUniform4f(LightPositionUniform_pf_zero,0.0f,(float) ((Math.cos(LightAngleZero)) * radius),(float)((Math.sin(LightAngleZero)) * radius),0.0f);
			GLES32.glUniform4f(LightPositionUniform_pf_one,(float) ((Math.cos(LightAngleZero)) * radius),0.0f,(float)((Math.sin(LightAngleZero)) * radius),0.0f);
			GLES32.glUniform4f(LightPositionUniform_pf_two, 0.0f,(float) ((Math.sin(LightAngleZero)) * radius),(float) ((Math.cos(LightAngleZero)) * radius),0.0f);
		}
		else
		{
			GLES32.glUniform1i(LKeyPressedUniform_pf, 0);
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
