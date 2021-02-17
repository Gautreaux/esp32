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
    //console.log(data);
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