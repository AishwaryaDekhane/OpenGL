//global variable
var canvas = null;
var gl = null;
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;

//when whole webglmacros is const , all inside it are automatically const
const WebGLMacros = {
    VDG_ATTRIBUTE_VERTEX: 0,
    VDG_ATTRIBUTE_COLOR: 1,
    VDG_ATTRIBUTE_NORMAL: 2,
    VDG_ATTRIBUTE_TEXTURE0: 3,
};

var vertexShaderObject;
var fragmentShaderObject;
var shaderProgramObject;

var ModelMatrixUniform_pf;
var ViewMatrixUniform_pf;
var ProjectionMatrixUniform_pf;
var MaterialShininessUniform_pf;
var LKeyPressedUniform_pf;
var KaUniform_pf;
var KdUniform_pf;
var KsUniform_pf;

var LaUniform_pf_zero;
var LdUniform_pf_zero;
var LsUniform_pf_zero;
var LightPositionUniform_pf_zero;

var LaUniform_pf_one;
var LdUniform_pf_one;
var LsUniform_pf_one;
var LightPositionUniform_pf_one;

var LaUniform_pf_two;
var LdUniform_pf_two;
var LsUniform_pf_two;
var LightPositionUniform_pf_two;

var sphere = null;



//LIGHT0
var LightAmbientZero = [0.0, 0.0, 0.0];
var LightDiffuseZero = [1.0, 0.0, 0.0];
var LightSpecularZero = [1.0, 0.0, 0.0];
var LightPositionZero = [100.0, 0.0, 0.0];

//LIGHT1
var LightAmbientOne = [0.0, 0.0, 0.0];
var LightDiffuseOne = [0.0, 1.0, 0.0];
var LightSpecularOne = [0.0, 1.0, 0.0];
var LightPositionOne = [0.0, -100.0, 0.0];

//LIGHT2
var LightAmbientTwo = [0.0, 0.0, 0.0];
var LightDiffuseTwo = [0.0, 0.0, 1.0];
var LightSpecularTwo = [0.0, 0.0, 1.0];
var LightPositionTwo = [0.0, 0.0, 100.0];

var MaterialAmbient = [0.0, 0.0, 0.0];
var MaterialDiffuse = [1.0, 1.0, 1.0];
var MaterialSpecular = [1.0, 1.0, 1.0];
var MaterialShininess = [128.0];

var LightAngleZero = 0.0;
var LightAngleOne = 0.0;
var LightAngleTwo = 0.0;

var bLKeyPressed = false;
var perspectiveProjectionMatrix;

// to start animation :to have requestAnimationFrame() to be called "cross browser" compatible
var requestAnimationFrame = window.requestAnimationFrame || window.webkitRequestAnimationFrame || window.mozRequestAnimationFrame || window.oRequestAnimationFrame || window.msRequestAnimationFrame || null;

//to stop animation : to have cancelAnimationFrame() to be called "cross browser" compatible
var cancelAnimationFrame = window.cancelAnimationFrame || window.webkitCancelRequestAnimationFrame || window.webkitCancelAnimationFrame || window.mozCancelRequestAnimationFrame || window.mozCancelAnimationFrame || window.oCancelRequestAnimationFrame || window.oCancelAnimationFrame || window.msCancelRequestAnimationFrame || window.msCancelAnimationFrame || null;

//onload function
function main() {
    //get <canvas> element
    canvas = document.getElementById("AMC");
    if (!canvas)
        console.log("obtaining canvas failed\n");
    else
        console.log("obtaining canvas succeeded\n");

    //print canvas width and height on console
    //console.log("Canvas Width : " + canvas.width + " And Canvas Height : " + canvas.height);
    canvas_original_width = canvas.width;
    canvas_original_height = canvas.height;

    //register keyboards keydown event handler
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mouseDown, false);
    window.addEventListener("resize", resize, false);

    //initialize webgl
    init();

    //start drawing here as warming-up
    resize();
    draw();

}

function keyDown(event) {

    //code
    switch (event.keyCode) {
        case 27: //escape
            //uninitialize
            uninitialize();
            //close our application tab
            window.close();
            break;

        case 70: //for full screen
            togglefullScreen();
            break;

        case 76: //for l or L
            if (bLKeyPressed == false)
                bLKeyPressed = true;
            else
                bLKeyPressed = false;
            break;
    }
}

function mouseDown() {
    //code
    // alert("Mouse is clicked");
}


function togglefullScreen() {
    //code
    var fullscreen_element = document.fullscreenElement || document.webkitFullscreenElement || document.mozFullScreenElement || document.msFullscreenElement || null;

    //if not fullscreen
    if (fullscreen_element == null) {
        console.log("full screen\n");
        if (canvas.requestFullscreen)
            canvas.requestFullscreen();
        else if (canvas.mozRequestFullScreen)
            canvas.mozRequestFullScreen();
        else if (canvas.webkitFullscreenElement)
            canvas.webkitFullscreenElement();
        else if (canvas.msRequestFullscreen)
            canvas.msRequestFullscreen();
        bFullscreen = true;
    } else //if already fullscreen
    {
        console.log("full screen out\n");
        if (document.exitFullscreen)
            document.exitFullscreen();
        else if (document.mozCancelFullScreen)
            document.mozCancelFullScreen();
        else if (document.webkitExitFullscreen)
            document.webkitExitFullscreen();
        else if (document.msExitFullscreen)
            document.msExitFullscreen();
        bFullscreen = false;
    }
}

function init() {
    //code
    //get webgl 2.0 context
    gl = canvas.getContext("webgl2");

    if (gl == null) {
        console.log("failed to get th rendering context for webgl");
        return;
    }
    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

    console.log("inside init\n");
    //vertex shader
    var vertexShaderSourceCode =
        "#version 300 es" +
        "\n" +
        "precision mediump int;" +
        "in vec4 vPosition;" +
        "in vec3 vNormal;" +
        "uniform mat4 u_model_matrix;" +
        "uniform mat4 u_projection_matrix;" +
        "uniform mat4 u_view_matrix;" +
        "uniform mediump int u_LKeyPressed;" +
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

        "view_vector = vec3(-eyeCoordinates.xyz);" +

        "}" +
        "gl_Position = u_projection_matrix * u_model_matrix * u_view_matrix * vPosition;" +
        "}";

    vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObject, vertexShaderSourceCode);
    gl.compileShader(vertexShaderObject);
    if (gl.getShaderParameter(vertexShaderObject, gl.COMPILE_STATUS) == false) {
        var error = gl.getShaderInfoLog(vertexShaderObject);
        if (error.length > 0) {
            alert(error);
            uninitialize();
        }
    }
    console.log("vertexshader succeeded\n");

    //fragment shader
    var fragmentShaderSourceCode =
        "#version 300 es" +
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
        "}";

    fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObject, fragmentShaderSourceCode);
    gl.compileShader(fragmentShaderObject);
    if (gl.getShaderParameter(fragmentShaderObject, gl.COMPILE_STATUS) == false) {
        var error = gl.getShaderInfoLog(fragmentShaderObject);
        if (error.length > 0) {
            alert(error);
            uninitialize();
        }
    }
    console.log("fragmentshader succeeded\n");

    //shader program
    shaderProgramObject = gl.createProgram();
    gl.attachShader(shaderProgramObject, vertexShaderObject);
    gl.attachShader(shaderProgramObject, fragmentShaderObject);

    //pre-binding of shader program object with vertex shader attributes
    gl.bindAttribLocation(shaderProgramObject, WebGLMacros.VDG_ATTRIBUTE_VERTEX, "vPosition");
    gl.bindAttribLocation(shaderProgramObject, WebGLMacros.VDG_ATTRIBUTE_NORMAL, "vNormal");

    gl.linkProgram(shaderProgramObject);
    if (!gl.getProgramParameter(shaderProgramObject, gl.LINK_STATUS)) {
        var error = gl.getProgramInfoLog(shaderProgramObject);
        if (error.length > 0) {
            alert(error);
            uninitialize();
        }
    }
    console.log("shaderprogram succeeded\n");

    //get mvp uniform location

    ModelMatrixUniform_pf = gl.getUniformLocation(shaderProgramObject, "u_model_matrix");
    ProjectionMatrixUniform_pf = gl.getUniformLocation(shaderProgramObject, "u_projection_matrix");
    ViewMatrixUniform_pf = gl.getUniformLocation(shaderProgramObject, "u_view_matrix");

    LKeyPressedUniform_pf = gl.getUniformLocation(shaderProgramObject, "u_LKeyPressed");
    MaterialShininessUniform_pf = gl.getUniformLocation(shaderProgramObject, "u_material_shininess");

    LaUniform_pf_zero = gl.getUniformLocation(shaderProgramObject, "u_La_zero");
    LaUniform_pf_one = gl.getUniformLocation(shaderProgramObject, "u_La_one");
    LaUniform_pf_two = gl.getUniformLocation(shaderProgramObject, "u_La_two");
    KaUniform_pf = gl.getUniformLocation(shaderProgramObject, "u_Ka");

    LdUniform_pf_zero = gl.getUniformLocation(shaderProgramObject, "u_Ld_zero");
    LdUniform_pf_one = gl.getUniformLocation(shaderProgramObject, "u_Ld_one");
    LdUniform_pf_two = gl.getUniformLocation(shaderProgramObject, "u_Ld_two");
    KdUniform_pf = gl.getUniformLocation(shaderProgramObject, "u_Kd");

    LsUniform_pf_zero = gl.getUniformLocation(shaderProgramObject, "u_Ls_zero");
    LsUniform_pf_one = gl.getUniformLocation(shaderProgramObject, "u_Ls_one");
    LsUniform_pf_two = gl.getUniformLocation(shaderProgramObject, "u_Ls_two");
    KsUniform_pf = gl.getUniformLocation(shaderProgramObject, "u_Ks");

    LightPositionUniform_pf_zero = gl.getUniformLocation(shaderProgramObject, "u_light_position_zero");
    LightPositionUniform_pf_one = gl.getUniformLocation(shaderProgramObject, "u_light_position_one");
    LightPositionUniform_pf_two = gl.getUniformLocation(shaderProgramObject, "u_light_position_two");

    //vertices, colors, shader attribs, vbo_position_triangle, vao_triangle initializations

    sphere = new Mesh();
    makeSphere(sphere, 2.0, 30, 30);

    //set claer color
    gl.clearColor(0.0, 0.0, 0.0, 1.0); //black

    //depth test will always be enabled
    gl.enable(gl.DEPTH_TEST);

    //depth test to do
    gl.depthFunc(gl.LEQUAL);

    //WE will always cull back faces for better performance
    gl.enable(gl.CULL_FACE);

    //initialize projection matrix
    perspectiveProjectionMatrix = mat4.create();
}

function resize() {
    //code

    console.log("inside resize\n");

    if (bFullscreen == true) {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    } else {
        canvas.width = canvas_original_width;
        canvas.height = canvas_original_height;
    }

    //set the viewport to match
    gl.viewport(0, 0, canvas.width, canvas.height);

    //perspective
    mat4.perspective(perspectiveProjectionMatrix, 45.0, parseFloat(canvas.width) / parseFloat(canvas.height), 0.1, 100);

    console.log("after perspective call\n");
}

function degToRad(degrees) {
    //code
    return (degrees * Math.PI / 180);
}

function draw() {
    //code
    var radius = 100;

    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);

    console.log("inside draw\n");

    if (bLKeyPressed == true) {

        //setting light properties
        gl.uniform1i(LKeyPressedUniform_pf, 1);
        gl.uniform1f(MaterialShininessUniform_pf, 128.0);

        gl.uniform3fv(LaUniform_pf_zero, LightAmbientZero);
        gl.uniform3fv(LaUniform_pf_one, LightAmbientOne);
        gl.uniform3fv(LaUniform_pf_two, LightAmbientTwo);
        gl.uniform3fv(KaUniform_pf, MaterialAmbient);

        gl.uniform3fv(LdUniform_pf_zero, LightDiffuseZero);
        gl.uniform3fv(LdUniform_pf_one, LightDiffuseOne);
        gl.uniform3fv(LdUniform_pf_two, LightDiffuseTwo);
        gl.uniform3fv(KdUniform_pf, MaterialDiffuse);

        gl.uniform3fv(LsUniform_pf_zero, LightSpecularZero);
        gl.uniform3fv(LsUniform_pf_one, LightSpecularOne);
        gl.uniform3fv(LsUniform_pf_two, LightSpecularTwo);
        gl.uniform3fv(KsUniform_pf, MaterialSpecular);

        gl.uniform4f(LightPositionUniform_pf_zero, 0.0, ((Math.cos(LightAngleZero)) * radius), ((Math.sin(LightAngleZero)) * radius), 0.0);
        gl.uniform4f(LightPositionUniform_pf_one, ((Math.cos(LightAngleOne)) * radius), 0.0, ((Math.sin(LightAngleOne)) * radius), 0.0);
        gl.uniform4f(LightPositionUniform_pf_two, 0.0, ((Math.sin(LightAngleTwo)) * radius), ((Math.cos(LightAngleTwo)) * radius), 0.0);

    } else {
        gl.uniform1i(LKeyPressedUniform_pf, 0);
    }

    var modelMatrix = mat4.create();
    var rotationMatrixX = mat4.create();
    var rotationMatrixY = mat4.create();
    var rotationMatrixZ = mat4.create();
    var ViewMatrix = mat4.create();

    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -6.0]);
    mat4.rotateX(rotationMatrixX, rotationMatrixX, degToRad(LightAngleOne));
    mat4.rotateY(rotationMatrixY, rotationMatrixY, degToRad(LightAngleTwo));
    mat4.rotateZ(rotationMatrixZ, rotationMatrixZ, degToRad(LightAngleZero));

    mat4.multiply(ViewMatrix, ViewMatrix, rotationMatrixX);
    mat4.multiply(ViewMatrix, ViewMatrix, rotationMatrixY);
    mat4.multiply(ViewMatrix, ViewMatrix, rotationMatrixZ);

    gl.uniformMatrix4fv(ModelMatrixUniform_pf, false, modelMatrix);
    gl.uniformMatrix4fv(ViewMatrixUniform_pf, false, ViewMatrix);
    gl.uniformMatrix4fv(ProjectionMatrixUniform_pf, false, perspectiveProjectionMatrix);

    sphere.draw();

    gl.useProgram(null);

    LightAngleZero = LightAngleZero + 0.005;
    if (LightAngleZero >= 360.0)
        LightAngleZero = LightAngleZero - 360.0;

    LightAngleOne = LightAngleOne + 0.005;
    if (LightAngleOne >= 360.0)
        LightAngleOne = LightAngleOne - 360.0;

    LightAngleTwo = LightAngleTwo + 0.005;
    if (LightAngleTwo >= 360.0)
        LightAngleTwo = LightAngleTwo - 360.0;

    //animation loop
    requestAnimationFrame(draw, canvas);
}

function uninitialize() {
    //code

    console.log("inside uninitialize\n");

    if (sphere) {
        sphere.deallocate();
        sphere = null;
    }

    if (shaderProgramObject) {
        if (fragmentShaderObject) {
            gl.detachShader(shaderProgramObject, fragmentShaderObject);
            gl.detachShader(fragmentShaderObject);
            fragmentShaderObject = null;
        }

        if (vertexShaderObject) {
            gl.detachShader(shaderProgramObject, vertexShaderObject);
            gl.detachShader(vertexShaderObject);
            vertexShaderObject = null;
        }

        gl.deleteProgram(shaderProgramObject);
        shaderProgramObject = null;
    }
}