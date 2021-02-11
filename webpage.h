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
    if (mySocket == null) {
        console.log("Cannot send data, socket is not connected.")
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
<!-- <div class="flexContent"></div> -->
</div>
</body>
)&";
