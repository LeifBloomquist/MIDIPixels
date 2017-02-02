// -------------------------------------------------------------
// Arduino control of NeoPixel strips via MIDI.

//#include <MIDI.h>               // Use1ByteParsing set to false, and BaudRate set to 115200
#include <Adafruit_NeoPixel.h>
//#include <FastLED-3.1.3\FastLED.h>
#include <HardwareSerial.h>     // Increased the buffer size to 1024

#define PIN 11
#define DEBUG_PIN 12

#define NUM_PIXELS  60  

#define CHANNEL_RED   1
#define CHANNEL_GREEN 2
#define CHANNEL_BLUE  3
#define CHANNEL_WHITE 4

#define CC_BRIGHTNESS 74

#define OFFSET 1  // 1 for my handler, 0 for MIDI

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel debug_strip = Adafruit_NeoPixel(NUM_PIXELS, DEBUG_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel left_strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);
//
//CRGB leds[NUM_PIXELS];

boolean changed = false;

// Implement my own #&*$ bufffer!
byte mybuffer[3 * 200] = { 0 };
int index = 0;

// ----------------------------------------------------------------------------

void setup()
{
    MIDIsetup();
    left_strip.begin();
    debug_strip.begin();
    PixelRefresh();
}

void loop()
{
    debug_strip.clear();
    debug_strip.setPixelColor(0, 0, 50, 0);
 
    // Don't do a heck of a lot here.  All the fun stuff is done in the MIDI handlers.
    changed = false;

    // Read all data
    index = 0;

    while (Serial.available() > 0)
    {  
        mybuffer[index++] = Serial.read();
        debug_strip.setPixelColor(7, index, 0, 0);
    }

    debug_strip.setPixelColor(1, 0, 0, 20);
    PixelRefresh();

    ParseMIDI(mybuffer, index);
    
   
    // Only update NeoPixels if no data, and pattern has changed.
//    if (changed)
    {
 //       PixelRefresh();
 //       changed = false;
    }
}

// ----------------------------------------------------------------------------

inline void PixelRefresh()
{
    left_strip.show();
    debug_strip.show();
}

void MIDIsetup()
{
    Serial.begin(115200);
}

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

inline void HandleMIDI(byte status, byte data1, byte data2)
{
    switch (status & 0xF0)
    {
        case 0x90:
            HandleNoteOn(status & 0x0F, data1, data2);
            break;

        case 0x80:
            HandleNoteOff(status & 0x0F, data1, data2);
            break;

        case 0xB0:
            HandleControlChange(status & 0x0F, data1, data2);
            break;

        default:  // Ignore all others
            break;
    }
}


// -----------------------------------------------------------------------------
void HandleNoteOn(byte channel, byte note, byte velocity)
{
    // This acts like a NoteOff.
    if (velocity == 0)
    {
        HandleNoteOff(channel, note, velocity);
        return;
    }

    byte brightness = velocity << 2;  // 0 to 254

    uint32_t current_color = left_strip.getPixelColor(note);

    byte red = (current_color >> 16) & 0xFF;
    byte green = (current_color >> 8) & 0xFF;
    byte blue = (current_color)& 0xFF;

    switch (channel + OFFSET)
    {
    case CHANNEL_RED:
        red = brightness;
        break;

    case CHANNEL_GREEN:
        green = brightness;
        break;

    case CHANNEL_BLUE:
        blue = brightness;
        break;

    case CHANNEL_WHITE:
        red = brightness;
        green = brightness;
        blue = brightness;
        break;

    default:  // Ignore all others
        return;
    }

    left_strip.setPixelColor(note, red, green, blue);
    changed = true;
}

// -----------------------------------------------------------------------------
void HandleNoteOff(byte channel, byte note, byte velocity)
{
    uint32_t current_color = left_strip.getPixelColor(note);

    byte red = (current_color >> 16) & 0xFF;
    byte green = (current_color >> 8) & 0xFF;
    byte blue = (current_color)& 0xFF;

    switch (channel + OFFSET)
    {
    case CHANNEL_RED:
        red = 0;
        break;

    case CHANNEL_GREEN:
        green = 0;
        break;

    case CHANNEL_BLUE:
        blue = 0;
        break;

    case CHANNEL_WHITE:
        red = 0;
        green = 0;
        blue = 0;
        break;

    default:  // Ignore all others
        return;
    }

    left_strip.setPixelColor(note, red, green, blue);
    changed = true;
}


void HandleControlChange(byte channel, byte number, byte value)
{
    switch (number)
    {
        case CC_BRIGHTNESS:
            HandleBrightness(channel, value * 2);
            break;

        default:  // Ignore all others
            return;
    }
}

uint32_t HandleBrightness(byte channel, byte brightness)
{
    byte red = 0;
    byte green = 0;
    byte blue = 0;

    switch (channel + OFFSET)
    {
    case CHANNEL_RED:
        red = brightness;
        break;

    case CHANNEL_GREEN:
        green = brightness;
        break;

    case CHANNEL_BLUE:
        blue = brightness;
        break;

    case CHANNEL_WHITE:
        red = brightness;
        green = brightness;
        blue = brightness;
        break;

    default:  // Ignore all others
        return 0;
    }

    for (int i = 0; i < NUM_PIXELS; i++)
    {
        left_strip.setPixelColor(i, red, green, blue);
    }
    changed = true;
}

// EOF