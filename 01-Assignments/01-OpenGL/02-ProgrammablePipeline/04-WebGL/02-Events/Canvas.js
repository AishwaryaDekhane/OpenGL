//onload function
function main() {
    //get <canvas> element
    var canvas = document.getElementById("AMC");
    if (!canvas)
        console.log("obtaining canvas failed\n");
    else
        console.log("obtaining canvas succeeded\n");

    //print canvas width and height on console
    console.log("Canvas Width : " + canvas.width + " And Canvas Height : " + canvas.height);

    // get 2D context
    var context = canvas.getContext("2d");
    if (!context)
        console.log("Obtaining 2D Context Failed\n");
    else
        console.log("Obtaining 2D Context Succeeded\n");

    // fill canvas with black ccolor
    context.fillStyle = "black"; // ”#000000”
    context.fillRect(0, 0, canvas.width, canvas.height);

    // center the text
    context.textAlign = "center"; // center horizontally
    context.textBaseline = "middle"; // center vertically 

    // text
    var str = "Hello World !!!";

    // text font
    context.font = "48px sans-serif";

    // text color 
    context.fillStyle = "white"; // ”#FFFFFF”

    // display the text in center 
    context.fillText(str, canvas.width / 2, canvas.height / 2);

    //register keyboards keydown event handler
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mouseDown, false);
}

function keyDown(event) {
    //code
    alert("A key is pressed");
}

function mouseDown() {
    //code
    alert("Mouse is clicked");
}