//global variable
var canvas = null;
var gl = null;
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;

//when whole webglmacros is const , all inside it are automatically const
const WebGLMacros = {
    AMC_ATTRIBUTE_VERTEX: 0,
    AMC_ATTRIBUTE_COLOR: 1,
    AMC_ATTRIBUTE_NORMAL: 2,
    AMC_ATTRIBUTE_TEXTURE0: 3,
};

var vertexShaderObject;
var fragmentShaderObject;
var shaderProgramObject;

var vao_red;
var vao_green;
var vao_blue;
var vao_triangle;
var vao_rectangle;
var vao_inner_circle;

var vbo_red_line_position;
var vbo_green_line_position;
var vbo_blue_line_position;
var vbo_position_rectangle;
var vbo_position_triangle;
var vbo_position_inner_circle;

var vbo_red_line_color;
var vbo_green_line_color;
var vbo_blue_line_color;
var vbo_color_triangle;
var vbo_color_rectangle;
var vbo_color_inner_circle;
var mvpUniform;

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
        "in vec4 vColor;" +
        "out vec4 out_color;" +
        "uniform mat4 u_mvp_matrix;" +
        "void main(void)" +
        "{" +
        "gl_Position = u_mvp_matrix * vPosition;" +
        "out_color = vColor;" +
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
        "in vec4 out_color;" +
        "void main(void)" +
        "{" +
        "FragColor = out_color;" +
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
    gl.bindAttribLocation(shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_VERTEX, "vPosition");
    gl.bindAttribLocation(shaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_COLOR, "vColor");

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
    mvpUniform = gl.getUniformLocation(shaderProgramObject, "u_mvp_matrix");

    //vertices, colors, shader attribs, vbo_red_line_position, vao_red initializations
    var redLine = new Float32Array([
        1.0, 0.0, 0.0, -1.0, 0.0, 0.0,
    ]);

    var redColor = new Float32Array([
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0
    ]);

    var greenLine = new Float32Array([
        0.0, 1.0, 0.0,
        0.0, -1.0, 0.0,
    ]);

    var greenColor = new Float32Array([
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0
    ]);

    var blueLine = new Float32Array([-0.95, 1.0, 0.0, -0.95, -1.0, 0.0,

        -0.90, 1.0, 0.0, -0.90, -1.0, 0.0,

        -0.85, 1.0, 0.0, -0.85, -1.0, 0.0,

        -0.80, 1.0, 0.0, -0.80, -1.0, 0.0,

        -0.75, 1.0, 0.0, -0.75, -1.0, 0.0,

        -0.70, 1.0, 0.0, -0.70, -1.0, 0.0,

        -0.65, 1.0, 0.0, -0.65, -1.0, 0.0,

        -0.60, 1.0, 0.0, -0.60, -1.0, 0.0,

        -0.55, 1.0, 0.0, -0.55, -1.0, 0.0,

        -0.50, 1.0, 0.0, -0.50, -1.0, 0.0,

        -0.45, 1.0, 0.0, -0.45, -1.0, 0.0,

        -0.40, 1.0, 0.0, -0.40, -1.0, 0.0,

        -0.35, 1.0, 0.0, -0.35, -1.0, 0.0,

        -0.30, 1.0, 0.0, -0.30, -1.0, 0.0,

        -0.25, 1.0, 0.0, -0.25, -1.0, 0.0,

        -0.20, 1.0, 0.0, -0.20, -1.0, 0.0,

        -0.15, 1.0, 0.0, -0.15, -1.0, 0.0,

        -0.10, 1.0, 0.0, -0.10, -1.0, 0.0,

        -0.05, 1.0, 0.0, -0.05, -1.0, 0.0,

        0.95, 1.0, 0.0,
        0.95, -1.0, 0.0,

        0.90, 1.0, 0.0,
        0.90, -1.0, 0.0,

        0.85, 1.0, 0.0,
        0.85, -1.0, 0.0,

        0.80, 1.0, 0.0,
        0.80, -1.0, 0.0,

        0.75, 1.0, 0.0,
        0.75, -1.0, 0.0,

        0.70, 1.0, 0.0,
        0.70, -1.0, 0.0,

        0.65, 1.0, 0.0,
        0.65, -1.0, 0.0,

        0.60, 1.0, 0.0,
        0.60, -1.0, 0.0,

        0.55, 1.0, 0.0,
        0.55, -1.0, 0.0,

        0.50, 1.0, 0.0,
        0.50, -1.0, 0.0,

        0.45, 1.0, 0.0,
        0.45, -1.0, 0.0,

        0.40, 1.0, 0.0,
        0.40, -1.0, 0.0,

        0.35, 1.0, 0.0,
        0.35, -1.0, 0.0,

        0.30, 1.0, 0.0,
        0.30, -1.0, 0.0,

        0.25, 1.0, 0.0,
        0.25, -1.0, 0.0,

        0.20, 1.0, 0.0,
        0.20, -1.0, 0.0,

        0.15, 1.0, 0.0,
        0.15, -1.0, 0.0,

        0.10, 1.0, 0.0,
        0.10, -1.0, 0.0,

        0.05, 1.0, 0.0,
        0.05, -1.0, 0.0,

        1.0, -0.95, 0.0, -1.0, -0.95, 0.0,

        1.0, -0.90, 0.0, -1.0, -0.90, 0.0,

        1.0, -0.85, 0.0, -1.0, -0.85, 0.0,

        1.0, -0.80, 0.0, -1.0, -0.80, 0.0,

        1.0, -0.75, 0.0, -1.0, -0.75, 0.0,

        1.0, -0.70, 0.0, -1.0, -0.70, 0.0,

        1.0, -0.65, 0.0, -1.0, -0.65, 0.0,

        1.0, -0.60, 0.0, -1.0, -0.60, 0.0,

        1.0, -0.55, 0.0, -1.0, -0.55, 0.0,

        1.0, -0.50, 0.0, -1.0, -0.50, 0.0,

        1.0, -0.45, 0.0, -1.0, -0.45, 0.0,

        1.0, -0.40, 0.0, -1.0, -0.40, 0.0,

        1.0, -0.35, 0.0, -1.0, -0.35, 0.0,

        1.0, -0.30, 0.0, -1.0, -0.30, 0.0,

        1.0, -0.25, 0.0, -1.0, -0.25, 0.0,

        1.0, -0.20, 0.0, -1.0, -0.20, 0.0,

        1.0, -0.15, 0.0, -1.0, -0.15, 0.0,

        1.0, -0.10, 0.0, -1.0, -0.10, 0.0,

        1.0, -0.05, 0.0, -1.0, -0.05, 0.0,

        1.0, 0.95, 0.0, -1.0, 0.95, 0.0,

        1.0, 0.90, 0.0, -1.0, 0.90, 0.0,

        1.0, 0.85, 0.0, -1.0, 0.85, 0.0,

        1.0, 0.80, 0.0, -1.0, 0.80, 0.0,

        1.0, 0.75, 0.0, -1.0, 0.75, 0.0,

        1.0, 0.70, 0.0, -1.0, 0.70, 0.0,

        1.0, 0.65, 0.0, -1.0, 0.65, 0.0,

        1.0, 0.60, 0.0, -1.0, 0.60, 0.0,

        1.0, 0.55, 0.0, -1.0, 0.55, 0.0,

        1.0, 0.50, 0.0, -1.0, 0.50, 0.0,

        1.0, 0.45, 0.0, -1.0, 0.45, 0.0,

        1.0, 0.40, 0.0, -1.0, 0.40, 0.0,

        1.0, 0.35, 0.0, -1.0, 0.35, 0.0,

        1.0, 0.30, 0.0, -1.0, 0.30, 0.0,

        1.0, 0.25, 0.0, -1.0, 0.25, 0.0,

        1.0, 0.20, 0.0, -1.0, 0.20, 0.0,

        1.0, 0.15, 0.0, -1.0, 0.15, 0.0,

        1.0, 0.10, 0.0, -1.0, 0.10, 0.0,

        1.0, 0.05, 0.0, -1.0, 0.05, 0.0
    ]);

    var trianglevertices = new Float32Array([
        0.0, 0.5, 0.0, -0.5, -0.5, 0.0, -0.5, -0.5, 0.0,
        0.5, -0.5, 0.0,
        0.5, -0.5, 0.0,
        0.0, 0.5, 0.0,
    ]);

    var trianglecolor = new Float32Array([
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
    ]);

    var rectanglevertices = new Float32Array([-0.5, -0.5, 0.0,
        0.5, -0.5, 0.0, -0.5, -0.5, 0.0, -0.5, 0.5, 0.0, -0.5, 0.5, 0.0,
        0.5, 0.5, 0.0,
        0.5, 0.5, 0.0,
        0.5, -0.5, 0.0,
    ]);

    var rectanglecolor = new Float32Array([
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
    ]);

    var circlecolor = new Float32Array([
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,

        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,

        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
    ]);

    vao_red = gl.createVertexArray();
    gl.bindVertexArray(vao_red);

    vbo_red_line_position = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_red_line_position);
    gl.bufferData(gl.ARRAY_BUFFER, redLine, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vbo_red_line_color = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_red_line_color);
    gl.bufferData(gl.ARRAY_BUFFER, redColor, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);

    console.log("vao_red,vbo_red_line_position succeeded\n");

    vao_green = gl.createVertexArray();
    gl.bindVertexArray(vao_green);

    vbo_green_line_position = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_green_line_position);
    gl.bufferData(gl.ARRAY_BUFFER, greenLine, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vbo_green_line_color = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_green_line_color);
    gl.bufferData(gl.ARRAY_BUFFER, greenColor, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);

    console.log("vao_green,vbo_green_line_position succeeded\n");

    vao_blue = gl.createVertexArray();
    gl.bindVertexArray(vao_blue);

    vbo_blue_line_position = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_blue_line_position);
    gl.bufferData(gl.ARRAY_BUFFER, blueLine, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.vertexAttrib3f(WebGLMacros.AMC_ATTRIBUTE_COLOR, 0.0, 0.0, 1.0);

    gl.bindVertexArray(null);

    console.log("vao_blue,vbo_blue_line_position succeeded\n");

    //-----------------------------------------------------------------------------------------

    vao_triangle = gl.createVertexArray();
    gl.bindVertexArray(vao_triangle);

    vbo_position_triangle = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position_triangle);
    gl.bufferData(gl.ARRAY_BUFFER, trianglevertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vbo_color_triangle = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_color_triangle);
    gl.bufferData(gl.ARRAY_BUFFER, trianglecolor, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);

    console.log("vao_triangle,vbo_position_triangle succeeded\n");

    vao_rectangle = gl.createVertexArray();
    gl.bindVertexArray(vao_rectangle);

    vbo_position_rectangle = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position_rectangle);
    gl.bufferData(gl.ARRAY_BUFFER, rectanglevertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vbo_color_rectangle = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_color_rectangle);
    gl.bufferData(gl.ARRAY_BUFFER, rectanglecolor, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);

    console.log("vao_rectangle,vbo_position_rectangle succeeded\n");

    //---------------------------------------------------------------------------------------

    var innerCircleVertex = new Float32Array(10);
    var radius_inner = 0.308;

    vao_inner_circle = gl.createVertexArray();
    gl.bindVertexArray(vao_inner_circle);

    for (var angle = 0.0; angle < (2.0 * 3.142); angle = angle + 0.01) {
        innerCircleVertex[0] = ((Math.cos(angle) * radius_inner));
        innerCircleVertex[1] = ((Math.sin(angle) * radius_inner));
        innerCircleVertex[2] = 0.0;
    }

    vbo_position_inner_circle = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position_inner_circle);
    gl.bufferData(gl.ARRAY_BUFFER, innerCircleVertex, gl.DYNAMIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vbo_color_inner_circle = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_color_inner_circle);
    gl.bufferData(gl.ARRAY_BUFFER, circlecolor, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);
    //---------------------------------------------------------------------------------- 

    //set claer color
    gl.clearColor(0.0, 0.0, 0.0, 1.0); //black

    //depth test will always be enabled
    gl.enable(gl.DEPTH_TEST);

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

    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -1.2]);

    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

    gl.bindVertexArray(vao_red);

    gl.drawArrays(gl.LINES, 0, 2);

    gl.bindVertexArray(null);

    gl.bindVertexArray(vao_green);

    gl.drawArrays(gl.LINES, 0, 2);

    gl.bindVertexArray(null);

    gl.bindVertexArray(vao_blue);

    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 2, 2);
    gl.drawArrays(gl.LINES, 4, 2);
    gl.drawArrays(gl.LINES, 6, 2);
    gl.drawArrays(gl.LINES, 8, 2);
    gl.drawArrays(gl.LINES, 10, 2);
    gl.drawArrays(gl.LINES, 12, 2);
    gl.drawArrays(gl.LINES, 14, 2);
    gl.drawArrays(gl.LINES, 16, 2);
    gl.drawArrays(gl.LINES, 18, 2);
    gl.drawArrays(gl.LINES, 20, 2);

    gl.drawArrays(gl.LINES, 22, 2);
    gl.drawArrays(gl.LINES, 24, 2);
    gl.drawArrays(gl.LINES, 26, 2);
    gl.drawArrays(gl.LINES, 28, 2);
    gl.drawArrays(gl.LINES, 30, 2);
    gl.drawArrays(gl.LINES, 32, 2);
    gl.drawArrays(gl.LINES, 34, 2);
    gl.drawArrays(gl.LINES, 36, 2);
    gl.drawArrays(gl.LINES, 38, 2);
    gl.drawArrays(gl.LINES, 40, 2);
    gl.drawArrays(gl.LINES, 42, 2);

    gl.drawArrays(gl.LINES, 44, 2);
    gl.drawArrays(gl.LINES, 46, 2);
    gl.drawArrays(gl.LINES, 48, 2);
    gl.drawArrays(gl.LINES, 50, 2);
    gl.drawArrays(gl.LINES, 52, 2);
    gl.drawArrays(gl.LINES, 54, 2);
    gl.drawArrays(gl.LINES, 56, 2);
    gl.drawArrays(gl.LINES, 58, 2);
    gl.drawArrays(gl.LINES, 60, 2);
    gl.drawArrays(gl.LINES, 62, 2);
    gl.drawArrays(gl.LINES, 64, 2);

    gl.drawArrays(gl.LINES, 66, 2);
    gl.drawArrays(gl.LINES, 68, 2);
    gl.drawArrays(gl.LINES, 70, 2);
    gl.drawArrays(gl.LINES, 72, 2);
    gl.drawArrays(gl.LINES, 74, 2);
    gl.drawArrays(gl.LINES, 76, 2);
    gl.drawArrays(gl.LINES, 78, 2);
    gl.drawArrays(gl.LINES, 80, 2);
    gl.drawArrays(gl.LINES, 82, 2);
    gl.drawArrays(gl.LINES, 84, 2);
    gl.drawArrays(gl.LINES, 86, 2);

    gl.drawArrays(gl.LINES, 88, 2);
    gl.drawArrays(gl.LINES, 90, 2);
    gl.drawArrays(gl.LINES, 92, 2);
    gl.drawArrays(gl.LINES, 94, 2);
    gl.drawArrays(gl.LINES, 96, 2);
    gl.drawArrays(gl.LINES, 98, 2);
    gl.drawArrays(gl.LINES, 100, 2);
    gl.drawArrays(gl.LINES, 102, 2);
    gl.drawArrays(gl.LINES, 104, 2);
    gl.drawArrays(gl.LINES, 106, 2);
    gl.drawArrays(gl.LINES, 108, 2);

    gl.drawArrays(gl.LINES, 110, 2);
    gl.drawArrays(gl.LINES, 112, 2);
    gl.drawArrays(gl.LINES, 114, 2);
    gl.drawArrays(gl.LINES, 116, 2);
    gl.drawArrays(gl.LINES, 118, 2);
    gl.drawArrays(gl.LINES, 120, 2);
    gl.drawArrays(gl.LINES, 122, 2);
    gl.drawArrays(gl.LINES, 124, 2);
    gl.drawArrays(gl.LINES, 126, 2);
    gl.drawArrays(gl.LINES, 128, 2);
    gl.drawArrays(gl.LINES, 130, 2);

    gl.drawArrays(gl.LINES, 132, 2);
    gl.drawArrays(gl.LINES, 134, 2);
    gl.drawArrays(gl.LINES, 136, 2);
    gl.drawArrays(gl.LINES, 138, 2);
    gl.drawArrays(gl.LINES, 140, 2);
    gl.drawArrays(gl.LINES, 142, 2);
    gl.drawArrays(gl.LINES, 144, 2);
    gl.drawArrays(gl.LINES, 146, 2);
    gl.drawArrays(gl.LINES, 148, 2);
    gl.drawArrays(gl.LINES, 150, 2);
    gl.drawArrays(gl.LINES, 152, 2);

    gl.drawArrays(gl.LINES, 154, 2);
    gl.drawArrays(gl.LINES, 156, 2);
    gl.drawArrays(gl.LINES, 158, 2);
    gl.drawArrays(gl.LINES, 160, 2);
    gl.drawArrays(gl.LINES, 162, 2);
    gl.drawArrays(gl.LINES, 164, 2);

    gl.bindVertexArray(null);

    //--------------------------------------------------------------------------------------------------------
    //mat4.identity(modelViewMatrix);
    //mat4.identity(modelViewProjectionMatrix);

    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.001, -2.0]);

    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

    gl.bindVertexArray(vao_triangle);

    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 2, 2);
    gl.drawArrays(gl.LINES, 4, 2);

    gl.bindVertexArray(null);

    //-------------------------------------------------------------------------------------
    //mat4.identity(modelViewMatrix);
    //mat4.identity(modelViewProjectionMatrix);

    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    mat4.translate(modelViewMatrix, modelViewMatrix, [0.003, 0.003, -2.0]);

    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

    gl.bindVertexArray(vao_rectangle);

    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 2, 2);
    gl.drawArrays(gl.LINES, 4, 2);
    gl.drawArrays(gl.LINES, 6, 2);

    gl.bindVertexArray(null);
    //-----------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    var innerCircleVertex = new Float32Array(4);
    var radius_inner = 0.308;

    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -2.0]);
    mat4.rotateY(modelViewMatrix, modelViewMatrix, degToRad(angle_circle));

    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

    gl.bindVertexArray(vao_inner_circle);

    for (var angle = 0.0; angle < (2.0 * 3.142); angle = angle + 0.01) {
        innerCircleVertex[0] = ((Math.cos(angle) * radius_inner));
        innerCircleVertex[1] = ((Math.sin(angle) * radius_inner));
        innerCircleVertex[2] = 0.0;
    }

    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position_inner_circle);
    gl.bufferData(gl.ARRAY_BUFFER, innerCircleVertex, gl.DYNAMIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    //gl.pointSize(2.0);
    gl.drawArrays(gl.POINTS, 0, 1);

    gl.bindVertexArray(null);
    //---------------------------------------------------------

    gl.useProgram(null);

    //animation loop
    requestAnimationFrame(draw, canvas);
}

function uninitialize() {
    //code

    console.log("inside uninitialize\n");

    if (vao_red) {
        gl.deleteVertexArray(vao_red);
        vao_red = null;
    }

    if (vbo_red_line_position) {
        gl.deleteBuffer(vbo_red_line_position);
        vbo_red_line_position = null;
    }

    if (vbo_red_line_color) {
        gl.deleteBuffer(vbo_red_line_color);
        vbo_red_line_color = null;
    }

    if (vao_green) {
        gl.deleteVertexArray(vao_green);
        vao_green = null;
    }

    if (vbo_green_line_position) {
        gl.deleteBuffer(vbo_green_line_position);
        vbo_green_line_position = null;
    }

    if (vbo_green_line_color) {
        gl.deleteBuffer(vbo_green_line_color);
        vbo_green_line_color = null;
    }

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

    if (vao_rectangle) {
        gl.deleteVertexArray(vao_rectangle);
        vao_rectangle = null;
    }

    if (vbo_position_rectangle) {
        gl.deleteBuffer(vbo_position_rectangle);
        vbo_position_rectangle = null;
    }

    if (vbo_color_rectangle) {
        gl.deleteBuffer(vbo_color_rectangle);
        vbo_color_rectangle = null;
    }

    if (vao_inner_circle) {
        gl.deleteVertexArray(vao_inner_circle);
        vao_inner_circle = null;
    }

    if (vbo_position_inner_circle) {
        gl.deleteBuffer(vbo_position_inner_circle);
        vbo_position_inner_circle = null;
    }

    if (vbo_color_inner_circle) {
        gl.deleteBuffer(vbo_color_inner_circle);
        vbo_color_inner_circle = null;
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