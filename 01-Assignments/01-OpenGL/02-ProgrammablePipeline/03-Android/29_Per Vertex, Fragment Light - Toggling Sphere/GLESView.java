package com.AMD.toggling;

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
	private int vertxShaderObject_pv;
	private int fragmentShaderObject_pv;
	private int ShaderProgramObject_pv;		//java did'nt have private int/uint

	private int vertexShaderObject_pf;
	private int fragmentShaderObject_pf;
	private int ShaderProgramObject_pf;

	//we did'nt have address in java so pass array of one
	private int [] vao_sphere = new int [1];
	private int[] vbo_sphere_position = new int[1];
    private int[] vbo_sphere_normal = new int[1];
    private int[] vbo_sphere_element = new int[1];

	private int modelMatrix_pvUniform_pv;
	private int viewMatrix_pvUniform_pv;
	private int projectionMatrix_pvUniform_pv;
	private int LaUniform_pv;
	private int KaUniform_pv;
	private int LdUniform_pv;
	private int KdUniform_pv;
	private int LsUniform_pv;
	private int KsUniform_pv;
	private int LightPositionUniform_pv;
	private int MaterialShininessUniform_pv;
	private int singleTap_pv;
	
	private int modelMatrix_pvUniform_pf;
	private int viewMatrix_pvUniform_pf;
	private int projectionMatrix_pvUniform_pf;
	private int LaUniform_pf;
	private int KaUniform_pf;
	private int LdUniform_pf;
	private int KdUniform_pf;
	private int LsUniform_pf;
	private int KsUniform_pf;
	private int LightPositionUniform_pf;
	private int MaterialShininessUniform_pf;
	private int singleTap_pf;

	private float [] LightAmbient = { 0.0f,0.0f,0.0f,0.0f };
	private float [] LightDiffuse = { 1.0f,1.0f,1.0f,1.0f };
	private float [] LightSpecular = { 1.0f,1.0f,1.0f,1.0f };
	private float [] LightPosition = { 100.0f,100.0f,100.0f,1.0f };

	private float [] MaterialAmbient = { 0.0f,0.0f,0.0f,0.0f };
	private float [] MaterialDiffuse = { 1.0f,1.0f,1.0f,1.0f };
	private float [] MaterialSpecular = { 1.0f,1.0f,1.0f,1.0f };
	private float [] MaterialShininess = { 128.0f };

	private float angle_sphere = 0.0f;

	private float[] perspectiveprojectionMatrix_pv = new float[16];		//4*4 matrix

	boolean gbAnimate = false;
	boolean gbLight = false;
	boolean gbVertex = false;
	boolean gbFragment = false;

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
			if (gbVertex == false)
			{
				gbVertex = true;
			}
			else
			{
				gbVertex = false;
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
		if (gbFragment == false)
			{
				gbFragment = true;
			}
			else
			{
				gbFragment = false;
			}
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

		//VERTEX SHADER FOR PV
		vertxShaderObject_pv = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

		final String vertexShaderSourceCode_pv = String.format
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
		GLES32.glShaderSource(vertxShaderObject_pv,vertexShaderSourceCode_pv);

		//compile the vertex shader
		GLES32.glCompileShader(vertxShaderObject_pv);

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

		int [] iShaderCompileStatus_pv = new int [1];
		int [] iInfoLogLength_pv = new int [1];
		String szInfoLog_pv = null;

		//step 1
		GLES32.glGetShaderiv(vertxShaderObject_pv, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus_pv, 0);


		//step 2
		if (iShaderCompileStatus_pv[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(vertxShaderObject_pv, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength_pv, 0);

			if (iInfoLogLength_pv[0] > 0)
			{
				szInfoLog_pv = GLES32.glGetShaderInfoLog(vertxShaderObject_pv);

				System.out.println("RTR: Vertex Shader Compilation Log" + szInfoLog_pv);
			
				uninitialize();
				
				System.exit(0);
			}
		}

		//VERTEX SHADER FOR PF
		vertexShaderObject_pf = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

		final String vertexShaderSourceCode_pf = String.format
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
		GLES32.glShaderSource(vertexShaderObject_pf,vertexShaderSourceCode_pf);

		//compile the vertex shader
		GLES32.glCompileShader(vertexShaderObject_pf);

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

		int [] iShaderCompileStatus_pf = new int [1];
		int [] iInfoLogLength_pf = new int [1];
		String szInfoLog_pf = null;

		//step 1
		GLES32.glGetShaderiv(vertexShaderObject_pf, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus_pf, 0);


		//step 2
		if (iShaderCompileStatus_pf[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(vertexShaderObject_pf, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength_pf, 0);

			if (iInfoLogLength_pf[0] > 0)
			{
				szInfoLog_pf = GLES32.glGetShaderInfoLog(vertexShaderObject_pf);

				System.out.println("RTR: Vertex Shader Compilation Log" + szInfoLog_pf);
			
				uninitialize();
				
				System.exit(0);
			}
		}


		//FRAGMENT SHADER for PV

		iShaderCompileStatus_pv[0] = 0;
		iInfoLogLength_pv[0] = 0;
		szInfoLog_pv = null;

		fragmentShaderObject_pv = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);

		final String fragmentShaderSourceCode_pv = String.format
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
		GLES32.glShaderSource(fragmentShaderObject_pv,fragmentShaderSourceCode_pv);

		//compile the vertex shader
		GLES32.glCompileShader(fragmentShaderObject_pv);

		//Error checking for FS

		//step 1
		GLES32.glGetShaderiv(fragmentShaderObject_pv, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus_pv, 0);


		//step 2
		if (iShaderCompileStatus_pv[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(fragmentShaderObject_pv, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength_pv, 0);

			if (iInfoLogLength_pv[0] > 0)
			{
				szInfoLog_pv = GLES32.glGetShaderInfoLog(fragmentShaderObject_pv);

				System.out.println("RTR: Fragemnt Shader Compilation Log" + szInfoLog_pv);
			
				uninitialize();
				
				System.exit(0);
			}
		}

		//FRAGMENT SHADER for PF

		iShaderCompileStatus_pf[0] = 0;
		iInfoLogLength_pf[0] = 0;
		szInfoLog_pf = null;

		fragmentShaderObject_pf = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);

		final String fragmentShaderSourceCode_pf = String.format
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
		GLES32.glShaderSource(fragmentShaderObject_pf,fragmentShaderSourceCode_pf);

		//compile the vertex shader
		GLES32.glCompileShader(fragmentShaderObject_pf);

		//Error checking for FS

		//step 1
		GLES32.glGetShaderiv(fragmentShaderObject_pf, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus_pf, 0);


		//step 2
		if (iShaderCompileStatus_pf[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(fragmentShaderObject_pf, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength_pf, 0);

			if (iInfoLogLength_pf[0] > 0)
			{
				szInfoLog_pf = GLES32.glGetShaderInfoLog(fragmentShaderObject_pf);

				System.out.println("RTR: Fragemnt Shader Compilation Log" + szInfoLog_pf);
			
				uninitialize();
				
				System.exit(0);
			}
		}


		
		//SHADER PROGRAM for PV

		//create obj
		ShaderProgramObject_pv = GLES32.glCreateProgram();

		//add vertex shader
		GLES32.glAttachShader(ShaderProgramObject_pv, vertxShaderObject_pv);

		//add fragment shader
		GLES32.glAttachShader(ShaderProgramObject_pv, fragmentShaderObject_pv);

		//Prelinking binding to vertex attributes
		GLES32.glBindAttribLocation(ShaderProgramObject_pv, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
		GLES32.glBindAttribLocation(ShaderProgramObject_pv, GLESMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

		GLES32.glLinkProgram(ShaderProgramObject_pv);

		//Error checking for shader

		iInfoLogLength_pv[0] = 0;
		szInfoLog_pv = null;
		int[] iProgramLinkStatus_pv = new int[1];

		//step 1
		GLES32.glGetProgramiv(ShaderProgramObject_pv, GLES32.GL_LINK_STATUS, iProgramLinkStatus_pv, 0);

		//step 2
		if (iProgramLinkStatus_pv[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetProgramiv(ShaderProgramObject_pv, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength_pv, 0);

			if (iInfoLogLength_pv[0] > 0)
			{
					szInfoLog_pv = GLES32.glGetShaderInfoLog(fragmentShaderObject_pv);

					System.out.println("RTR: Fragemnt Shader Compilation Log" + szInfoLog_pv);
			
					uninitialize();
				
					System.exit(0);		
			}
		}

		modelMatrix_pvUniform_pv = GLES32.glGetUniformLocation(ShaderProgramObject_pv, "u_model_matrix");
		projectionMatrix_pvUniform_pv = GLES32.glGetUniformLocation(ShaderProgramObject_pv, "u_projection_matrix");
		viewMatrix_pvUniform_pv = GLES32.glGetUniformLocation(ShaderProgramObject_pv, "u_view_matrix");
	
		singleTap_pv = GLES32.glGetUniformLocation(ShaderProgramObject_pv, "u_LKeyPressed");
		MaterialShininessUniform_pv = GLES32.glGetUniformLocation(ShaderProgramObject_pv, "u_matrial_shininess");
	
		LaUniform_pv = GLES32.glGetUniformLocation(ShaderProgramObject_pv, "u_La");
		KaUniform_pv = GLES32.glGetUniformLocation(ShaderProgramObject_pv, "u_Ka");
	
		LdUniform_pv = GLES32.glGetUniformLocation(ShaderProgramObject_pv, "u_Ld");
		KdUniform_pv = GLES32.glGetUniformLocation(ShaderProgramObject_pv, "u_Kd");
	
		LsUniform_pv = GLES32.glGetUniformLocation(ShaderProgramObject_pv, "u_Ls");
		KsUniform_pv = GLES32.glGetUniformLocation(ShaderProgramObject_pv, "u_Ks");
		LightPositionUniform_pv = GLES32.glGetUniformLocation(ShaderProgramObject_pv, "u_light_position");

		//--------------------------------------------------------------------------------------------------------------------------------


		//SHADER PROGRAM for PF

		//create obj
		ShaderProgramObject_pf = GLES32.glCreateProgram();

		//add vertex shader
		GLES32.glAttachShader(ShaderProgramObject_pf, vertexShaderObject_pf);

		//add fragment shader
		GLES32.glAttachShader(ShaderProgramObject_pf, fragmentShaderObject_pf);

		//Prelinking binding to vertex attributes
		GLES32.glBindAttribLocation(ShaderProgramObject_pf, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
		GLES32.glBindAttribLocation(ShaderProgramObject_pf, GLESMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

		GLES32.glLinkProgram(ShaderProgramObject_pf);

		//Error checking for shader

		iInfoLogLength_pf[0] = 0;
		szInfoLog_pf = null;
		int[] iProgramLinkStatus_pf = new int[1];

		//step 1
		GLES32.glGetProgramiv(ShaderProgramObject_pf, GLES32.GL_LINK_STATUS, iProgramLinkStatus_pf, 0);

		//step 2
		if (iProgramLinkStatus_pf[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetProgramiv(ShaderProgramObject_pf, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength_pf, 0);

			if (iInfoLogLength_pf[0] > 0)
			{
					szInfoLog_pf = GLES32.glGetShaderInfoLog(fragmentShaderObject_pf);

					System.out.println("RTR: Fragemnt Shader Compilation Log" + szInfoLog_pf);
			
					uninitialize();
				
					System.exit(0);		
			}
		}

		//--------------------------------------------------------------------------------------

		//Postlinking retriving uniform locations

		modelMatrix_pvUniform_pf = GLES32.glGetUniformLocation(ShaderProgramObject_pf, "u_model_matrix");
		projectionMatrix_pvUniform_pf = GLES32.glGetUniformLocation(ShaderProgramObject_pf, "u_projection_matrix");
		viewMatrix_pvUniform_pf = GLES32.glGetUniformLocation(ShaderProgramObject_pf, "u_view_matrix");

		singleTap_pf = GLES32.glGetUniformLocation(ShaderProgramObject_pf, "u_LKeyPressed");
		MaterialShininessUniform_pf = GLES32.glGetUniformLocation(ShaderProgramObject_pf, "u_material_shininess");

		LaUniform_pf = GLES32.glGetUniformLocation(ShaderProgramObject_pf, "u_La");
		KaUniform_pf = GLES32.glGetUniformLocation(ShaderProgramObject_pf, "u_Ka");

		LdUniform_pf = GLES32.glGetUniformLocation(ShaderProgramObject_pf, "u_Ld");
		KdUniform_pf = GLES32.glGetUniformLocation(ShaderProgramObject_pf, "u_Kd");

		LsUniform_pf = GLES32.glGetUniformLocation(ShaderProgramObject_pf, "u_Ls");
		KsUniform_pf = GLES32.glGetUniformLocation(ShaderProgramObject_pf, "u_Ks");
		LightPositionUniform_pf = GLES32.glGetUniformLocation(ShaderProgramObject_pf, "u_light_position");

		
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
		Matrix.setIdentityM(perspectiveprojectionMatrix_pv, 0);

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

		Matrix.perspectiveM(perspectiveprojectionMatrix_pv, 0 , 45.0f, ((float)width /(float) height), 0.1f, 100.0f);
	}

	private void display()
	{

		GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT |GLES32.GL_DEPTH_BUFFER_BIT);

		if (gbVertex == true && gbFragment == false)
		{
		//binding
		GLES32.glUseProgram(ShaderProgramObject_pv);

		//----------------------------------------------------------------------------------------------------------

		// OpenGL Drawing
		//declaration of matrices
		float [] modelMatrix_pv = new float[16];
		float [] viewMatrix_pv = new float[16];
		float [] translationMatrix_pv = new float[16];
		float [] rotationMatrix_pv = new float[16];
		float [] projectionMatrix_pv = new float[16];

		//--------------------------------------------------------------------------------------------------------------

		//initialize above matrices to identity
		Matrix.setIdentityM(modelMatrix_pv,0);
		Matrix.setIdentityM(viewMatrix_pv,0);
		Matrix.setIdentityM(translationMatrix_pv,0);
		Matrix.setIdentityM(projectionMatrix_pv,0);
		Matrix.setIdentityM(rotationMatrix_pv,0);


		//do necessary transformation if ant required
		//translateM(float[] m, int mOffset, float x, float y, float z)
		//Translates matrix m by x, y, and z in place.
		//translationMatrix_pv = translate(-1.5f, 0.0f, -6.0f);
		Matrix.translateM(translationMatrix_pv, 0, 0.0f, 0.0f, -4.0f);

		//ratationMatrix = rotate(angle_sphere, 1.0f, 1.0f, 1.0f);
		//static void	setRotateM(float[] rm, int rmOffset, float a, float x, float y, float z)
		//Creates a matrix for rotation by angle a (in degrees) around the axis (x, y, z).
		//Matrix.setRotateM(rotationMatrix_pv , 0, angle_sphere, 1.0f, 0.0f, 0.0f);

		//do necessary matrix multiplication
		//modelviewMatrix_pv = translationMatrix_pv * ratationMatrix;
		//modelViewprojectionMatrix_pv = perspectiveprojectionMatrix_pv * modelviewMatrix_pv

		Matrix.multiplyMM(modelMatrix_pv,0,
						  modelMatrix_pv,0,
						  translationMatrix_pv,0);	

		/*Matrix.multiplyMM(modelMatrix_pv,0,
						  modelMatrix_pv,0,
						  rotationMatrix_pv,0);	 */ 

		Matrix.multiplyMM(projectionMatrix_pv,0,
						  projectionMatrix_pv,0,
						  perspectiveprojectionMatrix_pv,0);
		

		//send necessary matrices to shader in respective uniforms
		GLES32.glUniformMatrix4fv(modelMatrix_pvUniform_pv, 1,false, modelMatrix_pv, 0);

		GLES32.glUniformMatrix4fv(projectionMatrix_pvUniform_pv, 1, false, projectionMatrix_pv, 0);
		//glUniformMatrix4fv(projectionMatrix_pvUniform, 1, GL_FALSE, perspectiveprojectionMatrix_pv);

		GLES32.glUniformMatrix4fv(viewMatrix_pvUniform_pv, 1, false, viewMatrix_pv, 0);
		//GL_FALSE = internally we are not transposing = row -> col / col -> row
		//OpenGL is col major while DirectX is row major

		if (gbLight == true)
		{
			GLES32.glUniform1i(singleTap_pv, 1);
			GLES32.glUniform1f(MaterialShininessUniform_pv,  128.0f );

			GLES32.glUniform3f(LaUniform_pv, 0.0f,0.0f,0.0f);
			GLES32.glUniform3f(KaUniform_pv, 0.0f,0.0f,0.0f);

			GLES32.glUniform3f(LdUniform_pv, 1.0f,1.0f,1.0f);
			GLES32.glUniform3f(KdUniform_pv,  1.0f,1.0f,1.0f);

			GLES32.glUniform3f(LsUniform_pv, 1.0f,1.0f,1.0f);
			GLES32.glUniform3f(KsUniform_pv,  1.0f,1.0f,1.0f);

			GLES32.glUniform4f(LightPositionUniform_pv,100.0f,100.0f,100.0f,1.0f);
			}
		else
		{
			GLES32.glUniform1i(singleTap_pv, 0);
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

	}

	else if(gbFragment == true && gbVertex == false)
	{
			//binding
			GLES32.glUseProgram(ShaderProgramObject_pf);

			//----------------------------------------------------------------------------------------------------------
	
			// OpenGL Drawing
			//declaration of matrices
			float [] modelMatrix_pf = new float[16];
			float [] viewMatrix_pf = new float[16];
			float [] translationMatrix_pf = new float[16];
			float [] rotationMatrix_pf = new float[16];
			float [] projectionMatrix_pf = new float[16];
	
			//--------------------------------------------------------------------------------------------------------------
	
			//initialize above matrices to identity
			Matrix.setIdentityM(modelMatrix_pf,0);
			Matrix.setIdentityM(viewMatrix_pf,0);
			Matrix.setIdentityM(translationMatrix_pf,0);
			Matrix.setIdentityM(projectionMatrix_pf,0);
			Matrix.setIdentityM(rotationMatrix_pf,0);
	
	
			//do necessary transformation if ant required
			//translateM(float[] m, int mOffset, float x, float y, float z)
			//Translates matrix m by x, y, and z in place.
			//translationMatrix_pv = translate(-1.5f, 0.0f, -6.0f);
			Matrix.translateM(translationMatrix_pf, 0, 0.0f, 0.0f, -4.0f);
	
			//ratationMatrix = rotate(angle_sphere, 1.0f, 1.0f, 1.0f);
			//static void	setRotateM(float[] rm, int rmOffset, float a, float x, float y, float z)
			//Creates a matrix for rotation by angle a (in degrees) around the axis (x, y, z).
			//Matrix.setRotateM(rotationMatrix_pv , 0, angle_sphere, 1.0f, 0.0f, 0.0f);
	
			//do necessary matrix multiplication
			//modelviewMatrix_pv = translationMatrix_pv * ratationMatrix;
			//modelViewprojectionMatrix_pv = perspectiveprojectionMatrix_pv * modelviewMatrix_pv
	
			Matrix.multiplyMM(modelMatrix_pf,0,
							  modelMatrix_pf,0,
							  translationMatrix_pf,0);	
	
			/*Matrix.multiplyMM(modelMatrix_pv,0,
							  modelMatrix_pv,0,
							  rotationMatrix_pv,0);	 */ 
	
			Matrix.multiplyMM(projectionMatrix_pf,0,
							  projectionMatrix_pf,0,
							  perspectiveprojectionMatrix_pv,0);
			
	
			//send necessary matrices to shader in respective uniforms
			GLES32.glUniformMatrix4fv(modelMatrix_pvUniform_pf, 1,false, modelMatrix_pf, 0);
	
			GLES32.glUniformMatrix4fv(projectionMatrix_pvUniform_pf, 1, false, projectionMatrix_pf, 0);
			//glUniformMatrix4fv(projectionMatrix_pvUniform, 1, GL_FALSE, perspectiveprojectionMatrix_pv);
	
			GLES32.glUniformMatrix4fv(viewMatrix_pvUniform_pf, 1, false, viewMatrix_pf, 0);
			//GL_FALSE = internally we are not transposing = row -> col / col -> row
			//OpenGL is col major while DirectX is row major
	
			if (gbLight == true)
			{
				GLES32.glUniform1i(singleTap_pf, 1);
				GLES32.glUniform1f(MaterialShininessUniform_pf,  128.0f );
	
				GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.0f,0.0f,0.0f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf,  1.0f,1.0f,1.0f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf,  1.0f,1.0f,1.0f);
	
				GLES32.glUniform4f(LightPositionUniform_pf,100.0f,100.0f,100.0f,1.0f);
				}
			else
			{
				GLES32.glUniform1i(singleTap_pf, 0);
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
	}

	else
	{
			//binding
			GLES32.glUseProgram(ShaderProgramObject_pf);

			//----------------------------------------------------------------------------------------------------------
	
			// OpenGL Drawing
			//declaration of matrices
			float [] modelMatrix_pf = new float[16];
			float [] viewMatrix_pf = new float[16];
			float [] translationMatrix_pf = new float[16];
			float [] rotationMatrix_pf = new float[16];
			float [] projectionMatrix_pf = new float[16];
	
			//--------------------------------------------------------------------------------------------------------------
	
			//initialize above matrices to identity
			Matrix.setIdentityM(modelMatrix_pf,0);
			Matrix.setIdentityM(viewMatrix_pf,0);
			Matrix.setIdentityM(translationMatrix_pf,0);
			Matrix.setIdentityM(projectionMatrix_pf,0);
			Matrix.setIdentityM(rotationMatrix_pf,0);
	
	
			//do necessary transformation if ant required
			//translateM(float[] m, int mOffset, float x, float y, float z)
			//Translates matrix m by x, y, and z in place.
			//translationMatrix_pv = translate(-1.5f, 0.0f, -6.0f);
			Matrix.translateM(translationMatrix_pf, 0, 0.0f, 0.0f, -4.0f);
	
			//ratationMatrix = rotate(angle_sphere, 1.0f, 1.0f, 1.0f);
			//static void	setRotateM(float[] rm, int rmOffset, float a, float x, float y, float z)
			//Creates a matrix for rotation by angle a (in degrees) around the axis (x, y, z).
			//Matrix.setRotateM(rotationMatrix_pv , 0, angle_sphere, 1.0f, 0.0f, 0.0f);
	
			//do necessary matrix multiplication
			//modelviewMatrix_pv = translationMatrix_pv * ratationMatrix;
			//modelViewprojectionMatrix_pv = perspectiveprojectionMatrix_pv * modelviewMatrix_pv
	
			Matrix.multiplyMM(modelMatrix_pf,0,
							  modelMatrix_pf,0,
							  translationMatrix_pf,0);	
	
			/*Matrix.multiplyMM(modelMatrix_pv,0,
							  modelMatrix_pv,0,
							  rotationMatrix_pv,0);	 */ 
	
			Matrix.multiplyMM(projectionMatrix_pf,0,
							  projectionMatrix_pf,0,
							  perspectiveprojectionMatrix_pv,0);
			
	
			//send necessary matrices to shader in respective uniforms
			GLES32.glUniformMatrix4fv(modelMatrix_pvUniform_pf, 1,false, modelMatrix_pf, 0);
	
			GLES32.glUniformMatrix4fv(projectionMatrix_pvUniform_pf, 1, false, projectionMatrix_pf, 0);
			//glUniformMatrix4fv(projectionMatrix_pvUniform, 1, GL_FALSE, perspectiveprojectionMatrix_pv);
	
			GLES32.glUniformMatrix4fv(viewMatrix_pvUniform_pf, 1, false, viewMatrix_pf, 0);
			//GL_FALSE = internally we are not transposing = row -> col / col -> row
			//OpenGL is col major while DirectX is row major
	
			if (gbLight == true)
			{
				GLES32.glUniform1i(singleTap_pf, 1);
				GLES32.glUniform1f(MaterialShininessUniform_pf,  128.0f );
	
				GLES32.glUniform3f(LaUniform_pf, 0.0f,0.0f,0.0f);
				GLES32.glUniform3f(KaUniform_pf, 0.0f,0.0f,0.0f);
	
				GLES32.glUniform3f(LdUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KdUniform_pf,  1.0f,1.0f,1.0f);
	
				GLES32.glUniform3f(LsUniform_pf, 1.0f,1.0f,1.0f);
				GLES32.glUniform3f(KsUniform_pf,  1.0f,1.0f,1.0f);
	
				GLES32.glUniform4f(LightPositionUniform_pf,100.0f,100.0f,100.0f,1.0f);
				}
			else
			{
				GLES32.glUniform1i(singleTap_pf, 0);
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
	}

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

		if(ShaderProgramObject_pv != 0)
			{
				int[] shaderCount = new int[1];
				int shaderNumber;

				GLES32.glUseProgram(ShaderProgramObject_pv);

				//ask program how many shaders are attached
				GLES32.glGetProgramiv(	ShaderProgramObject_pv,
										GLES32.GL_ATTACHED_SHADERS,
										shaderCount, 0);

				int[] shaders = new int[shaderCount[0]];

				if(shaderCount[0] != 0)
				{
					GLES32.glGetAttachedShaders(	ShaderProgramObject_pv,
													shaderCount[0],
													shaderCount, 0,
													shaders, 0);

					for(shaderNumber = 0; shaderNumber < shaderCount[0]; shaderNumber++)
					{
						//detach shaders
						GLES32.glDetachShader(	ShaderProgramObject_pv,
												shaders[shaderNumber]);

						//delete shaders
						GLES32.glDeleteShader(shaders[shaderNumber]);

						shaders[shaderNumber] = 0;
					}
				} 
				GLES32.glDeleteProgram(ShaderProgramObject_pv);
				ShaderProgramObject_pv = 0;
				GLES32.glUseProgram(0);
			}
	}
}
