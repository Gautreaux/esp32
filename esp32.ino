
#include <Arduino.h>
#include <Wire.h>
// #include "Adafruit_MCP23017.h"
#include <vector>

#include <WiFi.h>
#include "connectionSecrets.h"

#define MAX_PINS 40

#define GREEN_2 13
#define BLUE_2 14
#define RED_1 15
#define RED_2 16
#define GREEN_1 17
#define BLUE_1 18

//21, 22 removed because SDA/SCL
// const std::vector<int> gpioPins{1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 23, 25, 26, 27, 32, 33, 34, 35, 36, 39};
const std::vector<int> gpioPins{RED_1, GREEN_1, BLUE_1, RED_2, GREEN_2, BLUE_2};

WiFiServer server(80); 

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

    Serial.println("Starting Webserver...");

    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("IP Address: ");
    Serial.println(IP);

    server.begin();

    Serial.println("Initialization Completed.");
}

void loop()
{

    delay(1000);

    for (auto i : gpioPins)
    {
        digitalWrite(i, LOW);
    }

    // for (int i = 0; i < 16; i++)
    // {
    //     mcp.digitalWrite(i, HIGH);
    // }

    delay(1000);

    for (auto i : gpioPins)
    {
        digitalWrite(i, HIGH);
    }

    // for (int i = 0; i < 16; i++)
    // {
    //     mcp.digitalWrite(i, LOW);
    // }
}
