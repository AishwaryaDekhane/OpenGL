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

var light_ambient_zero = [0.0, 0.0, 0.0];
var light_diffuse_zero = [1.0, 0.0, 0.0];
var light_specular_zero = [1.0, 0.0, 0.0];
var light_position_zero = [-2.0, 0.0, 0.0, 1.0];

var light_ambient_one = [0.0, 0.0, 0.0];
var light_diffuse_one = [0.0, 0.0, 1.0];
var light_specular_one = [0.0, 0.0, 1.0];
var light_position_one = [2.0, 0.0, 0.0, 1.0];

var material_ambient = [0.0, 0.0, 0.0];
var material_diffuse = [1.0, 1.0, 1.0];
var material_specular = [1.0, 1.0, 1.0];
var material_shininess = 128.0;


var ModelMatrixUniform;
var ViewMatrixUniform;
var ProjectionMatrixUniform;
var LaUniform_red;
var KaUniform;
var LdUniform_red;
var KdUniform;
var LsUniform_red;
var KsUniform;
var LightPositionUniform_red;
var MaterialShininessUniform;

var ModelMatrixUniform_blue;
var ViewMatrixUniform_blue;
var ProjectionMatrixUniform_blue;
var LaUniform_blue;
var LdUniform_blue;
var LsUniform_blue;
var KaUniform_blue;
var KdUniform_blue;
var KsUniform_blue;
var LightPositionUniform_blue;
var MaterialShininessUniform_blue;
var LKeyPressedUniform;

var bLKeyPressed = false;
var perspectiveProjectionMatrix;

var vao_triangle;
var vbo_position_triangle;
var vbo_color_triangle;

var angle_tri = 0.0;

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
        "uniform int u_LKeyPressed;" +
        "uniform vec3 u_Ka;" +
        "uniform vec3 u_Kd;" +
        "uniform vec3 u_Ks;" +
        "uniform float u_matrial_shininess;" +
        "out vec3 phong_ADS_light;" +

        "uniform vec3 u_La_red;" +
        "uniform vec3 u_Ld_red;" +
        "uniform vec3 u_Ls_red;" +
        "uniform vec4 u_light_position_red;" +

        "uniform vec3 u_La_blue;" +
        "uniform vec3 u_Ld_blue;" +
        "uniform vec3 u_Ls_blue;" +
        "uniform vec4 u_light_position_blue;" +


        "void main(void)" +
        "{" +
        "if (u_LKeyPressed == 1)" +
        "{" +
        "vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;" +
        "vec3 tNorm = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);" +

        "vec3 light_direction_red = normalize(vec3(u_light_position_red - eyeCoordinates));" +
        "float tn_dot_Ld_red = max(dot(light_direction_red, tNorm), 0.0);" +
        "vec3 reflection_vector_red = reflect(-light_direction_red, tNorm);" +
        "vec3 viewer_vector_red = normalize(vec3(-eyeCoordinates.xyz));" +

        "vec3 light_direction_blue = normalize(vec3(u_light_position_blue - eyeCoordinates));" +
        "float tn_dot_Ld_blue = max(dot(light_direction_blue, tNorm), 0.0);" +
        "vec3 reflection_vector_blue = reflect(-light_direction_blue, tNorm);" +
        "vec3 viewer_vector_blue = normalize(vec3(-eyeCoordinates.xyz));" +

        "vec3 ambient = u_La_red * u_Ka + u_La_blue * u_Ka;" +
        "vec3 diffuse = u_Ld_red * u_Kd * tn_dot_Ld_red + u_Ld_blue * u_Kd * tn_dot_Ld_blue;" +
        "vec3 specular = u_Ls_red * u_Ks * pow(max(dot(reflection_vector_red, viewer_vector_red), 0.0), u_matrial_shininess) +  u_Ls_blue * u_Ks * pow(max(dot(reflection_vector_blue, viewer_vector_blue), 0.0), u_matrial_shininess);" +
        "phong_ADS_light = ambient + diffuse + specular;" +


        "}" +
        "else" +
        "{" +
        "phong_ADS_light = vec3(1.0, 1.0, 1.0);" +
        "}" +
        "gl_Position = u_projection_matrix * u_model_matrix * u_view_matrix * vPosition;" +
        "}"

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
        "in vec3 phong_ADS_light;" +
        "out vec4 FragColor;" +
        "uniform int u_LKeyPressed;" +
        "void main(void)" +
        "{" +
        "if (u_LKeyPressed == 1)" +
        "{" +
        "FragColor = vec4(phong_ADS_light,1.0);" +
        "}" +
        "else" +
        "{" +
        "FragColor = vec4(1.0, 1.0, 1.0, 1.0);" +
        "}" +
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

    var trianglevertices = new Float32Array([
        0.0, 1.0, 0.0, -1.0, -1.0, 1.0,
        1.0, -1.0, 1.0,

        0.0, 1.0, 0.0,
        1.0, -1.0, 1.0,
        1.0, -1.0, -1.0,

        0.0, 1.0, 0.0,
        1.0, -1.0, -1.0, -1.0, -1.0, -1.0,

        0.0, 1.0, 0.0, -1.0, -1.0, -1.0, -1.0, -1.0, 1.0,
    ]);

    var trianglecolor = new Float32Array([
        0.0, 0.447214, 0.894427, //front-top
        0.0, 0.447214, 0.894427, //front-left
        0.0, 0.447214, 0.894427, //front-right

        0.894427, 0.447214, 0.0, //right-top
        0.894427, 0.447214, 0.0, //right-left
        0.894427, 0.447214, 0.0, //right-right

        0.0, 0.447214, -0.894427, //back-top
        0.0, 0.447214, -0.894427, //back-left
        0.0, 0.447214, -0.894427, //back-right

        -0.894427, 0.447214, 0.0, //left-top
        -0.894427, 0.447214, 0.0, //left-left
        -0.894427, 0.447214, 0.0, //left-right
    ]);

    vao_triangle = gl.createVertexArray();
    gl.bindVertexArray(vao_triangle);

    vbo_position_triangle = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position_triangle);
    gl.bufferData(gl.ARRAY_BUFFER, trianglevertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vbo_color_triangle = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_color_triangle);
    gl.bufferData(gl.ARRAY_BUFFER, trianglecolor, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_NORMAL);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);

    console.log("vao_triangle,vbo_position_triangle succeeded\n");

    //get mvp uniform location

    /* ModelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_model_matrix");
     ProjectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projection_matrix");
     ViewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_view_matrix");

   
     MaterialShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_matrial_shininess");

     LaUniform = gl.getUniformLocation(shaderProgramObject, "u_La");
     KaUniform = gl.getUniformLocation(shaderProgramObject, "u_Ka");

     LdUniform = gl.getUniformLocation(shaderProgramObject, "u_Ld");
     KdUniform = gl.getUniformLocation(shaderProgramObject, "u_Kd");

     LsUniform = gl.getUniformLocation(shaderProgramObject, "u_Ls");
     KsUniform = gl.getUniformLocation(shaderProgramObject, "u_Ks");
     LightPositionUniform = gl.getUniformLocation(shaderProgramObject, "u_light_position");*/


    ModelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_model_matrix");
    ProjectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projection_matrix");
    ViewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_view_matrix");

    LKeyPressUniform = gl.getUniformLocation(shaderProgramObject, "u_LKeyPressed");
    MaterialShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_matrial_shininess");

    LaUniform_red = gl.getUniformLocation(shaderProgramObject, "u_La_red");
    LaUniform_blue = gl.getUniformLocation(shaderProgramObject, "u_La_blue");
    KaUniform = gl.getUniformLocation(shaderProgramObject, "u_Ka");

    LdUniform_red = gl.getUniformLocation(shaderProgramObject, "u_Ld_red");
    LdUniform_blue = gl.getUniformLocation(shaderProgramObject, "u_Ld_blue");
    KdUniform = gl.getUniformLocation(shaderProgramObject, "u_Kd");

    LsUniform_red = gl.getUniformLocation(shaderProgramObject, "u_Ls_red");
    LsUniform_blue = gl.getUniformLocation(shaderProgramObject, "u_Ls_blue");
    KsUniform = gl.getUniformLocation(shaderProgramObject, "u_Ks");

    LightPositionUniform_red = gl.getUniformLocation(shaderProgramObject, "u_light_position_red");
    LightPositionUniform_blue = gl.getUniformLocation(shaderProgramObject, "u_light_position_blue");

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
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);

    console.log("inside draw\n");

    if (bLKeyPressed == true) {
        gl.uniform1i(LKeyPressUniform, 1);

        //setting light properties
        gl.uniform3fv(LaUniform_red, light_ambient_zero);
        gl.uniform3fv(LdUniform_red, light_diffuse_zero);
        gl.uniform3fv(LsUniform_red, light_specular_zero);
        gl.uniform4fv(LightPositionUniform_red, light_position_zero);
        //gl.uniform4f(LightPositionUniform_red, -2.0f,0.0f,0.0f);

        gl.uniform3fv(LaUniform_blue, light_ambient_one);
        gl.uniform3fv(LdUniform_blue, light_diffuse_one);
        gl.uniform3fv(LsUniform_blue, light_specular_one);
        gl.uniform4fv(LightPositionUniform_blue, light_position_one);
        //gl.uniform4f(LightPositionUniform_blue, 2.0f,0.0f,0.0f);

        //setting material properties
        gl.uniform3fv(KaUniform, material_ambient);
        gl.uniform3fv(KdUniform, material_diffuse);
        gl.uniform3fv(KsUniform, material_specular);
        gl.uniform1f(MaterialShininessUniform, material_shininess);
    } else {
        gl.uniform1i(LKeyPressUniform, 0);
    }

    var modelMatrix = mat4.create();
    var ViewMatrix = mat4.create();

    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -4.0]);
    mat4.rotateY(modelMatrix, modelMatrix, degToRad(angle_tri));

    //mat4.multiply(ViewMatrix, perspectiveProjectionMatrix, modelMatrix);

    gl.uniformMatrix4fv(ModelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(ViewMatrixUniform, false, ViewMatrix);
    gl.uniformMatrix4fv(ProjectionMatrixUniform, false, perspectiveProjectionMatrix);

    gl.bindVertexArray(vao_triangle);

    gl.drawArrays(gl.TRIANGLES, 0, 12);

    gl.bindVertexArray(null);

    gl.useProgram(null);

    angle_tri = angle_tri + 0.5;
    if (angle_tri >= 360.0)
        angle_tri = angle_tri - 360.0;

    //animation loop
    requestAnimationFrame(draw, canvas);
}

function uninitialize() {
    //code

    console.log("inside uninitialize\n");

    if (vao_triangle) {
        gl.deleteVertexArray(vao_triangle);
        vao_triangle = null;
    }

    if (vbo_position_triangle) {
        gl.deleteBuffer(vbo_position_triangle);
        vbo_position_triangle = null;
    }

    if (vbo_color_triangle) {
        gl.deleteBuffer(vbo_color_triangle);
        vbo_color_triangle = null;
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