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

byte status = 0;
byte data1 = 0;
byte data2 = 0;

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
    // 1. Wait for the signal from the slave (positive edge). 
    while (digitalRead(SIGNAL_PIN) == LOW) {}

    digitalWriteFast(LED_PIN, HIGH);

    // 2. Wait for MIDI data (may already be buffered)
    while (true)
    {
        status = ReadByte(Serial);   // Wait for any byte

        if ((status & 0x80) > 0)     // Status Byte detected
        {
            data1 = ReadByte(Serial);  // Read following data bytes
            data2 = ReadByte(Serial);
            break;
        }
    }

    // 3. Send the three bytes
    softSerial.write(status);
    softSerial.write(data1);
    softSerial.write(data2);    

    digitalWriteFast(LED_PIN, LOW);

    // 3. GOTO 1
}

void MIDIsetup()
{
    Serial.begin(115200);
}

int ReadByte(Stream& in)
{
    while (in.available() == 0) {}
    return in.read();
}

// EOF
