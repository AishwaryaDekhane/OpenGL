//global variable
var canvas = null;
var gl = null;
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;
var gbEnableLight = true;
var lightAngle = 0.0;
var giWidth;
var giHeight;

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

var lightAmbient = new Float32Array([0.0, 0.0, 0.0]);
var lightDiffused = new Float32Array([1.0, 1.0, 1.0]);
var lightSpecular = new Float32Array([1.0, 1.0, 1.0]);

var materialAmbient = new Float32Array([0.0, 0.0, 0.0]);
var materialDiffuse = new Float32Array([0.0, 1.0, 0.0]);
var materialSpecular = new Float32Array([1.0, 1.0, 1.0]);
var materialShininess = new Float32Array([1.0]);

var lightPosition = new Float32Array([10.0, 0.0, 0.0, 1.0]);

var sphere = null;
const distanceFromBall = 20.0;
const radius = 20.0;

var modelUniform;
var viewUniform;
var isLKeyPressed;
var laUniform;
var ldUniform;
var lsUniform;
var kaUniform;
var kdUniform;
var ksUniform;
var lightPositionUniform;
var materialShininessUniform;
var bUpdateXPos = false;
var bUpdateYPos = false;
var perspectiveProjectionMatrix;

var materialAmbient;
var materialDiffuse;
var materialSpecular;

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

    switch (event.key) {
        case 'x':
        case 'X':
            bUpdateXPos = true;
            bUpdateYPos = false;
            break;

        case 'y':
        case 'Y':
            bUpdateXPos = false;
            bUpdateYPos = true;
            break;

        case 'z':
        case 'Z':
            bUpdateXPos = false;
            bUpdateYPos = false;
            break;
        default:
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
        "uniform mat4 u_model_matrix;" +
        "uniform mat4 u_view_matrix;" +
        "uniform mat4 u_projection_matrix;" +
        "uniform int u_lKeyIsPress;" +

        "out vec3 phong_ads_light;" +
        "out vec3 tNorm;" +
        "out vec3 light_direction;" +
        "out vec3 view_vector;" +
        "uniform vec4 u_lightPosition;" +

        "void main(void)" +
        "{" +
        "if(u_lKeyIsPress ==1)" +
        "{" +
        "vec4 eye_coordinate = u_view_matrix * u_model_matrix * vPosition;" +
        "tNorm = mat3(((u_view_matrix * u_model_matrix))) * vNormal;" +
        "light_direction = vec3(u_lightPosition  - eye_coordinate );" +
        "view_vector = normalize(vec3(-eye_coordinate));" +
        "}" +
        "gl_Position = u_projection_matrix * u_view_matrix *u_model_matrix *vPosition;" +
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
        "uniform vec3 u_la;" +
        "uniform vec3 u_ka;" +
        "uniform vec3 u_ld;" +
        "uniform vec3 u_kd;" +
        "uniform vec3 u_ls;" +
        "uniform vec3 u_ks;" +
        "in vec3 phong_ads_light;" +
        "in vec3 tNorm;" +
        "in vec3 light_direction;" +
        "in vec3 view_vector;" +
        "out vec4 FragColor;" +
        "uniform int u_lKeyIsPress;" +
        "uniform float u_material_shininess;" +
        "void main(void)" +
        "{" +
        "if(u_lKeyIsPress ==1)" +
        "{" +
        "vec3 normalized_tNorm = normalize(tNorm);" +
        "vec3 normalized_light_direction = normalize(light_direction);" +
        "vec3 normalized_view_vector = normalize(view_vector);" +
        "vec3 reflection_vector = reflect(-normalized_light_direction,normalized_tNorm);" +
        "float tn_dot_ld = max(dot(normalized_light_direction.xyz,normalized_tNorm),0.0f);" +
        "vec3 ambient = u_la * u_ka;" +
        "vec3 diffuse = u_ld  * u_kd * tn_dot_ld;" +
        "vec3 specular = u_ls  * u_ks * pow(max(dot(reflection_vector,normalized_view_vector),0.0f),u_material_shininess);" +
        "vec3 phong_ads_light = ambient + diffuse + specular;" +
        "FragColor = vec4(phong_ads_light, 1.0);" +
        "}" +
        "else" +
        "{" +
        "FragColor = vec4(1.0,1.0,1.0,1.0);" +
        "}" +
        "}"

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

    modelUniform = gl.getUniformLocation(shaderProgramObject, "u_model_matrix");
    viewUniform = gl.getUniformLocation(shaderProgramObject, "u_view_matrix");
    perspectiveMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projection_matrix");
    isLKeyPressed = gl.getUniformLocation(shaderProgramObject, "u_lKeyIsPress");
    laUniform = gl.getUniformLocation(shaderProgramObject, "u_la");
    ldUniform = gl.getUniformLocation(shaderProgramObject, "u_ld");
    lsUniform = gl.getUniformLocation(shaderProgramObject, "u_ls");
    kaUniform = gl.getUniformLocation(shaderProgramObject, "u_ka");
    kdUniform = gl.getUniformLocation(shaderProgramObject, "u_kd");
    ksUniform = gl.getUniformLocation(shaderProgramObject, "u_ks");
    lightPositionUniform = gl.getUniformLocation(shaderProgramObject, "u_lightPosition");
    materialShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_material_shininess");


    sphere = new Mesh();
    makeSphere(sphere, 1.0, 30, 30);

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
    giWidth = canvas.width;
    giHeight = canvas.height;

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

    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();
    var rowDiff = giWidth / 4,
        columnDiff = giHeight / 6;
    var currentSphereNum = 1;
    var lightPosition = [0.0, 0.0, 0.0, 1.0];
    //rectangle
    mat4.identity(modelMatrix);
    mat4.identity(viewMatrix);
    mat4.identity(modelViewProjectionMatrix);
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -4.0]);

    gl.uniformMatrix4fv(modelUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewUniform, false, viewMatrix);
    gl.uniformMatrix4fv(perspectiveMatrixUniform, false, perspectiveProjectionMatrix);
    if (gbEnableLight) {
        if (bUpdateXPos) {
            lightPosition[0] = distanceFromBall;
            lightPosition[1] = Math.cos(degToRad(lightAngle)) * radius;
            lightPosition[2] = Math.sin(degToRad(lightAngle)) * radius;
            lightPosition[3] = 1.0;
        } else if (bUpdateYPos) {
            lightPosition[0] = Math.cos(degToRad(lightAngle)) * radius;
            lightPosition[1] = distanceFromBall;
            lightPosition[2] = Math.sin(degToRad(lightAngle)) * radius;
            lightPosition[3] = 1.0;
        } else {
            lightPosition[0] = Math.cos(degToRad(lightAngle)) * radius;
            lightPosition[1] = Math.sin(degToRad(lightAngle)) * radius;
            lightPosition[2] = distanceFromBall;
            lightPosition[3] = 1.0;
        }
        gl.uniform1i(isLKeyPressed, 1);
        gl.uniform3f(ldUniform, 1.0, 0.0, 1.0);
        gl.uniform3f(kdUniform, 0.5, 0.5, 0.5);
        //light color


        gl.uniform3fv(laUniform, lightAmbient);
        gl.uniform3fv(ldUniform, lightDiffused);
        gl.uniform3fv(lsUniform, lightSpecular);
        gl.uniform4fv(lightPositionUniform, lightPosition);
        gl.uniform1i(isLKeyPressed, 1);

    } else {
        gl.uniform1i(isLKeyPressed, 0);
    }
    for (var i = 0; i < 4; i++) {
        //fprintf_s(gpFile, "Current row : %d  and x: %f \n", i, i*rowDiff);
        for (var j = 0; j < 6; j++) {
            getSphereMaterial(currentSphereNum);
            //fprintf_s(gpFile, "\t Current column : %d  and y: %f \n", j, j*columnDiff);
            drawSphere(i * rowDiff, j * columnDiff, rowDiff, columnDiff);

            currentSphereNum++;
        }

    }
    gl.useProgram(null);
    lightAngle = lightAngle + 4.50;
    if (lightAngle >= 360) {
        lightAngle = 0.0;
    }
    //animation loop
    requestAnimationFrame(draw, canvas);
}

function drawSphere(x, y, width, height) {
    //material color
    gl.uniform3fv(kaUniform, materialAmbient);
    gl.uniform3fv(kdUniform, materialDiffuse);
    gl.uniform3fv(ksUniform, materialSpecular);

    gl.uniform1f(materialShininessUniform, materialShininess[0]);
    gl.viewport(x, y, width, height);
    sphere.draw();
}

function getSphereMaterial(iSphereNum) {
    switch (iSphereNum) {
        case 1:
            //Emerald
            materialAmbient[0] = 0.0215;
            materialAmbient[1] = 0.1745;
            materialAmbient[2] = 0.0215;
            materialAmbient[3] = 1.0;

            materialSpecular[0] = 0.633;
            materialSpecular[1] = 0.727811;
            materialSpecular[2] = 0.633;
            materialSpecular[3] = 1.0;

            materialDiffuse[0] = 0.07568;
            materialDiffuse[1] = 0.61424;
            materialDiffuse[2] = 0.07568;
            materialDiffuse[3] = 1.0;

            materialShininess[0] = 0.6 * 128;
            break;
        case 2:
            //Jade
            materialAmbient[0] = 0.135;
            materialAmbient[1] = 0.225;
            materialAmbient[2] = 0.1575;
            materialAmbient[3] = 1.0;



            materialDiffuse[0] = 0.54;
            materialDiffuse[1] = 0.89;
            materialDiffuse[2] = 0.63;
            materialDiffuse[3] = 1.0;



            materialSpecular[0] = 0.316228;
            materialSpecular[1] = 0.316228;
            materialSpecular[2] = 0.316228;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.1 * 128;

            break;
        case 3:
            //Obsidian
            materialAmbient[0] = 0.05375;
            materialAmbient[1] = 0.05;
            materialAmbient[2] = 0.06625;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.018275;
            materialDiffuse[1] = 0.17;
            materialDiffuse[2] = 0.22525;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.332741;
            materialSpecular[1] = 0.328634;
            materialSpecular[2] = 0.346435;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.3 * 128;;

            break;
        case 4:
            //Pearl
            materialAmbient[0] = 0.25;
            materialAmbient[1] = 0.20725;
            materialAmbient[2] = 0.20725;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 1.0;
            materialDiffuse[1] = 0.829;
            materialDiffuse[2] = 0.829;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.296648;
            materialSpecular[1] = 0.296648;
            materialSpecular[2] = 0.296648;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.088 * 128;
            break;
        case 5:
            //Ruby
            materialAmbient[0] = 0.1745;
            materialAmbient[1] = 0.01175;
            materialAmbient[2] = 0.1175;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.61424;
            materialDiffuse[1] = 0.04136;
            materialDiffuse[2] = 0.04136;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.727811;
            materialSpecular[1] = 0.626959;
            materialSpecular[2] = 0.626959;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.6 * 128;
            break;
        case 6:
            //Turqoise
            materialAmbient[0] = 0.1;
            materialAmbient[1] = 0.18725;
            materialAmbient[2] = 0.1745;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.396;
            materialDiffuse[1] = 0.74151;
            materialDiffuse[2] = 0.69102;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.297254;
            materialSpecular[1] = 0.30829;
            materialSpecular[2] = 0.306678;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.1 * 128;
            break;
        case 7:
            //Brass
            materialAmbient[0] = 0.329412;
            materialAmbient[1] = 0.223529;
            materialAmbient[2] = 0.027451;
            materialAmbient[3] = 1.0;
            materialDiffuse[0] = 0.780392;
            materialDiffuse[1] = 0.568627;
            materialDiffuse[2] = 0.113725;
            materialDiffuse[3] = 1.0;
            materialSpecular[0] = 0.992157;
            materialSpecular[1] = 0.941176;
            materialSpecular[2] = 0.807843;
            materialSpecular[3] = 1.0;
            materialShininess[0] = 0.21794872 * 128;

            break;
        case 8:
            //Bronze
            materialAmbient[0] = 0.2125;
            materialAmbient[1] = 0.1275;
            materialAmbient[2] = 0.054;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.714;
            materialDiffuse[1] = 0.4284;
            materialDiffuse[2] = 0.18144;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.393548;
            materialSpecular[1] = 0.271906;
            materialSpecular[2] = 0.166721;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.2 * 128;
            break;
        case 9:
            //chrome
            materialAmbient[0] = 0.25;
            materialAmbient[1] = 0.25;
            materialAmbient[2] = 0.25;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.4;
            materialDiffuse[1] = 0.4;
            materialDiffuse[2] = 0.4;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.774597;
            materialSpecular[1] = 0.774597;
            materialSpecular[2] = 0.774597;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.6 * 128;
            break;
        case 10:
            //copper
            materialAmbient[0] = 0.19125;
            materialAmbient[1] = 0.0725;
            materialAmbient[2] = 0.0225;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.7038;
            materialDiffuse[1] = 0.27048;
            materialDiffuse[2] = 0.0828;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.25777;
            materialSpecular[1] = 0.137622;
            materialSpecular[2] = 0.086014;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.1 * 128;
            break;
        case 11:
            //gold
            materialAmbient[0] = 0.24725;
            materialAmbient[1] = 0.1995;
            materialAmbient[2] = 0.0745;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.75164;
            materialDiffuse[1] = 0.60648;
            materialDiffuse[2] = 0.22648;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.628281;
            materialSpecular[1] = 0.555802;
            materialSpecular[2] = 0.366065;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.4 * 128;
            break;
        case 12:
            //Silver
            materialAmbient[0] = 0.19225;
            materialAmbient[1] = 0.19225;
            materialAmbient[2] = 0.19225;
            materialAmbient[3] = 1.0;

            materialDiffuse[0] = 0.50754;
            materialDiffuse[1] = 0.50754;
            materialDiffuse[2] = 0.50754;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.508273;
            materialSpecular[1] = 0.508273;
            materialSpecular[2] = 0.508273;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.4 * 128;
            break;

        case 13:
            //black
            materialAmbient[0] = 0.0;
            materialAmbient[1] = 0.0;
            materialAmbient[2] = 0.0;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.01;
            materialDiffuse[1] = 0.01;
            materialDiffuse[2] = 0.01;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.50;
            materialSpecular[1] = 0.50;
            materialSpecular[2] = 0.50;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.25 * 128;
            break;
        case 14:
            //Cyan
            materialAmbient[0] = 0.0;
            materialAmbient[1] = 0.1;
            materialAmbient[2] = 0.06;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.0;
            materialDiffuse[1] = 0.50980392;
            materialDiffuse[2] = 0.50980392;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.50196078;
            materialSpecular[1] = 0.50196078;
            materialSpecular[2] = 0.50196078;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.25 * 128;
            break;
        case 15:
            //green
            materialAmbient[0] = 0.0;
            materialAmbient[1] = 0.0;
            materialAmbient[2] = 0.0;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.1;
            materialDiffuse[1] = 0.35;
            materialDiffuse[2] = 0.1;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.45;
            materialSpecular[1] = 0.55;
            materialSpecular[2] = 0.45;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.25 * 128;
            break;
        case 16:
            //Red
            materialAmbient[0] = 0.0;
            materialAmbient[1] = 0.0;
            materialAmbient[2] = 0.0;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.5;
            materialDiffuse[1] = 0.0;
            materialDiffuse[2] = 0.0;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.7;
            materialSpecular[1] = 0.6;
            materialSpecular[2] = 0.6;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.25 * 128;
            break;
        case 17:
            //White
            materialAmbient[0] = 0.0;
            materialAmbient[1] = 0.0;
            materialAmbient[2] = 0.0;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.55;
            materialDiffuse[1] = 0.55;
            materialDiffuse[2] = 0.55;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.70;
            materialSpecular[1] = 0.70;
            materialSpecular[2] = 0.70;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.6 * 128;
            break;
        case 18:
            //Yellow
            materialAmbient[0] = 0.0;
            materialAmbient[1] = 0.0;
            materialAmbient[2] = 0.0;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.5;
            materialDiffuse[1] = 0.5;
            materialDiffuse[2] = 0.0;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.6;
            materialSpecular[1] = 0.6;
            materialSpecular[2] = 0.5;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.25 * 128;
            break;
        case 19:
            //black
            materialAmbient[0] = 0.02;
            materialAmbient[1] = 0.02;
            materialAmbient[2] = 0.02;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.01;
            materialDiffuse[1] = 0.01;
            materialDiffuse[2] = 0.01;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.4;
            materialSpecular[1] = 0.4;
            materialSpecular[2] = 0.4;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.078125 * 128;
            break;
        case 20:
            //Cyan
            materialAmbient[0] = 0.0;
            materialAmbient[1] = 0.05;
            materialAmbient[2] = 0.05;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.4;
            materialDiffuse[1] = 0.5;
            materialDiffuse[2] = 0.5;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.04;
            materialSpecular[1] = 0.7;
            materialSpecular[2] = 0.7;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.078125 * 128;
            break;
        case 21:
            //Green
            materialAmbient[0] = 0.0;
            materialAmbient[1] = 0.05;
            materialAmbient[2] = 0.0;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.4;
            materialDiffuse[1] = 0.5;
            materialDiffuse[2] = 0.4;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.04;
            materialSpecular[1] = 0.7;
            materialSpecular[2] = 0.04;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.078125 * 128;
            break;
        case 22:
            //Red
            materialAmbient[0] = 0.05;
            materialAmbient[1] = 0.0;
            materialAmbient[2] = 0.0;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.5;
            materialDiffuse[1] = 0.4;
            materialDiffuse[2] = 0.4;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.7;
            materialSpecular[1] = 0.04;
            materialSpecular[2] = 0.04;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.088 * 128;
            break;
        case 23:
            //White
            materialAmbient[0] = 0.05;
            materialAmbient[1] = 0.05;
            materialAmbient[2] = 0.05;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.05;
            materialDiffuse[1] = 0.05;
            materialDiffuse[2] = 0.05;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.7;
            materialSpecular[1] = 0.7;
            materialSpecular[2] = 0.7;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.078125 * 128;
        case 24:
            //Yellow
            materialAmbient[0] = 0.05;
            materialAmbient[1] = 0.05;
            materialAmbient[2] = 0.0;
            materialAmbient[3] = 1.0;


            materialDiffuse[0] = 0.5;
            materialDiffuse[1] = 0.5;
            materialDiffuse[2] = 0.4;
            materialDiffuse[3] = 1.0;


            materialSpecular[0] = 0.7;
            materialSpecular[1] = 0.7;
            materialSpecular[2] = 0.04;
            materialSpecular[3] = 1.0;

            materialShininess[0] = 0.1 * 128;
            break;
        default:
            break;
    }
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