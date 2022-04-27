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

var light_ambient = [0.0, 0.0, 0.0];
var light_diffuse = [1.0, 1.0, 1.0];
var light_specular = [1.0, 1.0, 1.0];
var light_position = [100.0, 100.0, 100.0, 1.0];

var material_ambient = [0.0, 0.0, 0.0];
var material_diffuse = [1.0, 1.0, 1.0];
var material_specular = [1.0, 1.0, 1.0];
var material_shininess = 128.0;

var sphere = null;

var ModelMatrixUniform;
var ViewMatrixUniform;
var ProjectionMatrixUniform;
var LaUniform;
var KaUniform;
var LdUniform;
var KdUniform;
var LsUniform;
var KsUniform;
var LightPositionUniform;
var MaterialShininessUniform;
var LKeyPressUniform;

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
        "in vec4 vPosition;" +
        "in vec3 vNormal;" +
        "uniform mat4 u_model_matrix;" +
        "uniform mat4 u_projection_matrix;" +
        "uniform mat4 u_view_matrix;" +
        "uniform mediump int u_LKeyPressed;" +
        "uniform vec4 u_light_position;" +
        "out vec3 tNorm;" +
        "out vec3 light_direction;" +
        "out vec3 view_vector;" +

        "void main(void)" +
        "{" +
        "if (u_LKeyPressed == 1)" +
        "{" +
        "vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;" +
        "tNorm = mat3(u_view_matrix * u_model_matrix) * vNormal;" +
        "light_direction = vec3(u_light_position) - eyeCoordinates.xyz;" +
        "view_vector = -eyeCoordinates.xyz;" +
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
        "vec3 phong_ads_color;" +
        "if (u_LKeyPressed == 1)" +
        "{" +
        "vec3 normalized_tNorm = normalize(tNorm);" +
        "vec3 normalized_light_direction = normalize(light_direction);" +
        "vec3 normalized_view_vector = normalize(view_vector);" +
        "vec3 ambient = u_La * u_Ka;" +
        "float tn_dot_ld = max(dot(normalized_light_direction,normalized_tNorm),0.0);" +
        "vec3 diffuse = u_Ld  * u_Kd * tn_dot_ld;" +
        "vec3 reflection_vector = reflect(-normalized_light_direction,normalized_tNorm);" +
        "vec3 specular = u_Ls  * u_Ks * pow(max(dot(reflection_vector,normalized_view_vector),0.0),u_material_shininess);" +
        "phong_ads_color = ambient + diffuse + specular;" +
        "}" +
        "else" +
        "{" +
        "phong_ads_color = vec3(1.0, 1.0, 1.0);" +
        "}" +
        "FragColor =vec4(phong_ads_color,1.0);" +
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

    ModelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_model_matrix");
    ProjectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projection_matrix");
    ViewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_view_matrix");

    LKeyPressUniform = gl.getUniformLocation(shaderProgramObject, "u_LKeyPressed");
    MaterialShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_material_shininess");

    LaUniform = gl.getUniformLocation(shaderProgramObject, "u_La");
    KaUniform = gl.getUniformLocation(shaderProgramObject, "u_Ka");

    LdUniform = gl.getUniformLocation(shaderProgramObject, "u_Ld");
    KdUniform = gl.getUniformLocation(shaderProgramObject, "u_Kd");

    LsUniform = gl.getUniformLocation(shaderProgramObject, "u_Ls");
    KsUniform = gl.getUniformLocation(shaderProgramObject, "u_Ks");
    LightPositionUniform = gl.getUniformLocation(shaderProgramObject, "u_light_position");

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
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);

    console.log("inside draw\n");

    if (bLKeyPressed == true) {
        gl.uniform1i(LKeyPressUniform, 1);

        //setting light properties
        gl.uniform3fv(LaUniform, light_ambient);
        gl.uniform3fv(LdUniform, light_diffuse);
        gl.uniform3fv(LsUniform, light_specular);
        gl.uniform4fv(LightPositionUniform, light_position);

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
    var projectionMatrix = mat4.create();

    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -6.0]);

    mat4.multiply(projectionMatrix, projectionMatrix, perspectiveProjectionMatrix);

    gl.uniformMatrix4fv(ModelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(ViewMatrixUniform, false, ViewMatrix);
    gl.uniformMatrix4fv(ProjectionMatrixUniform, false, projectionMatrix);

    sphere.draw();

    gl.useProgram(null);

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