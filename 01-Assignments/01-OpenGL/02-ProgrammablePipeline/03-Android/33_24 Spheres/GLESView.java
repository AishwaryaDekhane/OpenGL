package com.AMD.many_spheres;

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

	private int ModelMatrixUniform_pf;
	private int ViewMatrixUniform_pf;
	private int ProjectionMatrixUniform_pf;
	private int LaUniform_pf;
	private int KaUniform_pf;
	private int LdUniform_pf;
	private int KdUniform_pf;
	private int LsUniform_pf;
	private int KsUniform_pf;
	private int LightPositionUniform_pf;
	private int MaterialShininessUniform_pf;
	private int singleTap;

	private int KeyPress = 0;
	private float radius = 100.0f;

	private float [] LightAmbient = { 0.0f,0.0f,0.0f,0.0f };
	private float [] LightDiffuse = { 1.0f,1.0f,1.0f,1.0f };
	private float [] LightSpecular = { 1.0f,1.0f,1.0f,1.0f };
	private float [] LightPosition = { 100.0f,100.0f,100.0f,1.0f };

	private float [] MaterialAmbient = { 0.0f,0.0f,0.0f,0.0f };
	private float [] MaterialDiffuse = { 1.0f,1.0f,1.0f,1.0f };
	private float [] MaterialSpecular = { 1.0f,1.0f,1.0f,1.0f };
	private float [] MaterialShininess = { 128.0f };

	private float AngleOfXRotation = 0.0f;
	private float AngleOfYRotation = 0.0f;
	private float AngleOfZRotation = 0.0f;

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
		KeyPress++;

		if(KeyPress > 4)
		{
			KeyPress = 1;
		}

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
				"uniform vec4 u_light_position;" +
				"out vec3 phong_ADS_light;" +
				"out vec3 tNorm;" +
				"out vec3 light_direction;" +
				"out vec3 view_vector;" +

				"void main(void)" +
				"{" +
				"if (u_LKeyPressed == 1)" +
				"{" +
				"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;" +
				"tNorm = mat3(u_view_matrix * u_model_matrix) * vNormal;" +
				"light_direction = vec3(u_light_position - eyeCoordinates);" +
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
				"uniform vec3 u_La;" +
					"uniform vec3 u_Ld;" +
					"uniform vec3 u_Ls;" +
					"uniform vec3 u_Ka;" +
					"uniform vec3 u_Kd;" +
					"uniform vec3 u_Ks;" +
					"uniform float u_material_shininess;" +
					"in vec3 phong_ADS_light;" +
					"in vec3 tNorm;" +
					"in vec3 light_direction;" +
					"in vec3 view_vector;" +
					"out vec4 FragColor;" +
					"uniform int u_LKeyPressed;" +

					"void main(void)" +
					"{" +
					"vec4 color;" +
					"if (u_LKeyPressed == 1)" +
					"{" +
					"vec3 normalized_tNorm = normalize(tNorm);" +
					"vec3 normalized_light_direction = normalize(light_direction);" +
					"vec3 normalized_view_vector = normalize(view_vector);" +
					"vec3 reflection_vector = reflect(-normalized_light_direction,normalized_tNorm);" +
					"float tn_dot_ld = max(dot(normalized_light_direction.xyz,normalized_tNorm),0.0f);" +
					"vec3 ambient = u_La * u_Ka;" +
					"vec3 diffuse = u_Ld  * u_Kd * tn_dot_ld;" +
					"vec3 specular = u_Ls  * u_Ks * pow(max(dot(reflection_vector,normalized_view_vector),0.0f),u_material_shininess);" +
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

		ModelMatrixUniform_pf =  GLES32.glGetUniformLocation(shaderProgramObject, "u_model_matrix");
		ProjectionMatrixUniform_pf =  GLES32.glGetUniformLocation(shaderProgramObject, "u_projection_matrix");
		ViewMatrixUniform_pf =  GLES32.glGetUniformLocation(shaderProgramObject, "u_view_matrix");

		singleTap =  GLES32.glGetUniformLocation(shaderProgramObject, "u_LKeyPressed");
		MaterialShininessUniform_pf =  GLES32.glGetUniformLocation(shaderProgramObject, "u_material_shininess");

		LaUniform_pf =  GLES32.glGetUniformLocation(shaderProgramObject, "u_La");
		KaUniform_pf =  GLES32.glGetUniformLocation(shaderProgramObject, "u_Ka");

		LdUniform_pf =  GLES32.glGetUniformLocation(shaderProgramObject, "u_Ld");
		KdUniform_pf =  GLES32.glGetUniformLocation(shaderProgramObject, "u_Kd");

		LsUniform_pf =  GLES32.glGetUniformLocation(shaderProgramObject, "u_Ls");
		KsUniform_pf =  GLES32.glGetUniformLocation(shaderProgramObject, "u_Ks");
		LightPositionUniform_pf =  GLES32.glGetUniformLocation(shaderProgramObject, "u_light_position");

		
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
		AngleOfXRotation = AngleOfXRotation + 0.5f;
		if (AngleOfXRotation >= 360.0f)
			AngleOfXRotation = AngleOfXRotation - 360.0f;

			AngleOfYRotation = AngleOfYRotation + 0.005f;
			if (AngleOfYRotation >= 360.0f)
				AngleOfYRotation = AngleOfYRotation - 360.0f;
		
			AngleOfZRotation = AngleOfZRotation + 0.005f;
			if (AngleOfZRotation >= 360.0f)
				AngleOfZRotation = AngleOfZRotation - 360.0f;
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
		//-------------------- 1st sphere on 1st column, emerald ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.6f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.0215f,0.1745f,0.0215f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.07568f,0.61424f,0.07568f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.633f,0.727811f,0.633f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, -3.5f, 2.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 2nd sphere on 1st column, jade ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.1f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf,0.135f,0.2225f,0.1575f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.54f,0.89f, 0.63f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.316228f,
				0.316228f,
				0.316228f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, -3.5f, 1.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 3rd sphere on 1st column, obsidian ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.3f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.05375f,
				0.05f,    
				0.06625f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.18275f,
				0.17f,    
				0.22525f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.332741f,
				0.328634f,
				0.346435f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, -3.5f, 0.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 4th sphere on 1st column, pearl ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.088f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.25f,
				0.20725f,
				0.20725f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 1.0f,   
				0.829f,
				0.829f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.296648f,
				0.296648f,
				0.296648f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, -3.5f, -0.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 5th sphere on 1st column, ruby ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.6f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.1745f,
				0.01175f,
				0.01175f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.61424f,
				0.04136f,
				0.04136f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.727811f,
				0.626959f,
				0.626959f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, -3.5f, -1.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 6th sphere on 1st column, turquoise ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.1f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.1f,   
				0.18725f,
				0.1745f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.396f,
				0.74151f,
				0.69102f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf,0.297254f,
				0.30829f,  
				0.306678f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, -3.5f, -2.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 7- 1st sphere on 2nd column, brass ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.21794872f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.329412f,
				0.223529f,
				0.027451f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.780392f,
				0.568627f,
				0.113725f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.992157f,
				0.941176f,
				0.807843f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, -1.3f, 2.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 8- 2nd sphere on 2nd column, bronze ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.2f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.2125f,
				0.1275f,
				0.054f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.714f,
				0.4284f,  
				0.18144f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.393548f,
				0.271906f,
				0.166721f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, -1.3f, 1.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 9- 3rd sphere on 2nd column, chrome -----------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.6f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.25f,
				0.25f,
				0.25f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.4f, 
				0.4f,
				0.4f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.774597f,
				0.774597f,
				0.774597f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, -1.3f, 0.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 10- 4th sphere on 2nd column, copper -----------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.1f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.19125f,
				0.0735f,
				0.0225f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.7038f,  
				0.27048f,
				0.0828f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.256777f,
				0.137622f,
				0.086014f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, -1.3f, -0.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 11- 5th sphere on 2nd column, gold ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.4f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.24725f,
				0.1995f, 
				0.0745f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.75164f,
				0.60648f,
				0.22648f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.628281f,
				0.555802f,
				0.366065f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, -1.3f, -1.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 12- 6th sphere on 2nd column, silver ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.4f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.19225f,
				0.19225f,
				0.19225f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.50754f,
				0.50754f,
				0.50754f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.508273f,
				0.508273f,
				0.508273f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, -1.3f, -2.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 13- 1st sphere on 3rd column, black plastic ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.25f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.0f,  
				0.0f,
				0.0f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.01f,
				0.01f,
				0.01f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.50f,
				0.50f,
				0.50f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, 1.3f, 2.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 14- 2nd sphere on 3rd column, cyan plastic ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.25f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.0f,
				0.1f,
				0.06f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.0f,        
				0.50980392f,
				0.50980392f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.50196078f,
				0.50196078f,
				0.50196078f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, 1.3f, 1.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 15- 3rd sphere on 2nd column, green plastic------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.25f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.0f,  
				0.0f,
				0.0f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.1f,  
				0.35f,
				0.1f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.45f,
				0.55f,
				0.45f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, 1.3f, 0.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 16- 4th sphere on 3rd column, red plastic------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.25f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.0f, 
				0.0f,
				0.0f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.5f,
				0.0f,
				0.0f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.7f,
				0.6f,
				0.6f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, 1.3f, -0.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 17- 5th sphere on 3rd column, white plastic------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.25f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.0f,
				0.0f,
				0.0f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.55f,
				0.55f,
				0.55f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.70f,
				0.70f,
				0.70f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, 1.3f, -1.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 18- 6th sphere on 3rd column, yellow plastic ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.25f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.0f,
				0.0f,
				0.0f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.5f,
				0.5f,
				0.0f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.60f,
				0.60f,
				0.50f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, 1.3f, -2.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 19- 1st sphere on 4th column, black rubber ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.078125f * 128);

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf,0.02f,
				0.02f,
				0.02f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.01f,
				0.01f,
				0.01f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.4f,  
				0.4f,  
				0.4f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, 3.5f, 2.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 20- 2nd sphere on 4th column, cyan rubber ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.078125f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.0f, 
				0.05f,
				0.05f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.4f,  
				0.5f,
				0.5f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.04f,
				0.7f,
				0.7f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, 3.5f, 1.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 21- 3rd sphere on 4th column, green rubber ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.078125f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.0f,
				0.05f,
				0.0f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.4f,
				0.5f,
				0.4f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.04f,
				0.7f,
				0.04f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, 3.5f, 0.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
	//-------------------- 22- 4th sphere on 4th column, red rubber------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf, 0.078125f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.05f,
				0.0f,
				0.0f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.5f, 
				0.4f,
				0.4f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.7f,
				0.04f,
				0.04f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, 3.5f, -0.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 23- 5th sphere on 4th column, white ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.078125f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.05f,
				0.05f,
				0.05f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.5f,
				0.5f,
				0.5f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.7f,
				0.7f,
				0.7f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, 3.5f, -1.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
		 GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
		 GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
		
		//unbind va0
		GLES32.glBindVertexArray(0);
		//----------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------
		//-------------------- 24- 6th sphere on 4th column, yellow rubber ------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pf,  0.078125f * 128 );

			GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.05f,
				0.05f,
				0.0f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf, 0.5f,
				0.5f,
				0.4f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf, 0.7f,  
				0.7f,
				0.04f);

			if (KeyPress == 1)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfXRotation, 1.0f, 0.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfXRotation)) * radius),(float)((Math.sin(AngleOfXRotation)) * radius),0.0f);
	
			}
			else if (KeyPress == 2)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfYRotation, 0.0f, 1.0f, 0.0f);
				GLES32.glUniform4f(LightPositionUniform_pf,0.0f,(float) ((Math.cos(AngleOfYRotation)) * radius),(float)((Math.sin(AngleOfYRotation)) * radius),0.0f);
			}
			else if (KeyPress == 3)
			{
				Matrix.setRotateM(rotationMatrix , 0, AngleOfZRotation, 0.0f, 0.0f, 1.0f);
				GLES32.glUniform4f(LightPositionUniform_pf, 0.0f,(float) ((Math.sin(AngleOfZRotation)) * radius),(float) ((Math.cos(AngleOfZRotation)) * radius),0.0f);
			}
		}
		else
		{
			GLES32.glUniform1i(singleTap, 0);
		}

		//do necessary transformation if ant required
		Matrix.translateM(translationMatrix, 0, 3.5f, -2.5f, -11.0f);

		//do necessary matrix multiplication
		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  translationMatrix,0);	

		Matrix.multiplyMM(modelMatrix,0,
						  modelMatrix,0,
						  rotationMatrix,0);	 

		Matrix.multiplyMM(projectionMatrix,0,
						  projectionMatrix,0,
						  perspectiveProjectionMatrix,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(ModelMatrixUniform_pf, 1,false, modelMatrix, 0);
		GLES32.glUniformMatrix4fv(ProjectionMatrixUniform_pf, 1, false, projectionMatrix, 0);
		GLES32.glUniformMatrix4fv(ViewMatrixUniform_pf, 1, false, viewMatrix, 0);

		//bind with vao(this will avoid many repetitive binding with vbo)
		 GLES32.glBindVertexArray(vao_sphere[0]);
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
