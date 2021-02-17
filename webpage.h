const char webpage_html[] PROGMEM = R"&(
<head>
<script>
mySocket = null; //global variable for storing the socket 

//TODO - fix this for the future
myHost = "192.168.4.1";
myPort = "81";

//called once on page load
function initFunction() {
    if(typeof myHost == 'undefined') {
        console.log("No host provided, defaulting to 192.168.0.199")
        myHost = '192.168.0.199'
    }

    if(typeof myPort == 'undefined') {
        console.log("No port provided, defaulting to 20462")
        myPort = '20462'
    }

    let s = "ws://" + myHost + ":" + myPort;

    connect(s);    
    
    element = document.getElementById("hostField");
    element.innerHTML = "HOST: " + myHost;

    element = document.getElementById("portField");
    element.innerHTML = "PORT: " + myPort;


    // window.addEventListener('resize', rescaleHandler);
    // rescaleHandler(); //run the first time

    initJoysticks();
}

//connect to a websockets server at path
function connect(path) {
    if(mySocket != null){
        mySocket.close();
        mySocket = null;
    }
    
    console.log("Client started. Tying connection on: " + path)

    setStatus("Socket Connecting", "yellow")

    //create the socket object
    const socket = new WebSocket(path)

    //add the callbacks as necessary
    socket.onopen = onSocketOpen; //called when connection established
    socket.onerror = onSocketError; //called when error
    socket.onmessage = onSocketReceive; //called each message received
    socket.onclose = onSocketClose; //called each close
}

//called when the socket establishes a connection to the server
function onSocketOpen(event) {
    setStatus("Socket Opened", "lime")
    console.log("Socket Opened");

    //now that the socket is properly opened, capture it
    mySocket = this; //this refers to the socket object
}

//called on a socket error
//   most commonly, this is due to connection timeout
//most errors also cause the socket to close, 
//   calling onclose callback after the onerror callback
function onSocketError(event) {
    setStatus("Socket Error", "#f53636"); //set background to a soft red color
    console.log("Socket Error");
}

//called when the socket is closed
function onSocketClose(event) {
    setStatus("Socket Closed", "#f53636"); //set background to a soft red color
    console.log("Socket Closed");

    //now that the socket is invalid, remove it
    mySocket = null;
}

//called when the socket receives data
function onSocketReceive(event) {
    let message = event.data;
    if(!processCommand(message)){
        console.log("Unknown Command Received:'" + message + "'");
    }
}

//sends the input parameter into the socket
function send(data) {
    console.log(data);
    if (mySocket == null) {
        console.log("Cannot send data, socket is not connected: " + data);
    }
    else {
        mySocket.send(data);
    }
}

//set the status bar to contain the message and color specified
function setStatus(message, color){
    console.log("Socket Status: " + message)
    element = document.getElementById("body");
    element.style.backgroundColor = color;
}

function sendTestMessage(message){
    let v = document.getElementById("testMessageInput").value;
    console.log("Sending Test Message: " + v);
    send(v)
}

//connect to user specified server
function manualConnection(){
    let path = document.getElementById("manualConnection").value;

    if(path.length == 0 || path.indexOf(':') == -1){
        //TODO - make more apparent on the screen?
        console.log("Invalid path construction.");
        return;
    }

    let i = path.indexOf("ws://");
    if(i == -1){
        path = "ws://" + path;
    }
    else if(i != 0){
        //TODO - make more apparent on the screen?
        console.log("Invalid path, protocol cannot appear mid path.");
        return;
    }

    //TODO - check for and add port if necessary

    console.log("Manual connection path ok with: " + path);

    connect(path);
}

//returns true on successful command, else false
function processCommand(cmd){
    if(cmd[0] === 'b'){
        setButtonReadout(cmd[1] == '1');

        return true;
    }


    return false;
}
</script>
<script>
function updateLED() {
    let led_elements = document.getElementsByClassName("LEDCHECK");

    var s = "L";

    for(i = 0; i < led_elements.length; i++){
        if(led_elements[i].checked){
            s += "1";
        }else{
            s += "0";
        }
    }
    // console.log(s);
    send(s);
}

function setButtonReadout(b){
    let e = document.getElementById("ButtonReadout");
    if(b){
        e.innerHTML = "Pressed";
    }else{
        e.innerHTML = "Released";
    }
}

//connect to user specified server
function manualConnection(){
    let path = document.getElementById("manualConnection").value;

    if(path.length == 0 || path.indexOf(':') == -1){
        //TODO - make more apparent on the screen?
        console.log("Invalid path construction.");
        return;
    }

    let i = path.indexOf("ws://");
    if(i == -1){
        path = "ws://" + path;
    }
    else if(i != 0){
        //TODO - make more apparent on the screen?
        console.log("Invalid path, protocol cannot appear mid path.");
        return;
    }

    console.log("Manual connection path ok with: " + path);

    connect(path);
}
</script>
<script>


Joystick_Wrappers = null;
Joystick_Colliders = null;
Joystick_Joysticks = null;
Joystick_TouchIdentifiers = [null, null];
Joystick_TouchStarts = [null, null];
Joystick_TouchMoveFunctions = [null, null];
Joystick_ResetFunctions = [null, null];
Joystick_MaxMovement = null;

function initJoysticks(){
    let EXPECTED_LEN = 2;
    Joystick_Wrappers = document.getElementsByClassName("joystickWrapper");

    if(Joystick_Wrappers.length != EXPECTED_LEN){
        console.error("Error in joystick wrappers: expected len " + EXPECTED_LEN + " got len " + Joystick_Wrappers.length);
    }

    Joystick_Colliders = document.getElementsByClassName("joystickCollider");

    if(Joystick_Colliders.length != EXPECTED_LEN){
        console.error("Error in joystick colliders: expected len " + EXPECTED_LEN + " got len " + Joystick_Colliders.length);
    }

    Joystick_Joysticks = document.getElementsByClassName("joystick");

    if(Joystick_Joysticks.length != EXPECTED_LEN){
        console.error("Error in joystick objects: expected len " + EXPECTED_LEN + " got len " + Joystick_Joysticks.length);
    }

    if(Joystick_Wrappers.length != Joystick_TouchIdentifiers.length){
        console.err("Error: joystick_wrappers len (" + Joystick_Wrappers.length + ") does not match expected len in identifiers (" + Joystick_TouchIdentifiers.length + ")")
    }

    if(Joystick_Wrappers.length != Joystick_TouchStarts.length){
        console.err("Error: joystick_wrappers len (" + Joystick_Wrappers.length + ") does not match expected len in touch starts (" + Joystick_TouchStarts.length + ")")
    }

    if(Joystick_Wrappers.length != Joystick_TouchMoveFunctions.length){
        console.err("Error: joystick_wrappers len (" + Joystick_Wrappers.length + ") does not match expected len in touch move functions (" + Joystick_TouchMoveFunctions.length + ")")
    }

    if(Joystick_Wrappers.length != Joystick_ResetFunctions.length){
        console.err("Error: joystick_wrappers len (" + Joystick_Wrappers.length + ") does not match expected len in touch move functions (" + Joystick_ResetFunctions.length + ")")
    }

    for(let i = 0; i < Joystick_Wrappers.length; i++){
        Joystick_Colliders[i]
            .addEventListener('touchstart',
            joystickTouchStartFactory(i),
            // {passive: true}
            );
        Joystick_Colliders[i]
            .addEventListener('touchend',
            joystickTouchEndFactory(i),
            );
        Joystick_TouchMoveFunctions[i] = joystickTouchMoveFactory(i);
        Joystick_ResetFunctions[i] = joystickResetFactory(i);
    }

    console.log("Joysticks initialized");
}

function joystickTouchStartFactory(joystickIndex){
    return function(event){
        event.preventDefault();
        console.log("Joystick-start index: " + joystickIndex);

        if(event.targetTouches.length != 0 && Joystick_TouchIdentifiers[joystickIndex] == null){
            let thisTouch = event.targetTouches[0];

            Joystick_TouchIdentifiers[joystickIndex] = thisTouch.identifier;

            let pos = [thisTouch.clientX, thisTouch.clientY];
            Joystick_TouchStarts[joystickIndex] = pos;

            console.log("New touch on joystick " + joystickIndex + ": " + pos);

            Joystick_Colliders[joystickIndex].addEventListener("touchmove", Joystick_TouchMoveFunctions[joystickIndex]);

            //TODO - code for the movement size and things
            //setting max movement size

            let p = Joystick_Colliders[joystickIndex].parentElement.parentElement.clientHeight;
            let j = Joystick_Colliders[joystickIndex].clientHeight;
            Joystick_MaxMovement = (.5*(p-j));
        }
    }
}

function joystickTouchEndFactory(joystickIndex){
    return function(event){
        event.preventDefault();
        console.log("Joystick-end index: " + joystickIndex);

        if(Joystick_TouchIdentifiers[joystickIndex] == null){
            //there is no relevant touch for this joystick
            return;
        }

        Joystick_ResetFunctions[joystickIndex]();
    }
}

function joystickTouchMoveFactory(joystickIndex){
    return function(event){
        event.preventDefault();
        // console.log("Joystick-move index: " + joystickIndex);

        var thisTouch = null;

        for(i = 0; i < event.targetTouches.length; i++){
            if(event.targetTouches[i].identifier == Joystick_TouchIdentifiers[joystickIndex]){
                thisTouch = event.targetTouches[i];
                break;
            }
        }

        if(thisTouch === null)
        {
            //should be unreachable
            console.error("Lost this touch from touchpoint set.")
            Joystick_ResetFunctions[joystickIndex]();
            return;
        }

        var thisPos = [thisTouch.clientX, thisTouch.clientY];
        // console.log(thisPos);

        let thisVector = [
            ((thisPos[0]-Joystick_TouchStarts[joystickIndex][0])/Joystick_MaxMovement),
            ((thisPos[1]-Joystick_TouchStarts[joystickIndex][1])/Joystick_MaxMovement),
        ];

        let vectorMagnitude = Math.sqrt(Math.pow(thisVector[0], 2) + Math.pow(thisVector[1], 2));
        if(vectorMagnitude > 1){
            thisVector[0] /= vectorMagnitude;
            thisVector[1] /= vectorMagnitude;
        }

        // console.log(thisVector);

        let j = Joystick_Joysticks[joystickIndex];
        j.style.left = String(thisVector[0]*Joystick_MaxMovement);
        j.style.top = String(thisVector[1]*Joystick_MaxMovement);

        sendJoystickVector(joystickIndex, thisVector);
    }
}

function joystickResetFactory(joystickIndex){
    return function(){
        console.log("Joystick-reset index: " + joystickIndex);

        Joystick_TouchIdentifiers[joystickIndex] = null;
        Joystick_TouchStarts[joystickIndex] = null;
        
        Joystick_Colliders[joystickIndex].removeEventListener("touchmove", Joystick_TouchMoveFunctions[joystickIndex]);

        Joystick_Joysticks[joystickIndex].style.left = "0";
        Joystick_Joysticks[joystickIndex].style.top = "0";

        sendJoystickVector(joystickIndex, [0,0])
    }
}

function sendJoystickVector(joystickIndex, joystickVector){
    send("J" + joystickIndex + " " + joystickVector[0].toFixed(3) + " " + joystickVector[1].toFixed(3));
}
</script>
<style>

#contentBound{
    display: flex;
    flex-direction: row;
    flex-wrap: wrap;
}

.flexContent{
    border: 2px solid black;
    background-color: aqua;  
    margin: 5px;
}
</style>
<style>
#controlsWrapper{
    width: 100%;
    height: 100vh;
}

.controlStack{
    height: 100%;
    width: 30%;
    background-color: maroon;
    display: inline-block;
}
</style>
<style>
:root{
    --joystickCollider-size: 30%;
}

.joystickWrapper{
    position: relative;
    background-color: grey;
    width: 100%;
}

.joystickWrapper:after{
    content: "";
    display: block;
    padding-bottom: 100%;
}

.joystickSubWrapper{
    position: absolute;
    width: 100%;
    height: 100%;
}

.inscribedCircle{
    --border-width: 4px;
    height: calc(100% - 2 * var(--border-width));
    width: calc(100% - 2 * var(--border-width));
    border: var(--border-width) solid black;
    border-radius: 50%;
}

.limitCircle{
    --border-width: 2px;
    height: calc(100% - var(--joystickCollider-size));
    width: calc(100% - var(--joystickCollider-size));
    border: var(--border-width) solid darkgray;
    border-radius: 50%;

    margin: 0;
    position: absolute;
    top: 50%;
    left: 50%;
    -ms-transform: translate(-50%, -50%);
    transform: translate(-50%, -50%);

}

.joystickCollider{
    background-color: rgba(255, 0, 0, .2);
    height: var(--joystickCollider-size);
    width: var(--joystickCollider-size);
    border-radius: 50%;


    margin: 0;
    position: absolute;
    top: 50%;
    left: 50%;
    -ms-transform: translate(-50%, -50%);
    transform: translate(-50%, -50%);
}

.joystick{
    background-color: red;
    height: 100%;
    width: 100%;
    border-radius: 50%;
    position: relative;
    left:0px;
    top:0px;
}
</style>
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
</head>

<body onload="initFunction()" onunload="cleanupFunction()" id="body">
Turns green on websockets connection, red on failure
<div id="contentBound">
<div class="flexContent">
<H4>Manual Connection</H4>
<button onclick="manualConnection()">Connect</button>
<input type="path", id="manualConnection">
<div id="hostField">HOST</div>
<div id="portField">PORT</div>
</div>
<div class="flexContent">
<H4>Test Message</H4>
<input type="text" id="testMessageInput">
<button onclick="sendTestMessage()">Send</button>
</div>
<div class="flexContent">
<H4>LED1</H4>
<input type="checkbox", onclick="updateLED()", class="LEDCHECK", id="LED1_RED">RED<br>
<input type="checkbox", onclick="updateLED()", class="LEDCHECK", id="LED1_GREEN">GREEN<br>
<input type="checkbox", onclick="updateLED()", class="LEDCHECK", id="LED1_BLUE">BLUE<br>
</div>
<div class="flexContent">
<H4>LED2</H4>
<input type="checkbox", onclick="updateLED()", class="LEDCHECK", id="LED2_RED">RED<br>
<input type="checkbox", onclick="updateLED()", class="LEDCHECK", id="LED2_GREEN">GREEN<br>
<input type="checkbox", onclick="updateLED()", class="LEDCHECK", id="LED2_BLUE">BLUE<br>
</div>
<div class="flexContent">
<H4>Button State</H4>
<p id="ButtonReadout">UKN</p>
</div>
<div class="flexContent", id="controlsWrapper">
<div class="controlStack">
<div class="joystickWrapper">
<div class="joystickSubWrapper">
<div class="inscribedCircle"><div class="limitCircle"></div></div>
<div class="joystickCollider"><!--For tracking input, but not moving while redrawing-->
<div class="joystick"></div>
</div>
</div>
</div>
</div>
<div class="controlStack">
<div class="joystickWrapper">
<div class="joystickSubWrapper">
<div class="inscribedCircle"><div class="limitCircle"></div></div>
<div class="joystickCollider"><!--For tracking input, but not moving while redrawing-->
<div class="joystick"></div>
</div>
</div>
</div>
</div>
</div>
<!-- <div class="flexContent"></div> -->
</div>
</body>
)&";
