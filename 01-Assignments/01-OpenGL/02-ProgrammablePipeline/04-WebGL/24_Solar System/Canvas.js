//global variable
var canvas = null;
var gl = null;
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;
var gbEnableLight = true;
var day = 0;
var year = 0;
var zIndex = -10.0;

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

var mvUniform;
var isLKeyPressed;
var ldUniform;
var kdUniform;
var lightPositionUniform;

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

        case 76:
            gbEnableLight = !gbEnableLight;
            break;

        case 89:
            //Y
            year = (year + 3) % 360;
            break;

        case 68:
            //D
            day = (day + 6) % 360;
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
        "precision highp float;" +
        "precision highp int;" +
        "in vec4 vPosition;" +
        "in vec3 vNormal;" +
        "uniform mat4 u_mv_matrix;" +
        "uniform mat4 u_projection_matrix;" +
        "uniform int u_lKeyIsPress;" +
        "uniform vec3 u_ld;" +
        "uniform vec3 u_kd;" +
        "out vec3 diffusedColor;" +
        "uniform vec4 u_light_position;" +

        "void main(void)" +
        "{" +
        "if(u_lKeyIsPress ==1)" +
        "{" +
        "vec4 eye_coordinate = u_mv_matrix * vPosition;" +
        "mat3 normal_matrix = mat3(((u_mv_matrix)));" +
        "vec3 tNorm = normalize(normal_matrix * vNormal);" +
        "vec3 s = vec3(u_light_position.xyz - eye_coordinate.xyz);" +
        "diffusedColor = u_ld * u_kd * max(dot(s,tNorm),0.0);" +
        "}" +
        "gl_Position = u_projection_matrix * u_mv_matrix*vPosition;" +
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
        "precision highp int;" +
        "in vec3 diffusedColor;" +
        "out vec4 FragColor;" +
        "uniform int u_lKeyIsPress;" +
        "void main(void)" +
        "{" +
        "if(u_lKeyIsPress == 1)" +
        "{" +
        "FragColor = vec4(diffusedColor, 1.0);" +
        "}" +
        "else" +
        "{" +
        "FragColor = vec4(1.0,1.0,1.0, 1.0);" +
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

    //get mvp uniform location

    mvUniform = gl.getUniformLocation(shaderProgramObject, "u_mv_matrix");
    perspectiveMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projection_matrix");
    isLKeyPressed = gl.getUniformLocation(shaderProgramObject, "u_lKeyIsPress");
    ldUniform = gl.getUniformLocation(shaderProgramObject, "u_ld");
    kdUniform = gl.getUniformLocation(shaderProgramObject, "u_kd");
    lightPositionUniform = gl.getUniformLocation(shaderProgramObject, "u_light_position");


    sun = new Mesh();
    makeSphere(sun, 2.0, 30, 30);
    earth = new Mesh();
    makeSphere(earth, 1.5, 15, 15);
    moon = new Mesh();
    makeSphere(moon, 0.5, 15, 15);

    //vertices, colors, shader attribs, vbo_position_triangle, vao_triangle initializations

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

    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();
    var translationMatrix = mat4.create();
    var rotationMatrix = mat4.create();
    var modelViewMatrix_sun = mat4.create();
    var modelViewMatrix_earth = mat4.create();
    var modelViewMatrix_moon = mat4.create();
    var tempMatrix = mat4.create();

    //--------------------------------------------------------sun
    mat4.identity(modelViewMatrix);
    mat4.identity(modelViewProjectionMatrix);
    mat4.identity(translationMatrix);
    mat4.identity(rotationMatrix);
    mat4.translate(translationMatrix, translationMatrix, [0.0, 0.0, zIndex]);
    mat4.multiply(modelViewMatrix_sun, translationMatrix, rotationMatrix);

    gl.uniformMatrix4fv(mvUniform, false, modelViewMatrix_sun);
    gl.uniformMatrix4fv(perspectiveMatrixUniform, false, perspectiveProjectionMatrix);
    if (gbEnableLight) {
        gl.uniform1i(isLKeyPressed, 1);
        gl.uniform3f(ldUniform, 1.0, 0.0, 1.0);
        gl.uniform3f(kdUniform, 0.5, 0.5, 0.5);
        gl.uniform4f(lightPositionUniform, 2.0, 0.0, 0.0, 1.0);

    } else {
        gl.uniform1i(isLKeyPressed, 0);
    }

    sun.draw();

    //----------------------------------------------------------------earth
    mat4.identity(modelViewMatrix);
    mat4.identity(modelViewProjectionMatrix);
    mat4.identity(translationMatrix);
    mat4.identity(rotationMatrix);

    mat4.translate(translationMatrix, translationMatrix, [0.0, 0.0, zIndex]);
    mat4.rotateY(rotationMatrix, rotationMatrix, degToRad(year));
    mat4.multiply(modelViewMatrix_sun, translationMatrix, rotationMatrix);

    mat4.rotateY(rotationMatrix, rotationMatrix, degToRad(90.0));
    mat4.translate(translationMatrix, translationMatrix, [-2.5, 0.0, zIndex]);
    mat4.multiply(modelViewMatrix_earth, translationMatrix, rotationMatrix);
    mat4.multiply(modelViewMatrix_earth, modelViewMatrix_sun, modelViewMatrix_earth);

    gl.uniformMatrix4fv(mvUniform, false, modelViewMatrix_earth);
    gl.uniformMatrix4fv(perspectiveMatrixUniform, false, perspectiveProjectionMatrix);
    if (gbEnableLight) {
        gl.uniform1i(isLKeyPressed, 1);
        gl.uniform3f(ldUniform, 1.0, 0.0, 1.0);
        gl.uniform3f(kdUniform, 0.5, 0.5, 0.5);
        gl.uniform4f(lightPositionUniform, 2.0, 0.0, 0.0, 1.0);

    } else {
        gl.uniform1i(isLKeyPressed, 0);
    }

    earth.draw();
    //-------------------------------------------------------------------moon
    mat4.identity(modelViewMatrix);
    mat4.identity(modelViewProjectionMatrix);
    mat4.identity(translationMatrix);
    mat4.identity(rotationMatrix);
    mat4.identity(tempMatrix);
    mat4.identity(modelViewMatrix_moon);

    mat4.rotateY(modelViewMatrix_earth, modelViewMatrix_earth, degToRad(day));

    mat4.translate(modelViewMatrix_earth, modelViewMatrix_earth, [2.5, 0.0, zIndex]);

    gl.uniformMatrix4fv(mvUniform, false, modelViewMatrix_earth);

    gl.uniformMatrix4fv(perspectiveMatrixUniform, false, perspectiveProjectionMatrix);
    if (gbEnableLight) {
        gl.uniform1i(isLKeyPressed, 1);
        gl.uniform3f(ldUniform, 1.0, 0.0, 1.0);
        gl.uniform3f(kdUniform, 0.5, 0.5, 0.5);
        gl.uniform4f(lightPositionUniform, 2.0, 0.0, 0.0, 1.0);

    } else {
        gl.uniform1i(isLKeyPressed, 0);
    }

    moon.draw();
    //----------------------------------------------------------------------------------------
    gl.useProgram(null);

    //animation loop
    requestAnimationFrame(draw, canvas);
}

function uninitialize() {
    //code

    console.log("inside uninitialize\n");

    if (shaderProgramObject) {
        if (fragmentShaderObject) {
            gl.detachShader(shaderProgramObject, fragmentShaderObject);
            gl.deleteShader(fragmentShaderObject);
            fragmentShaderObject = null;
        }
        if (vertexShaderObject) {
            gl.detachShader(shaderProgramObject, vertexShaderObject);
            gl.deleteShader(vertexShaderObject);
            vertexShaderObject = null;
        }

        gl.deleteProgram(shaderProgramObject);
        shaderProgramObject = null;
    }
}