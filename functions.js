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