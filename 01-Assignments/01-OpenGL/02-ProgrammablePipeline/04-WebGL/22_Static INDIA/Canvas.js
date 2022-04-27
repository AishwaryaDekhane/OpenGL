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

var mvpUniform;

var vao_first_I;
var vbo_first_I_pos;
var vbo_first_I_col;

var vao_N;
var vbo_N_pos;
var vbo_N_col;

var vao_D;
var vbo_D_pos;
var vbo_D_col;

var vao_last_I;
var vbo_last_I_pos;
var vbo_last_I_col;

var vao_A;
var vbo_A_pos;
var vbo_A_col;

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

    //vertices, colors, shader attribs, vbo_A_pos, vao_A initializations

    //--------------------------------------------------------------------------------------------------------------
    var first_I_pos = new Float32Array([

        -0.56, 0.5, 0.0, -0.56, -0.5, 0.0, -0.65, 0.5, 0.0, -0.47, 0.5, 0.0, -0.65, -0.5, 0.0, -0.47, -0.5, 0.0,
    ]);

    var first_I_col = new Float32Array([
        1.0, 0.6, 0.2,
        0.0705882353, 0.53333, 0.0274509804,
        1.0, 0.6, 0.2,
        1.0, 0.6, 0.2,
        0.0705882353, 0.53333, 0.0274509804,
        0.0705882353, 0.53333, 0.0274509804,

    ]);

    vao_first_I = gl.createVertexArray();
    gl.bindVertexArray(vao_first_I);

    vbo_first_I_pos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_first_I_pos);
    gl.bufferData(gl.ARRAY_BUFFER, first_I_pos, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vbo_first_I_col = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_first_I_col);
    gl.bufferData(gl.ARRAY_BUFFER, first_I_col, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);

    console.log("vao first i succeeded\n");
    //-------------------------------------------------------------------------------------------------------------
    var lineNpos = new Float32Array([-0.37, 0.5, 0.0, -0.37, -0.5, 0.0, -0.19, 0.5, 0.0, -0.19, -0.5, 0.0, -0.37, 0.5, 0.0, -0.19, -0.5, 0.0, ]);

    var lineNCol = new Float32Array([
        1.0, 0.6, 0.2,
        0.0705882353, 0.53333, 0.0274509804,
        1.0, 0.6, 0.2,
        0.0705882353, 0.53333, 0.0274509804,
        1.0, 0.6, 0.2,
        0.0705882353, 0.53333, 0.0274509804,
    ]);

    vao_N = gl.createVertexArray();
    gl.bindVertexArray(vao_N);

    vbo_N_pos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_N_pos);
    gl.bufferData(gl.ARRAY_BUFFER, lineNpos, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vbo_N_col = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_N_col);
    gl.bufferData(gl.ARRAY_BUFFER, lineNCol, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);

    console.log("vao N succeeded\n");
    //---------------------------------------------------------------------------------------------------------------------------------
    var Dpos = new Float32Array([-0.09, 0.5, 0.0, -0.09, -0.5, 0.0,
        0.09, 0.4, 0.0,
        0.09, -0.4, 0.0, -0.09, 0.5, 0.0,
        0.04, 0.5, 0.0, -0.09, -0.5, 0.0,
        0.04, -0.5, 0.0,
        0.04, 0.5, 0.0,
        0.09, 0.4, 0.0,
        0.04, -0.5, 0.0,
        0.09, -0.4, 0.0,
    ]);

    var DCol = new Float32Array([
        1.0, 0.6, 0.2,
        0.0705882353, 0.53333, 0.0274509804,
        1.0, 0.6, 0.2,
        0.0705882353, 0.53333, 0.0274509804,
        1.0, 0.6, 0.2,
        1.0, 0.6, 0.2,
        0.0705882353, 0.53333, 0.0274509804,
        0.0705882353, 0.53333, 0.0274509804,
        1.0, 0.6, 0.2,
        1.0, 0.6, 0.2,
        0.0705882353, 0.53333, 0.0274509804,
        0.0705882353, 0.53333, 0.0274509804,
    ]);

    vao_D = gl.createVertexArray();
    gl.bindVertexArray(vao_D);

    vbo_D_pos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_D_pos);
    gl.bufferData(gl.ARRAY_BUFFER, Dpos, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vbo_D_col = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_D_col);
    gl.bufferData(gl.ARRAY_BUFFER, DCol, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);

    console.log("vao d succeeded\n");
    //------------------------------------------------------------------------------------------------------------------
    var second_I_pos = new Float32Array([
        0.28, 0.5, 0.0,
        0.28, -0.5, 0.0,
        0.19, 0.5, 0.0,
        0.37, 0.5, 0.0,
        0.19, -0.5, 0.0,
        0.37, -0.5, 0.0,
    ]);

    var second_I_col = new Float32Array([
        1.0, 0.6, 0.2,
        0.0705882353, 0.53333, 0.0274509804,
        1.0, 0.6, 0.2,
        1.0, 0.6, 0.2,
        0.0705882353, 0.53333, 0.0274509804,
        0.0705882353, 0.53333, 0.0274509804,
    ]);


    vao_last_I = gl.createVertexArray();
    gl.bindVertexArray(vao_last_I);

    vbo_last_I_pos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_last_I_pos);
    gl.bufferData(gl.ARRAY_BUFFER, second_I_pos, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vbo_last_I_col = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_last_I_col);
    gl.bufferData(gl.ARRAY_BUFFER, second_I_col, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);

    console.log("vao i2 succeeded\n");
    //------------------------------------------------------------------------------------------------------------

    var A_pos = new Float32Array([
        0.56, 0.5, 0.0,
        0.47, -0.5, 0.0,
        0.56, 0.5, 0.0,
        0.65, -0.5, 0.0,
        0.52, 0.0, 0.0,
        0.60, 0.0, 0.0,
        0.525, 0.05, 0.0,
        0.595, 0.05, 0.0,
        0.51, -0.05, 0.0,
        0.61, -0.05, 0.0,
    ]);

    var A_col = new Float32Array([
        1.0, 0.6, 0.2,
        0.0705882353, 0.53333, 0.0274509804,
        1.0, 0.6, 0.2,
        0.0705882353, 0.53333, 0.0274509804,
        1.0, 1.0, 1.0,
        1.0, 1.0, 1.0,
        1.0, 0.6, 0.2,
        1.0, 0.6, 0.2,
        0.0705882353, 0.53333, 0.0274509804,
        0.0705882353, 0.53333, 0.0274509804,
    ]);


    vao_A = gl.createVertexArray();
    gl.bindVertexArray(vao_A);

    vbo_A_pos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_A_pos);
    gl.bufferData(gl.ARRAY_BUFFER, A_pos, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vbo_A_col = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_A_col);
    gl.bufferData(gl.ARRAY_BUFFER, A_col, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);

    console.log("vao a succeeded\n");
    //-------------------------------------------------------------------------------------------------------------------
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

    //========================================================================================
    //-------------------------------------------------------------------------------------------------------------------

    //for first I

    //do necessary transformation if ant required
    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -2.0]);

    //do necessary matrix multiplication
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major

    gl.getParameter(gl.LINE_WIDTH);
    gl.lineWidth(9.0);

    //bind with vao
    gl.bindVertexArray(vao_first_I);

    //draw scene
    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 2, 2);
    gl.drawArrays(gl.LINES, 4, 2);

    //unbind vao
    gl.bindVertexArray(null);
    //------------------------------------------------------------------------------------------------------------------

    //for line N

    //initialize above matrices to identity
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    //do necessary transformation if ant required
    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -2.0]);

    //do necessary matrix multiplication
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major

    gl.getParameter(gl.LINE_WIDTH);
    gl.lineWidth(9.0);

    //bind with vao
    gl.bindVertexArray(vao_N);

    //draw scene
    //glLineWidth(2.0f);

    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 2, 2);
    gl.drawArrays(gl.LINES, 4, 2);

    //unbind vao
    gl.bindVertexArray(null);
    //------------------------------------------------------------------------------------------------------------------

    //for D letter

    //initialize above matrices to identity
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    //do necessary transformation if ant required
    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -2.0]);

    //do necessary matrix multiplication
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major

    gl.lineWidth(9.0);

    //bind with vao
    gl.bindVertexArray(vao_D);

    //draw scene

    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 2, 2);
    gl.drawArrays(gl.LINES, 4, 2);
    gl.drawArrays(gl.LINES, 6, 2);
    gl.drawArrays(gl.LINES, 8, 2);
    gl.drawArrays(gl.LINES, 10, 2);

    //unbind vao
    gl.bindVertexArray(null);

    //--------------------------------------------------------------------------------------------------------------------


    //for last I

    //initialize above matrices to identity
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    //do necessary transformation if ant required
    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -2.0]);

    //do necessary matrix multiplication
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major

    gl.lineWidth(9.0);

    //bind with vao
    gl.bindVertexArray(vao_last_I);

    //draw scene
    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 2, 2);
    gl.drawArrays(gl.LINES, 4, 2);

    //unbind vao
    gl.bindVertexArray(null);
    //-------------------------------------------------------------------------------------------------------------------

    //for A

    //initialize above matrices to identity
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    //do necessary transformation if ant required
    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -2.0]);

    //do necessary matrix multiplication
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);
    //GL_FALSE = internally we are not transposing = row -> col / col -> row
    //OpenGL is col major while DirectX is row major

    gl.lineWidth(9.0);

    //bind with vao
    gl.bindVertexArray(vao_A);

    //draw scene
    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 2, 2);
    gl.drawArrays(gl.LINES, 4, 2);
    gl.drawArrays(gl.LINES, 6, 2);
    gl.drawArrays(gl.LINES, 8, 2);

    //unbind vao
    gl.bindVertexArray(null);
    //-------------------------------------------------------------------------------------------------------------------
    gl.useProgram(null);

    //animation loop
    requestAnimationFrame(draw, canvas);
}

function uninitialize() {
    //code

    console.log("inside uninitialize\n");

    if (vao_A) {
        gl.deleteVertexArray(vao_A);
        vao_A = null;
    }

    if (vbo_A_pos) {
        gl.deleteBuffer(vbo_A_pos);
        vbo_A_pos = null;
    }

    if (vbo_A_col) {
        gl.deleteBuffer(vbo_A_col);
        vbo_A_col = null;
    }

    if (vao_D) {
        gl.deleteVertexArray(vao_D);
        vao_D = null;
    }

    if (vbo_D_pos) {
        gl.deleteBuffer(vbo_D_pos);
        vbo_D_pos = null;
    }

    if (vbo_D_col) {
        gl.deleteBuffer(vbo_D_col);
        vbo_D_col = null;
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