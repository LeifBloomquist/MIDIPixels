// -------------------------------------------------------------
// Arduino control of NeoPixel strips via MIDI.

// MIDI "SLAVE!"  This Arduino controls the NeoPixels.

#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h> 
#include <digitalWriteFast.h>

#define LEFT_PIN  11
#define RIGHT_PIN 12

#define SIGNAL_PIN 7
#define LED_PIN   13

#define NUM_PIXELS  60  

#define CHANNEL_RED   0   // MIDI Channel 1
#define CHANNEL_GREEN 1   // MIDI Channel 2
#define CHANNEL_BLUE  2   // MIDI Channel 3
#define CHANNEL_WHITE 3   // MIDI Channel 4

#define CC_BRIGHTNESS 74

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel right_strip = Adafruit_NeoPixel(NUM_PIXELS, RIGHT_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel left_strip  = Adafruit_NeoPixel(NUM_PIXELS, LEFT_PIN,  NEO_GRB + NEO_KHZ800);

SoftwareSerial softSerial(A0, A1); // RX, TX

// ----------------------------------------------------------------------------

void setup()
{
    MIDIsetup();
    left_strip.begin();
    right_strip.begin();
    PixelRefresh();

    Serial.begin(115200);
    softSerial.begin(57600);

    pinModeFast(LED_PIN, OUTPUT);
    digitalWriteFast(LED_PIN, HIGH);

    pinModeFast(SIGNAL_PIN, OUTPUT);
    digitalWriteFast(SIGNAL_PIN, HIGH);
}

void loop()
{
    // 1. Set the signal high to tell the other side to send a single MIDI message.
    digitalWriteFast(SIGNAL_PIN, HIGH);
    digitalWriteFast(LED_PIN, HIGH);

    // 2. Wait for exactly three bytes.  
    while (softSerial.available() < 3)
    {
        ;   // TODO - Timeout detection?
    }

    // 3. Tell other side to wait again
    digitalWriteFast(SIGNAL_PIN, LOW);
    digitalWriteFast(LED_PIN, LOW);

    // 4. Parse the data
    byte status = softSerial.read();
    byte data1  = softSerial.read();
    byte data2  = softSerial.read();

    /*
    Serial.print("status=");  Serial.println(status, 16);
    Serial.print("data1=");  Serial.println(data1, 16);
    Serial.print("data2=");  Serial.println(data2, 16);
    Serial.println();
    */
    
    HandleMIDI(status, data1, data2);

    // 5. Refresh the NeoPixels while no data is coming in
    PixelRefresh();
      
    // 6. GOTO 1
}

// ----------------------------------------------------------------------------

inline void PixelRefresh()
{
    left_strip.show();
    right_strip.show();
}

void MIDIsetup()
{
    Serial.begin(115200);
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
    byte blue = (current_color) & 0xFF;

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
        return 0;
    }

    for (int i = 0; i < NUM_PIXELS; i++)
    {
        left_strip.setPixelColor(i, red, green, blue);
    }
}

// EOF
