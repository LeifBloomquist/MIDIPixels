// -------------------------------------------------------------
// Arduino control of NeoPixel strips via MIDI.
// By Leif Bloomquist

#include <MIDI.h>
#include <Adafruit_NeoPixel.h>

#define RIGHT_PIN 2
#define LEFT_PIN  3

#define CHANNEL_RED   1
#define CHANNEL_GREEN 2
#define CHANNEL_BLUE  3
#define CHANNEL_WHITE 4

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel right_strip = Adafruit_NeoPixel(60, RIGHT_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel left_strip  = Adafruit_NeoPixel(60, LEFT_PIN,  NEO_GRB + NEO_KHZ800);

MIDI_CREATE_DEFAULT_INSTANCE();  // Baud rate needs to be changed to 38400 in midi_Settings.h !!!!

// ----------------------------------------------------------------------------

void setup()
{
    MIDIsetup();

    right_strip.begin();
    left_strip.begin();

    PixelRefresh();
}

void loop()
{
    // Don't do a heck of a lot here.  All the fun stuff is done in the MIDI handlers.
    // Call MIDI.read the fastest you can for real-time performance.
    MIDI.read();
}

// ----------------------------------------------------------------------------

void MIDIsetup()
{
    // Initiate MIDI communications, listen to all channels
    MIDI.begin(MIDI_CHANNEL_OMNI);

    // No MIDIThru
    MIDI.turnThruOff();

    // Connect the functions to the library
    MIDI.setHandleNoteOn(HandleNoteOn);
    MIDI.setHandleNoteOff(HandleNoteOff);
    MIDI.setHandleControlChange(HandleControlChange);
}

void PixelRefresh()
{
    right_strip.show();
    left_strip.show();
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

    note &= 0x7F;

    uint32_t current_color = left_strip.getPixelColor(note);
   
    byte red   = (current_color >> 16) & 0xFF;
    byte green = (current_color >> 8) & 0xFF;
    byte blue  = (current_color) & 0xFF;

    switch (channel)
    {
        case CHANNEL_RED:
            red = velocity;
            break;

        case CHANNEL_GREEN:
            green = velocity;
            break;

        case CHANNEL_BLUE:
            blue = velocity;
            break;

        case CHANNEL_WHITE:
            red = velocity;
            green = velocity;
            blue = velocity;
            break;

        default:  // Ignore
            return;
    }

    left_strip.setPixelColor(note, red, green, blue);
    PixelRefresh();
}

// -----------------------------------------------------------------------------
void HandleNoteOff(byte channel, byte note, byte velocity)
{
    note &= 0x7F;

    uint32_t current_color = left_strip.getPixelColor(note);

    byte red = (current_color >> 16) & 0xFF;
    byte green = (current_color >> 8) & 0xFF;
    byte blue = (current_color) & 0xFF;

    switch (channel)
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

        default:  // Ignore
            return;
    }

    left_strip.setPixelColor(note, red, green, blue);
    PixelRefresh();
}


void HandleControlChange(byte channel, byte number, byte value)
{

}


// EOF