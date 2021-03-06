
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_MCP23017.h"
#include <vector>
#include <sstream>

#include <WiFi.h>
#include "webpage.h"

//TODO - transition to "ESPAsyncWebServer.h"
//  https://shawnhymel.com/1882/how-to-create-a-web-server-with-websockets-using-an-esp32-in-arduino/
#include <WebServer.h>

#include <WebSocketsServer.h>

#define PWM_FREQUENCY 5000
#define PWM_RESOLUTION 8
#define PWM_MAX_INT ((1 << PWM_RESOLUTION) - 1)

//when enabled will cause the ESP to connect
//  to an existing wifi network given by the information,
//  in the clientSecrets.h file
//when disabled, will cause the ESP to
//  host a new wifi network with the information
//  in the connectionSecrets.h file
// #define RUN_AS_CLIENT

#ifdef RUN_AS_CLIENT
#include "clientSecrets.h"
#else
#include "connectionSecrets.h"
#endif


//21, 22 removed because SDA/SCL
// const std::vector<int> gpioPins{1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 23, 25, 26, 27, 32, 33, 34, 35, 36, 39};

//Tracks if button was pressed in the last loop (true)
// bool wasButtonPressedLastLoop = false;

#define WEBSERVER_PORT 80
#define WEBSOCKET_PORT 81

#define DIRECTION_FORWARD 1
#define DIRECTION_NONE 0
#define DIRECTION_BACKWARDS -1

WebServer server(WEBSERVER_PORT); 
WebSocketsServer wss(WEBSOCKET_PORT);

struct MotorAbstract{
    uint8_t EN_Pin; //ON ESP32
    uint8_t A1_Pin; //ON MCP23017
    uint8_t A2_Pin; //ON MCP23017
};

//Stores all the motor information
// Array of MotorAbstracts initalized to proper values
const MotorAbstract MotorStructs[] = {
    {13, 0, 1},
    {12, 2, 3},
    {14, 4, 5},
    {5, 6, 7},
    {26, 8, 9},
    {25, 10, 11}
};

//maintains a list of the last directions the motor was moving
uint8_t MotorDirections[] = {
    DIRECTION_NONE,
    DIRECTION_NONE,
    DIRECTION_NONE,
    DIRECTION_NONE,
    DIRECTION_NONE,
    DIRECTION_NONE
};

// struct ServoAbstract{
//     uint8_t Signal_Pin;
// };

const uint8_t ServoStructs[] = {32, 33};

//For serving the ip and port information to the client
#define HOSTPORT_SIZE 64
char HostPort_js[64];

#define NUM_MOTORS (6)
#define NUM_SERVOS (2)

#define DEADZONE (0.05f)

Adafruit_MCP23017 mcp;

void setup()
{
    Serial.begin(9600); // Initialize the Serial interface with baud rate of 9600
    Serial.println("Beginning Initialization.");

    Serial.println("Enabling MCP23017/GPIO pins");
    mcp.begin();
    for (int i = 0; i < NUM_MOTORS; i++)
    {
        mcp.pinMode(MotorStructs[i].A1_Pin, OUTPUT);
        mcp.pinMode(MotorStructs[i].A2_Pin, OUTPUT);
        pinMode(MotorStructs[i].EN_Pin, OUTPUT);
    }

#ifdef RUN_AS_CLIENT
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Waiting for WIFI connection...");
    //count # of retries
    int retryCounter = 0;
    bool toggle = false;
    while(WiFi.status() != WL_CONNECTED){
        delay(1000);
        Serial.print(++retryCounter);
        Serial.print(" ");

        //light show for while retrying
        // if((toggle = !toggle)){
        //     digitalWrite(RED_1, HIGH);
        //     digitalWrite(RED_2, LOW);
        // }else{
        //     digitalWrite(RED_1, LOW);
        //     digitalWrite(RED_2, HIGH);
        // }
    }
    Serial.println("Connected");

    //turn off the light show
    // digitalWrite(RED_1, LOW);
    // digitalWrite(RED_2, LOW);
#else
    WiFi.mode(WIFI_AP);
    WiFi.softAP(SSID, PASSWORD);
#endif
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

   
    Serial.println("Starting Webserver...");

    server.begin();

    //start registering callbacks
    server.on("/", handle_root);
    server.on("/hostPort.js", handle_hostPort);
    server.onNotFound(handle_notFound);

    Serial.println("Starting Websockets Server...");
    wss.begin();
    wss.onEvent(handle_webSocketEvent);


    // pack host and port info into the buffer
 #ifdef RUN_AS_CLIENT
    IPAddress ip = WiFi.localIP();
 #else
    IPAddress ip = WiFi.softAPIP();
 #endif
    snprintf(HostPort_js, HOSTPORT_SIZE, "myHost = \"%d.%d.%d.%d\";\nmyPort = \"%d\";\n",
        ip[0], ip[1], ip[2], ip[3], WEBSOCKET_PORT
    );
    Serial.print("Resolved host port to: <<<\n");
    Serial.print(HostPort_js);
    Serial.println(">>>");

    Serial.println("Starting PWM signal channels...");
    for(int i = 0; i < NUM_MOTORS; i++){
        ledcSetup(i, PWM_FREQUENCY, PWM_RESOLUTION);
        ledcWrite(i, 0);
        ledcAttachPin(MotorStructs[i].EN_Pin, i);
    }
    for(int i = 0; i < NUM_SERVOS; i++){
        ledcSetup(i+NUM_MOTORS, 50, PWM_RESOLUTION);
        ledcWrite(i+NUM_MOTORS, 0);
        ledcAttachPin(ServoStructs[i], i+NUM_MOTORS);
    }

    Serial.println("Initialization Completed.");
}

void loop()
{

    server.handleClient();
    wss.loop();

    // if(digitalRead(BUTTON_PIN) == HIGH){
    //     if(wasButtonPressedLastLoop == false){
    //         wasButtonPressedLastLoop = true;
    //         // Serial.println("PRESSED");
    //         wss.broadcastTXT("b1");
    //     }
    // }else{
    //     if(wasButtonPressedLastLoop == true){
    //         wasButtonPressedLastLoop = false;
    //         // Serial.println("RELEASED");
    //         wss.broadcastTXT("b0");
    //     }
    // }
}

//called on requests for the root webpage
void handle_root(){
    Serial.println("root request");
    server.send(200, "text/html", webpage_html);
}

//called on requests for hostPort.js
void handle_hostPort(){
    server.send(200, "text/html", HostPort_js);
}

//called on page not found (404)
void handle_notFound(){
    Serial.println("Serving 404 Error");
    server.send(404, "text/html",
"<body><h1>404 Error</h1></body>"
    );

}

//called on any websockets connection
//  num     - connection id of the client connection
//  type    - the type of the connection
//  payload - pointer to message buffer
//              - payload is null terminated
//  length  - the length of the payload (in bytes)
void handle_webSocketEvent(uint8_t num, WStype_t type,
                           uint8_t *payload, size_t length)
{
    IPAddress ip = wss.remoteIP(num);
    switch (type)
    {
    case WStype_DISCONNECTED:
        // a client has
        //  timed-out or
        //  indicated that the connection is closing
        Serial.printf("[wss%u] Disconnected.\n", num);
        break;
    case WStype_CONNECTED:
        //a client has connected
        Serial.printf("[wss%u] Connected from %d.%d.%d.%d url: %s\n",
                    num, ip[0], ip[1], ip[2], ip[3], payload);
        break;
    case WStype_TEXT:
        if(!messageHandler(payload, length)){
            Serial.printf("[wss%u] Unknown Message (%d): %s\n", 
                num, length, payload);
        }
        // Serial.printf("[wss%u] MSG Received\n", num);
        break;
    //dont really want this cause there are a-lot of other
    //  message types that we never want to see
    //  such as ping/pong messages
    // default:
    //     break;
    }
}

int8_t getDirectionFromValue(const double value){
    if(value > DEADZONE){
        return DIRECTION_FORWARD;
    }else if(value < -DEADZONE){
        return DIRECTION_BACKWARDS;
    }else{
        return DIRECTION_NONE;
    }
}

void driveDCMotor(const uint8_t motorID, const double value)
{
    auto direction = getDirectionFromValue(value);
    auto magnitude = abs(value);

    //update l293d direction pins only when it changes
    if (MotorDirections[motorID] != direction)
    {
        //switch direction
        if (direction == DIRECTION_FORWARD)
        {
            mcp.digitalWrite(MotorStructs[motorID].A1_Pin, HIGH);
            mcp.digitalWrite(MotorStructs[motorID].A2_Pin, LOW);
            MotorDirections[motorID] = DIRECTION_FORWARD;
        }
        else if (direction == DIRECTION_BACKWARDS)
        {
            mcp.digitalWrite(MotorStructs[motorID].A1_Pin, LOW);
            mcp.digitalWrite(MotorStructs[motorID].A2_Pin, HIGH);
            MotorDirections[motorID] = DIRECTION_BACKWARDS;
        }
        else
        {
            mcp.digitalWrite(MotorStructs[motorID].A1_Pin, LOW);
            mcp.digitalWrite(MotorStructs[motorID].A2_Pin, LOW);
            MotorDirections[motorID] = DIRECTION_NONE;
        }
    }

    ledcWrite(motorID, magnitude * PWM_MAX_INT);
}


//called to process text messages
//  returns true if message was processed ok
//  else returns false
bool messageHandler(uint8_t* const payload, const size_t length){
    std::stringstream ss;
    switch (payload[0])
    {
    case 'J': {
        ss << payload;
        char j;
        ss >> j;
        int jsID;
        ss >> jsID;
        float x,y;
        ss >> x >> y;

        driveDCMotor(jsID, y);
        driveDCMotor(jsID+2, y);
        driveDCMotor(jsID+4, x);

        return true;
    }
    case 'L': {
        Serial.println("L command depreciated with deprecation of dev board.");
        return false;
    }
    case 'M': {
        char motorID = payload[1] - '0';
        double value = atof((const char*)(payload+2));
        driveDCMotor(motorID, value);
    }
    case 'S': {
        ss << payload;
        char s; 
        ss >> s;
        int sliderID;
        ss >> sliderID;
        float sliderPosition;
        ss >> sliderPosition;
        //Serial.printf("Slider: %d %.3f\n", sliderID, sliderPosition);
        ledcWrite(sliderID + NUM_MOTORS, (sliderPosition * PWM_MAX_INT));
        return true;
    }
    default: {
        return false;
    }
    }
    return false;
}
