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

var vao_triangle;
var vbo_position_triangle;
var vao_rectangle;
var vbo_position_rectangle;
var vbo_color_triangle;
var vbo_color_rectangle;
var mvpUniform;

var angle_tri = 0.0;
var angle_rec = 0.0;

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

    //vertices, colors, shader attribs, vbo_position_triangle, vao_triangle initializations
    var trianglevertices = new Float32Array([
        0.0, 1.0, 0.0,
        -1.0, -1.0, 1.0,
        1.0, -1.0, 1.0,

        0.0, 1.0, 0.0,
        1.0, -1.0, 1.0,
        1.0, -1.0, -1.0,

        0.0, 1.0, 0.0,
        1.0, -1.0, -1.0,
        -1.0, -1.0, -1.0,

        0.0, 1.0, 0.0,
        -1.0, -1.0, -1.0,
        -1.0, -1.0, 1.0
    ]);

    var trianglecolor = new Float32Array([
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,

        1.0, 0.0, 0.0,
        0.0, 0.0, 1.0,
        0.0, 1.0, 0.0,

        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,

        1.0, 0.0, 0.0,
        0.0, 0.0, 1.0,
        0.0, 1.0, 0.0
    ]);

    var rectanglevertices = new Float32Array([
        1.0, 1.0, -1.0,
        -1.0, 1.0, -1.0,
        -1.0, 1.0, 1.0,
        1.0, 1.0, 1.0,

        1.0, -1.0, 1.0,
        -1.0, -1.0, 1.0,
        -1.0, -1.0, -1.0,
        1.0, -1.0, -1.0,

        1.0, 1.0, 1.0,
        -1.0, 1.0, 1.0,
        -1.0, -1.0, 1.0,
        1.0, -1.0, 1.0,

        1.0, -1.0, -1.0,
        -1.0, -1.0, -1.0,
        -1.0, 1.0, -1.0,
        1.0, 1.0, -1.0,

        -1.0, 1.0, 1.0,
        -1.0, 1.0, -1.0,
        -1.0, -1.0, -1.0,
        -1.0, -1.0, 1.0,

        1.0, 1.0, -1.0,
        1.0, 1.0, 1.0,
        1.0, -1.0, 1.0,
        1.0, -1.0, -1.0
    ]);

    var rectanglecolor = new Float32Array([
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,

        1.0, 0.5, 0.0,
        1.0, 0.5, 0.0,
        1.0, 0.5, 0.0,
        1.0, 0.5, 0.0,

        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,

        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,

        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,

        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0
    ]);

    for (var i = 0; i < 72; i++) {
        if (rectanglevertices[i] > 0.0)
            rectanglevertices[i] = rectanglevertices[i] - 0.25;
        else if (rectanglevertices[i] < 0.0)
            rectanglevertices[i] = rectanglevertices[i] + 0.25;
        else
            rectanglevertices[i] = rectanglevertices[i];
    }

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

    mat4.translate(modelViewMatrix, modelViewMatrix, [-1.5, 0.0, -6.0]);
    mat4.rotateY(modelViewMatrix, modelViewMatrix, degToRad(angle_tri));

    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

    gl.bindVertexArray(vao_triangle);

    gl.drawArrays(gl.TRIANGLES, 0, 12);

    gl.bindVertexArray(null);

    //-------------------------------------------------------------------------------------
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    mat4.translate(modelViewMatrix, modelViewMatrix, [1.5, 0.0, -6.0]);
    mat4.rotateY(modelViewMatrix, modelViewMatrix, degToRad(angle_rec));

    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

    gl.bindVertexArray(vao_rectangle);

    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);

    gl.bindVertexArray(null);

    gl.useProgram(null);

    angle_tri = angle_tri + 0.5;
    if (angle_tri >= 360.0)
        angle_tri = angle_tri - 360.0;

    angle_rec = angle_rec + 0.5;
    if (angle_rec >= 360.0)
        angle_rec = angle_rec - 360.0;

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