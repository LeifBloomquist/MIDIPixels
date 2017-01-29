// -------------------------------------------------------------
// Arduino control of NeoPixel strips via MIDI.
// By Leif Bloomquist

#include <MIDI.h>   // Use1ByteParsing?
#include <Adafruit_NeoPixel.h>
#include <HardwareSerial.h>     // Need to increase the buffer size in here?

#define RIGHT_PIN 2
#define LEFT_PIN  3

#define NUM_PIXELS  60   // Per side!

#define CHANNEL_RED   1
#define CHANNEL_GREEN 2
#define CHANNEL_BLUE  3
#define CHANNEL_WHITE 4

#define CC_BRIGHTNESS_RIGHT 74
#define CC_BRIGHTNESS_LEFT  75

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel right_strip = Adafruit_NeoPixel(NUM_PIXELS, RIGHT_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel left_strip  = Adafruit_NeoPixel(NUM_PIXELS, LEFT_PIN,  NEO_GRB + NEO_KHZ800);

MIDI_CREATE_DEFAULT_INSTANCE();  // Baud rate needs to be changed to 38400/115200 in midi_Settings.h !!!!   Hairless MIDI must match

boolean changed = false;

// ----------------------------------------------------------------------------

void setup()
{
    MIDIsetup();

    right_strip.begin();
    left_strip.begin();
    PixelRefresh();

    Pulse();
}

void loop()
{
    // Don't do a heck of a lot here.  All the fun stuff is done in the MIDI handlers.
    changed = false;

    // Call MIDI.read the fastest you can for real-time performance.
    MIDI.read();

    if (changed)
    {
        PixelRefresh();
        changed = false;
    }
}

// ----------------------------------------------------------------------------

void MIDIsetup()
{
    // Initiate MIDI communications, listen to all channels
    MIDI.begin(MIDI_CHANNEL_OMNI);
    Serial.begin(115200);

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

    byte brightness = velocity << 2;  // 0 to 254

    uint32_t current_color = left_strip.getPixelColor(note);
   
    byte red   = (current_color >> 16) & 0xFF;
    byte green = (current_color >> 8) & 0xFF;
    byte blue  = (current_color) & 0xFF;

    switch (channel)
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
        case CC_BRIGHTNESS_RIGHT:
            HandleBrightnessRight(channel, value * 2);
            break;

        case CC_BRIGHTNESS_LEFT:
            HandleBrightnessLeft(channel, value * 2);
            break;


        default:  // Ignore all others
            return;
    }
}

uint32_t CalculateBrightness(byte channel, byte brightness)
{
    byte red = 0;
    byte green = 0;
    byte blue = 0;

    switch (channel)
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

    default:  // Ignore all others (black)
        return 0;
    }

    return right_strip.Color(red, green, blue);
}

void HandleBrightnessRight(byte channel, byte brightness)
{
    uint32_t color = CalculateBrightness(channel, brightness);

    for (int i = 0; i < NUM_PIXELS; i++)
    {
        right_strip.setPixelColor(i, color);
    }
    changed = true;
}


void HandleBrightnessLeft (byte channel, byte brightness)
{
    uint32_t color = CalculateBrightness(channel, brightness);

    for (int i = 0; i < NUM_PIXELS; i++)
    {
        left_strip.setPixelColor(i, color);
    }
    changed = true;
}

void Pulse()
{
    for (byte bright = 0; bright < 250; bright +=5)
    {
        for (int i = 0; i < NUM_PIXELS; i++)
        {
            left_strip.setPixelColor(i, bright, bright, bright);
            right_strip.setPixelColor(i, bright, bright, bright);
        }       
        PixelRefresh();
    }

    for (byte bright = 250; bright > 0; bright -= 5)
    {
        for (int i = 0; i < NUM_PIXELS; i++)
        {
            left_strip.setPixelColor(i, bright, bright, bright);
            right_strip.setPixelColor(i, bright, bright, bright);
        }        
        PixelRefresh();
    }

    ClearAllPixels();
}

void ClearAllPixels()
{
    left_strip.clear();
    right_strip.clear();
    PixelRefresh();
}

// EOF