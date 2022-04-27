//global variable
var canvas = null;
var context = null;

//onload function
function main() {
    //get <canvas> element
    canvas = document.getElementById("AMC");
    if (!canvas)
        console.log("obtaining canvas failed\n");
    else
        console.log("obtaining canvas succeeded\n");

    //print canvas width and height on console
    console.log("Canvas Width : " + canvas.width + " And Canvas Height : " + canvas.height);

    // get 2D context
    context = canvas.getContext("2d");
    if (!context)
        console.log("Obtaining 2D Context Failed\n");
    else
        console.log("Obtaining 2D Context Succeeded\n");

    // fill canvas with black ccolor
    context.fillStyle = "black"; // ”#000000”
    context.fillRect(0, 0, canvas.width, canvas.height);

    //draw text
    drawText("Hello World !!!");

    //register keyboards keydown event handler
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mouseDown, false);
}

function keyDown(event) {

    //code
    switch (event.keyCode) {
        case 70: //for full screen
            togglefullScreen();

            //repaint
            //drawText("Hello World!!!");
            break;

    }
    //   alert("A key is pressed");
}

function mouseDown() {
    //code
    // alert("Mouse is clicked");
}

function drawText(text) {
    //code
    //center the text
    context.textAlign = "center"; //center horizontally
    context.textBaseline = "middle"; //center vertically

    //text font
    context.font = "48px sans-serif";

    //text color
    context.fillStyle = "white";

    // display the text in center 
    context.fillText(text, canvas.width / 2, canvas.height / 2);
}

function togglefullScreen() {
    //code
    var fullscreen_element = document.fullscreenElement || document.webkitFullscreenElement || document.mozFullScreenElement || document.msFullscreenElement || null;

    //if not fullscreen
    if (fullscreen_element == null) {
        if (canvas.requestFullscreen)
            canvas.requestFullscreen();
        else if (canvas.mozRequestFullScreen)
            canvas.mozRequestFullScreen();
        else if (canvas.webkitFullscreenElement)
            canvas.webkitFullscreenElement();
        else if (canvas.msRequestFullscreen)
            canvas.msRequestFullscreen();
    } else //if already fullscreen
    {
        if (document.exitFullscreen)
            document.exitFullscreen();
        else if (document.mozCancelFullScreen)
            document.mozCancelFullScreen();
        else if (document.webkitExitFullscreen)
            document.webkitExitFullscreen();
        else if (document.msExitFullscreen)
            document.msExitFullscreen();
    }
}