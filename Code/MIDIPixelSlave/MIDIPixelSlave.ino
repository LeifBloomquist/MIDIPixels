// -------------------------------------------------------------
// Arduino control of NeoPixel strips via MIDI.

// MIDI "SLAVE!"  This Arduino controls the NeoPixels.

#include <SoftwareSerial.h> 
#include <digitalWriteFast.h>
#include <Adafruit_NeoPixel.h>

#define RX_PIN        8
#define TX_PIN        9
                      
#define RIGHT_PIN     11
#define LEFT_PIN      12

#define SIGNAL_PIN    10
#define LED_PIN       13

#define NUM_PIXELS    60  

#define CHANNEL_BOTH_RED    0   // MIDI Channel 1
#define CHANNEL_BOTH_GREEN  1   // MIDI Channel 2
#define CHANNEL_BOTH_BLUE   2   // MIDI Channel 3
#define CHANNEL_BOTH_WHITE  3   // MIDI Channel 4
#define CHANNEL_RIGHT_RED   4   // MIDI Channel 5
#define CHANNEL_RIGHT_GREEN 5   // MIDI Channel 6
#define CHANNEL_RIGHT_BLUE  6   // MIDI Channel 7
#define CHANNEL_RIGHT_WHITE 7   // MIDI Channel 8
#define CHANNEL_LEFT_RED    8   // MIDI Channel 9
#define CHANNEL_LEFT_GREEN  9   // MIDI Channel 10
#define CHANNEL_LEFT_BLUE   10  // MIDI Channel 11
#define CHANNEL_LEFT_WHITE  11  // MIDI Channel 12

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

SoftwareSerial softSerial(RX_PIN, TX_PIN); // RX, TX

// ----------------------------------------------------------------------------

void setup()
{
    left_strip.begin();
    right_strip.begin();
    PixelRefresh();

	softSerial.begin(57600);

    pinModeFast(SIGNAL_PIN, OUTPUT);
    digitalWriteFast(SIGNAL_PIN, HIGH);

    pinModeFast(LED_PIN, OUTPUT);
    digitalWriteFast(LED_PIN, LOW);

    StartupPulse();
}

void loop()
{
    // 1. Set the signal high to tell the other side to send a single MIDI message.
    digitalWriteFast(SIGNAL_PIN, HIGH);
    digitalWriteFast(LED_PIN, LOW);

    // 2. Wait for exactly three bytes.  
    while (softSerial.available() < 3)
    {
        ;   // TODO - Timeout detection?
    }

    // 3. Tell other side to wait again
    digitalWriteFast(SIGNAL_PIN, LOW);
    digitalWriteFast(LED_PIN, HIGH);

    // 4. Parse the data
    byte status = softSerial.read();
    byte data1  = softSerial.read();
    byte data2  = softSerial.read();

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

    byte brightness = velocity << 1;  // 0 to 254

	uint32_t current_color = GetCurrentPixelColor(channel, note);

    byte red = (current_color >> 16) & 0xFF;
    byte green = (current_color >> 8) & 0xFF;
    byte blue = (current_color) & 0xFF;

    switch (channel)
    {
	case CHANNEL_BOTH_RED:
	case CHANNEL_RIGHT_RED:
	case CHANNEL_LEFT_RED:
        red = brightness;
        break;

	case CHANNEL_BOTH_GREEN:
    case CHANNEL_RIGHT_GREEN:
	case CHANNEL_LEFT_GREEN:
        green = brightness;
        break;

	case CHANNEL_BOTH_BLUE:
    case CHANNEL_RIGHT_BLUE:
	case CHANNEL_LEFT_BLUE:
        blue = brightness;
        break;

	case CHANNEL_BOTH_WHITE:
    case CHANNEL_RIGHT_WHITE:
	case CHANNEL_LEFT_WHITE:
        red = brightness;
        green = brightness;
        blue = brightness;
        break;

    default:  // Ignore all others
        return;
    }

	SetPixelColor(channel, note, red, green, blue);   
}

// -----------------------------------------------------------------------------
void HandleNoteOff(byte channel, byte note, byte velocity)
{
	uint32_t current_color = GetCurrentPixelColor(channel, note);

    byte red = (current_color >> 16) & 0xFF;
    byte green = (current_color >> 8) & 0xFF;
    byte blue = (current_color) & 0xFF;

    switch (channel)
    {
	case CHANNEL_BOTH_RED:
	case CHANNEL_RIGHT_RED:
	case CHANNEL_LEFT_RED:
        red = 0;
        break;

	case CHANNEL_BOTH_GREEN:
	case CHANNEL_RIGHT_GREEN:
	case CHANNEL_LEFT_GREEN:
        green = 0;
        break;

	case CHANNEL_BOTH_BLUE:
	case CHANNEL_RIGHT_BLUE:
	case CHANNEL_LEFT_BLUE:
        blue = 0;
        break;

	case CHANNEL_BOTH_WHITE:
	case CHANNEL_RIGHT_WHITE:
	case CHANNEL_LEFT_WHITE:
        red = 0;
        green = 0;
        blue = 0;
        break;

    default:  // Ignore all others
        return;
    }

	SetPixelColor(channel, note, red, green, blue);
}


void HandleControlChange(byte channel, byte number, byte value)
{
    switch (number)
    {
    case CC_BRIGHTNESS:
        HandleBrightness(channel, value << 1);
        break;

    default:  // Ignore all others
        return;
    }
}

void HandleBrightness(byte channel, byte brightness)
{
    byte red = 0;
    byte green = 0;
    byte blue = 0;

    switch (channel)
    {
	case CHANNEL_BOTH_RED:
	case CHANNEL_RIGHT_RED:
	case CHANNEL_LEFT_RED:
        red = brightness;
        break;

	case CHANNEL_BOTH_GREEN:
	case CHANNEL_RIGHT_GREEN:
	case CHANNEL_LEFT_GREEN:
        green = brightness;
        break;

	case CHANNEL_BOTH_BLUE:
	case CHANNEL_RIGHT_BLUE:
	case CHANNEL_LEFT_BLUE:
        blue = brightness;
        break;

	case CHANNEL_BOTH_WHITE:
	case CHANNEL_RIGHT_WHITE:
	case CHANNEL_LEFT_WHITE:
        red = brightness;
        green = brightness;
        blue = brightness;
        break;

    default:  // Ignore all others
        return;
    }

	switch (channel)
	{
	case CHANNEL_BOTH_RED:
	case CHANNEL_BOTH_GREEN:
	case CHANNEL_BOTH_BLUE:
	case CHANNEL_BOTH_WHITE:
		for (int i = 0; i < NUM_PIXELS; i++)
		{
			right_strip.setPixelColor(i, red, green, blue);
			left_strip.setPixelColor(i, red, green, blue);
		}
		break;

	case CHANNEL_RIGHT_RED:
	case CHANNEL_RIGHT_GREEN:
	case CHANNEL_RIGHT_BLUE:
	case CHANNEL_RIGHT_WHITE:
		for (int i = 0; i < NUM_PIXELS; i++)
		{
			right_strip.setPixelColor(i, red, green, blue);
		}		
		break;

	case CHANNEL_LEFT_RED:
	case CHANNEL_LEFT_GREEN:
	case CHANNEL_LEFT_BLUE:
	case CHANNEL_LEFT_WHITE:
		for (int i = 0; i < NUM_PIXELS; i++)
		{
			left_strip.setPixelColor(i, red, green, blue);
		}
		break;

	default:
		// Ignore
		break;
	}  
}

uint32_t GetCurrentPixelColor(byte channel, byte note)
{
	switch (channel)
	{
	case CHANNEL_BOTH_RED:     // When reading current color from "both", assume RIGHT
	case CHANNEL_BOTH_GREEN:
	case CHANNEL_BOTH_BLUE:
	case CHANNEL_BOTH_WHITE:
	case CHANNEL_RIGHT_RED:
	case CHANNEL_RIGHT_GREEN:
	case CHANNEL_RIGHT_BLUE:
	case CHANNEL_RIGHT_WHITE:
		return right_strip.getPixelColor(note);
		break;

	case CHANNEL_LEFT_RED:
	case CHANNEL_LEFT_GREEN:
	case CHANNEL_LEFT_BLUE:
	case CHANNEL_LEFT_WHITE:
		return left_strip.getPixelColor(note);
		break;

	default:
		return 0;
	}
}

void SetPixelColor(byte channel, byte note, byte red, byte green, byte blue)
{
	switch (channel)
	{
	case CHANNEL_BOTH_RED:
	case CHANNEL_BOTH_GREEN:
	case CHANNEL_BOTH_BLUE:
	case CHANNEL_BOTH_WHITE:
		right_strip.setPixelColor(note, red, green, blue);
		left_strip.setPixelColor(note, red, green, blue);
		break;

	case CHANNEL_RIGHT_RED:
	case CHANNEL_RIGHT_GREEN:
	case CHANNEL_RIGHT_BLUE:
	case CHANNEL_RIGHT_WHITE:
		right_strip.setPixelColor(note, red, green, blue);
		break;

	case CHANNEL_LEFT_RED:
	case CHANNEL_LEFT_GREEN:
	case CHANNEL_LEFT_BLUE:
	case CHANNEL_LEFT_WHITE:
		left_strip.setPixelColor(note, red, green, blue);
		break;

	default:
		// Ignore
		break;
	}
}

void StartupPulse()
{
	ClearAllPixels();

	for (int i = 0; i < NUM_PIXELS; i++)
	{
		SetPixelColor(CHANNEL_BOTH_WHITE, i, 255, 255, 255);
		PixelRefresh();
		delay(20);
		SetPixelColor(CHANNEL_BOTH_WHITE, i, 0, 0, 0);
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
