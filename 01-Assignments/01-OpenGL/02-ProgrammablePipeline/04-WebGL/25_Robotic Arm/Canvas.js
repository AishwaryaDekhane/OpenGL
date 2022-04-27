//global variable
var canvas = null;
var gl = null;
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;
var gbEnableLight = true;

var Shoulder = 0;
var Elbow = 0;

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

var modelMatrixUniform, viewMatrixUniform, projectionMatrixUniform;

var perspectiveMatrixProjection;

var sphere_shoulder = null;
var sphere_elbow = null;

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

        case 69: //E
            Elbow = (Elbow + 3) % 360;
            break;
        case 101: //e
            Elbow = (Elbow - 3) % 360;
            break;

        case 115: //s 
            Shoulder = (Shoulder - 3) % 360;
            break;
        case 83: //S
            Shoulder = (Shoulder + 3) % 360;
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
        "uniform mat4 u_model_matrix;" +
        "uniform mat4 u_view_matrix;" +
        "uniform mat4 u_projection_matrix;" +
        "out vec4 out_color;" +
        "void main(void)" +
        "{" +
        "gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" +
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
        "out vec4 FragColor;" +
        "void main(void)" +
        "{" +
        "FragColor = vec4(0.5, 0.35, 0.05, 1.0);" +
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

    modelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_model_matrix");
    viewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_view_matrix");
    projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projection_matrix");


    //sphere
    sphere_shoulder = new Mesh();
    makeSphere(sphere_shoulder, 0.5, 10, 10);

    sphere_elbow = new Mesh();
    makeSphere(sphere_elbow, 0.5, 10, 10);

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
    perspectiveMatrixProjection = mat4.create();
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
    mat4.perspective(perspectiveMatrixProjection, 45.0, parseFloat(canvas.width) / parseFloat(canvas.height), 0.1, 100);

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

    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();

    //Shoulder
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -12.0]);
    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);

    mat4.rotateZ(modelMatrix, modelMatrix, degToRad(Shoulder));
    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);

    mat4.translate(modelMatrix, modelMatrix, [1.0, 0.0, 0.0]);
    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);

    mat4.scale(modelMatrix, modelMatrix, [2.0, 0.5, 1.0]);
    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);

    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveMatrixProjection);

    sphere_shoulder.draw();

    //Elbow
    mat4.identity(modelMatrix);
    mat4.identity(viewMatrix);

    //do necessary transformations here
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -12.0]);
    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);

    mat4.rotateZ(modelMatrix, modelMatrix, degToRad(Shoulder));
    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);

    mat4.translate(modelMatrix, modelMatrix, [2.0, 0.0, 0.0]);
    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);

    mat4.rotateZ(modelMatrix, modelMatrix, degToRad(Elbow));
    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);

    mat4.translate(modelMatrix, modelMatrix, [1.0, 0.0, 0.0]);
    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);

    mat4.scale(modelMatrix, modelMatrix, [2.0, 0.5, 1.0]);
    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);

    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveMatrixProjection);

    sphere_elbow.draw();

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