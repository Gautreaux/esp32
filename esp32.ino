
#include <Arduino.h>
#include <Wire.h>
// #include "Adafruit_MCP23017.h"
#include <vector>
#include <sstream>

#include <WiFi.h>
#include "webpage.h"

//TODO - transition to "ESPAsyncWebServer.h"
//  https://shawnhymel.com/1882/how-to-create-a-web-server-with-websockets-using-an-esp32-in-arduino/
#include <WebServer.h>

#include <WebSocketsServer.h>

#define MAX_PINS 40

#define GREEN_2 13
#define BLUE_2 14
#define RED_1 15
#define RED_2 16
#define GREEN_1 17
#define BLUE_1 18

#define BUTTON_PIN 23

#define LED1_RED_PWM_CHANNEL 0
#define LED1_GREEN_PWM_CHANNEL 1
#define LED2_RED_PWM_CHANNEL 2
#define LED2_GREEN_PWM_CHANNEL 3

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
const std::vector<int> gpioPins{RED_1, GREEN_1, BLUE_1, RED_2, GREEN_2, BLUE_2};

//Tracks if button was pressed in the last loop (true)
bool wasButtonPressedLastLoop = false;

WebServer server(80); 
WebSocketsServer wss(81);

// Adafruit_MCP23017 mcp;

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

    Serial.print("GPIO Checksum: ");
    Serial.println(gpioPins.size());

    for (auto i : gpioPins)
    {
        Serial.print("Enabling: ");
        Serial.println(i);
        pinMode(i, OUTPUT);
    }

    // scanI2C();

    // Serial.println("Enabling MCP23017 pins");
    // mcp.begin();
    // for (int i = 0; i < 16; i++)
    // {
    //     mcp.pinMode(0, OUTPUT);
    // }

    // pinMode(PROBE_PIN, OUTPUT);

    // for(int i = 0; i < MAX_PINS; i++){
    //     pinMode(i, OUTPUT);
    // }

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
        if((toggle = !toggle)){
            digitalWrite(RED_1, HIGH);
            digitalWrite(RED_2, LOW);
        }else{
            digitalWrite(RED_1, LOW);
            digitalWrite(RED_2, HIGH);
        }
    }
    Serial.println("Connected");

    //turn off the light show
    digitalWrite(RED_1, LOW);
    digitalWrite(RED_2, LOW);
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
    server.onNotFound(handle_notFound);

    Serial.println("Starting Websockets Server...");
    wss.begin();
    wss.onEvent(handle_webSocketEvent);

    Serial.println("Starting PWM signal channels...");
    ledcSetup(LED1_RED_PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcSetup(LED1_GREEN_PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcSetup(LED2_RED_PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcSetup(LED2_GREEN_PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);

    ledcAttachPin(RED_1, LED1_RED_PWM_CHANNEL);
    ledcAttachPin(GREEN_1, LED1_GREEN_PWM_CHANNEL);
    ledcAttachPin(RED_2, LED2_RED_PWM_CHANNEL);
    ledcAttachPin(GREEN_2, LED2_GREEN_PWM_CHANNEL);

    ledcWrite(LED1_RED_PWM_CHANNEL, 0);
    ledcWrite(LED1_GREEN_PWM_CHANNEL, 0);
    ledcWrite(LED2_RED_PWM_CHANNEL, 0);
    ledcWrite(LED2_GREEN_PWM_CHANNEL, 0);

    Serial.println("Initialization Completed.");
}

void loop()
{

    server.handleClient();
    wss.loop();

    if(digitalRead(BUTTON_PIN) == HIGH){
        if(wasButtonPressedLastLoop == false){
            wasButtonPressedLastLoop = true;
            // Serial.println("PRESSED");
            wss.broadcastTXT("b1");
        }
    }else{
        if(wasButtonPressedLastLoop == true){
            wasButtonPressedLastLoop = false;
            // Serial.println("RELEASED");
            wss.broadcastTXT("b0");
        }
    }

    // for(int i = 255; i >= 0; i--){
    //   ledcWrite(LED1_RED_PWM_CHANNEL, i);
    //   delay(20);
    // }

    //check if the button is pressed now

    // delay(1000);

    // for (auto i : gpioPins)
    // {
    //     digitalWrite(i, LOW);
    // }

    // // for (int i = 0; i < 16; i++)
    // // {
    // //     mcp.digitalWrite(i, HIGH);
    // // }

    // delay(1000);

    // for (auto i : gpioPins)
    // {
    //     digitalWrite(i, HIGH);
    // }

    // // for (int i = 0; i < 16; i++)
    // // {
    // //     mcp.digitalWrite(i, LOW);
    // // }
}

//called on requests for the root webpage
void handle_root(){
    Serial.println("root request");
    server.send(200, "text/html", webpage_html);
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
        for(unsigned int i = 0; i < 6; i++){
            digitalWrite(gpioPins.at(i), ((payload[i+1] == '0') ? LOW : HIGH));
        }
        return true;
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

        ledcWrite(((jsID == 0) ? LED1_GREEN_PWM_CHANNEL : LED2_GREEN_PWM_CHANNEL),
            uint32_t(PWM_MAX_INT*((y > .05) ? y : 0)));
        ledcWrite(((jsID == 0) ? LED1_RED_PWM_CHANNEL : LED2_RED_PWM_CHANNEL),
            uint32_t(PWM_MAX_INT*((y < -.05) ? -y : 0)));

        // if(y > .5){
        //     digitalWrite(((jsID == 0) ? GREEN_1 : GREEN_2), HIGH);
        //     digitalWrite(((jsID == 0) ? RED_1 : RED_2), LOW);
        // }else if(y < -.5){
        //     digitalWrite(((jsID == 0) ? GREEN_1 : GREEN_2), LOW);
        //     digitalWrite(((jsID == 0) ? RED_1 : RED_2), HIGH);
        // }else{
        //     digitalWrite(((jsID == 0) ? GREEN_1 : GREEN_2), LOW);
        //     digitalWrite(((jsID == 0) ? RED_1 : RED_2), LOW);
        // }
        return true;
    default:
        return false;
    }
    return false;
}
