
function sliderHandler(value, sliderID){
    // console.log(String(value) + " " + String(sliderID));
    //No need to round because the input limits us anyway
    send("S " + String(sliderID) + " " + String(value));
}