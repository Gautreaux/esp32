

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
        console.log("Joystick-move index: " + joystickIndex);

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
    send("J" + joystickIndex + " " + Math.round(joystickVector[0], 3) + " " + Math.round(joystickVector[1], 3))
}