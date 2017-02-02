// -------------------------------------------------------------
// Arduino control of NeoPixel strips via MIDI.

// MIDI MASTER!  This Arduino receives and buffers the MIDI to send to the Slave

#include <SoftwareSerial.h> 
#include <digitalWriteFast.h>

#define LEFT_PIN  11
#define RIGHT_PIN 12

#define SIGNAL_PIN 7
#define LED_PIN   13

SoftwareSerial softSerial(A0, A1); // RX, TX

// ----------------------------------------------------------------------------

void setup()
{
    MIDIsetup();

    softSerial.begin(57600);

    pinModeFast(LED_PIN, OUTPUT);
    digitalWriteFast(LED_PIN, LOW);

    pinModeFast(SIGNAL_PIN, INPUT);

    // 2. Send three bytes (dummy for now)
    softSerial.write(0x80);
    softSerial.write(0x05);
    softSerial.write(0x7F);
}

void loop()
{    
    // 1. Wait for the signal from the slave (positive edge)   
    while (digitalRead(SIGNAL_PIN) == LOW) {} 

    digitalWriteFast(LED_PIN, HIGH);

    // 2. Send three bytes (dummy for now)
    softSerial.write(0x90);
    softSerial.write(0x04);
    softSerial.write(0x7F);    

    digitalWriteFast(LED_PIN, LOW);

    // 3. GOTO 1
}

void MIDIsetup()
{
    Serial.begin(115200);
}

/*
inline void ParseMIDI(byte* buffer, int num)
{
for (int i = 0; i < num; i++)
{
if ((buffer[i] & 0x80) > 0)  // Status Byte
{
HandleMIDI(buffer[i], buffer[i + 1], buffer[i + 2]);
i += 2;
}
}
}
*/


// EOF
