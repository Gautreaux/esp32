

Joystick_Wrappers = null;
Joystick_Colliders = null;
Joystick_TouchIdentifiers = [null, null];
Joystick_TouchStarts = [null, null];
Joystick_TouchMoveFunctions = [null, null];
Joystick_ResetFunctions = [null, null];

function initJoysticks(){
    Joystick_Wrappers = document.getElementsByClassName("joystickWrapper");

    if(Joystick_Wrappers.length != 2){
        console.error("Error in joystick wrappers: expected len 2 got len " + Joystick_Wrappers.length);
    }

    Joystick_Colliders = document.getElementsByClassName("joystickCollider");

    if(Joystick_Colliders.length != 2){
        console.error("Error in joystick colliders: expected len 2 got len " + Joystick_Colliders.length);
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
            Joystick_TouchMoveFunctions[joystickIndex] = pos;

            console.log("New touch on joystick " + joystickIndex + ": " + pos);

            //TODO - code for the movement size and things
            //setting max movement ssize
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

        //will be empty on the thingy
        console.log(event.targetTouches);

        var thisTouch = null;

        for(i = 0; i < event.changedTouches.length; i++)
        {
            if(event.changedTouches[i].identifier = Joystick_TouchIdentifiers[joystickIndex])
            {
                thisTouch = event.changedTouches[i];
                break;
            }
        }
        
        if(thisTouch === null){
            console.error("On release of joystick " + joystickIndex + " could not find relevant touch start");
            Joystick_ResetFunctions[joystickIndex]();
            return;
        }

        //TODO - log info and things;

        Joystick_ResetFunctions[joystickIndex]();
    }
}

function joystickTouchMoveFactory(joystickIndex){
    return function(event){
        event.preventDefault();
        console.log("Joystick-move index: " + joystickIndex);
    }
}

function joystickResetFactory(joystickIndex){
    return function(){
        console.log("Joystick-reset index: " + joystickIndex);
    }
}