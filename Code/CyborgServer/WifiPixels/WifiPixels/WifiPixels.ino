#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>

#include "C:\\Leif\\GitHub\\ESP8266\\Common\\ssids.h"

#define NUM_LEDS 8 //60
#define DATA_PIN 4

#define UDP_PORT 1337
#define MAX_PACKET_SIZE 500

// UDP Server
WiFiUDP Udp;

// Define the array of leds
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ400);   // (LB) had to swap from RGB!

byte incomingPacket[MAX_PACKET_SIZE];

void setup()
{
    Serial.begin(115200);
    delay(100);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    Udp.begin(UDP_PORT);

    // Set up the NeoPixels
    strip.begin();
    strip.show();

    fillUpLEDs(255, 255, 255);
    fillUpLEDs(0, 0, 0);
}

void loop()
{
    int packetSize = Udp.parsePacket();
    if (packetSize > 0)
    {
        Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
        int len = Udp.read(incomingPacket, MAX_PACKET_SIZE);

        if (len > 0)
        {
            int offset = 0;
            for (uint16_t i = 0; i < strip.numPixels(); i++)
            {
                strip.setPixelColor(i, incomingPacket[offset++], incomingPacket[offset++], incomingPacket[offset++]);
            }
            strip.show();
        }
    }   
}


//Updates color of LEDs from bottom up
void fillUpLEDs(uint8_t r, uint8_t g, uint8_t b) {
    for (uint16_t i = 0; i<strip.numPixels(); i++){
        strip.setPixelColor(i, r, g, b);
        delay(10); //delay adds fun animation effect
        strip.show();
    }
}
