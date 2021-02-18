
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
#define RUN_AS_CLIENT

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
}

//Stores all the motor information
const MotorStructs[] = {
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

//need all the zeros to reserve the maximum conceivable amount of space
#define HOSTPORT_SIZE 64
char HostPort_js[64];

#define NUM_MOTORS (6)

#define DEADZONE (0.05f)

Adafruit_MCP23017 mcp;

//scan the i2c bus and print all the addresses found
void scanI2C()
{
    byte count = 0; // total i2c devices found

    for (byte i = 8; i < 120; i++)
    {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0)
        {
            Serial.print("Found I2C Device: ");
            Serial.print(" (0x");
            Serial.print(i, HEX);
            Serial.println(")");
            count++;
            delay(1); //ms
        }
    }
    Serial.print("I2C scan complete. ");
    if (count > 0)
    {
        Serial.print("Found ");
        Serial.print(count, HEX);
        Serial.println(" Device(s).");
    }
    else
    {
        Serial.println("Found no Devices");
    }
}

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
    IPAddress ip = WiFi.localIP();
    snprintf(HostPort_js, HOSTPORT_SIZE, "myHost = \"%d.%d.%d.%d\";\nmyPort = \"%d\";\n",
        ip[0], ip[1], ip[2], ip[3], WEBSOCKET_PORT
    );
    Serial.print("Resolved host port to: ");
    Serial.println(HostPort_js);

    Serial.println("Starting PWM signal channels...");
    for(int i = 0; i < NUM_MOTORS; i++){
        ledcSetup(i, PWM_RESOLUTION, PWM_RESOLUTION);
        ledcWrite(i, 0);
        ledcAttachPin(MotorStructs[i].EN_Pin, i);
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

//called to process text messages
//  returns true if message was processed ok
//  else returns false
bool messageHandler(uint8_t* const payload, const size_t length){
    std::stringstream ss;
    switch (payload[0])
    {
    case 'L':
        Serial.println("L command depreciated with deprecation of dev board.");
        return false;
    case 'J':
        ss << payload;
        char j;
        ss >> j;
        int jsID;
        ss >> jsID;
        float x,y;
        ss >> x >> y;
        // Serial.printf("%s\n", payload);
        // Serial.printf("J: %d %.3f %.3f\n", jsID, x, y);
        // Serial.printf("V: %dg %dr\n", 
        //   uint32_t(PWM_MAX_INT*((y > .05) ? y : 0)),
        //   uint32_t(PWM_MAX_INT*((y < -.05) ? -y : 0))
        // );
        // Serial.printf("%d\n", jsID);

        for(int i = jsID; i < NUM_MOTORS; i++){
            float abs_y = ((y < 0) ? -y : y);
            if(y > DEADZONE && MotorDirections[i] != DIRECTION_FORWARD){
                mcp.digitalWrite(MotorStructs[i].A1_Pin, HIGH);
                mcp.digitalWrite(MotorStructs[i].A2_Pin, LOW);
                MotorDirections[i] = DIRECTION_FORWARD;
            }else if(y < -DEADZONE && MotorDirections[i] != DIRECTION_BACKWARDS){
                mcp.digitalWrite(MotorStructs[i].A1_Pin, LOW);
                mcp.digitalWrite(MotorStructs[i].A2_Pin, HIGH);
                MotorDirections[i] = DIRECTION_BACKWARDS;
            }else if(MotorDirections[i] != DIRECTION_NONE){
                mcp.digitalWrite(MotorStructs[i].A1_Pin, LOW);
                mcp.digitalWrite(MotorStructs[i].A2_Pin, LOW);
                MotorDirections[i] = DIRECTION_NONE;
            }
            uint8_t pwmVal = uint8_t(abs_y * PWM_MAX_INT);
            ledcWrite(i, pwmVal);
        }

        // ledcWrite(((jsID == 0) ? LED1_GREEN_PWM_CHANNEL : LED2_GREEN_PWM_CHANNEL),
        //     uint32_t(PWM_MAX_INT*((y > .05) ? y : 0)));
        // ledcWrite(((jsID == 0) ? LED1_RED_PWM_CHANNEL : LED2_RED_PWM_CHANNEL),
        //     uint32_t(PWM_MAX_INT*((y < -.05) ? -y : 0)));

        return true;
    default:
        return false;
    }
    return false;
}
