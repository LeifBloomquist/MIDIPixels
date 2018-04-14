// -------------------------------------------------------------
// Arduino control of NeoPixel strips via Sound Input.

// This runs on the Arduino Nano designated MIDI "SLAVE!"  This Arduino controls the NeoPixels.

#include <digitalWriteFast.h>
#include <Adafruit_NeoPixel.h>

#define RIGHT_PIN     11
#define LEFT_PIN      12

#define CHANNEL_BOTH  1
#define CHANNEL_LEFT  2
#define CHANNEL_RIGHT 3

#define LED_PIN       13

#define NUM_PIXELS    60
#define MAX_BRIGHTNESS 140

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel right_strip = Adafruit_NeoPixel(NUM_PIXELS, RIGHT_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel left_strip  = Adafruit_NeoPixel(NUM_PIXELS, LEFT_PIN,  NEO_GRB + NEO_KHZ800);

// ----------------------------------------------------------------------------

void setup()
{
    pinModeFast(LED_PIN, OUTPUT);
    digitalWriteFast(LED_PIN, LOW);

    left_strip.begin();
    right_strip.begin();
    PixelRefresh();

    StartupPulse();
}

void loop()
{
    int effect = random(10);

    switch (effect)
    {  
    case 0:
        Effect2();
        break;

    case 1:
        Effect3();
        break;

    case 2:
        Effect4();
        break;

    default:
        Effect1();
        break;
    }

    
    ClearAllPixels();
}

// ----------------------------------------------------------------------------

void Effect1()
{
    digitalWriteFast(LED_PIN, HIGH);

    int num = random(1, 20);

    for (int i = 0; i < num; i++)
    {
        int pixel = random(NUM_PIXELS);
        SetPixelColor(CHANNEL_BOTH, pixel, 0, 0, MAX_BRIGHTNESS);
    }
    PixelRefresh();

    digitalWriteFast(LED_PIN, LOW);

    delay(1000);
}

void Effect2()
{
    digitalWriteFast(LED_PIN, HIGH);

    int num = 10;

    for (int i = 0; i < num; i++)
    {
        int pixel = random(NUM_PIXELS);
        SetPixelColor(CHANNEL_BOTH, pixel, 0, 0, MAX_BRIGHTNESS);
        PixelRefresh();
        delay(100);
    }
    digitalWriteFast(LED_PIN, LOW);   
}

void Effect3()
{
    digitalWriteFast(LED_PIN, HIGH);

    int num = random(1, 20);

    for (int i = 0; i < num; i++)
    {
        int pixel = random(NUM_PIXELS);
        SetPixelColor(CHANNEL_LEFT, pixel, 0, 0, MAX_BRIGHTNESS);
    }

    for (int i = 0; i < num; i++)
    {
        int pixel = random(NUM_PIXELS);
        SetPixelColor(CHANNEL_RIGHT, pixel, 0, 0, MAX_BRIGHTNESS);
    }

    PixelRefresh();

    digitalWriteFast(LED_PIN, LOW);
    delay(1000);
}

void Effect4()
{
    digitalWriteFast(LED_PIN, HIGH);

    int num = random(1, 20);
    
    for (int i = 0; i < num; i++)
    {
        int pixel = random(NUM_PIXELS);
        SetPixelColor(CHANNEL_BOTH, pixel, 0, 0, MAX_BRIGHTNESS);
    }
    PixelRefresh();

    // Left arm
    for (int i = NUM_PIXELS-1; i >= 0; i--)
    {
        uint32_t save = left_strip.getPixelColor(i);
        SetPixelColor(CHANNEL_LEFT, i, MAX_BRIGHTNESS, 0, 0);
        PixelRefresh();
        delay(8);  // approx 1000/120
        left_strip.setPixelColor(i, save);
    }

    // Right arm
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        uint32_t save = right_strip.getPixelColor(i);
        SetPixelColor(CHANNEL_RIGHT, i, MAX_BRIGHTNESS, 0, 0);
        PixelRefresh();
        delay(8);  // approx 1000/120
        right_strip.setPixelColor(i, save);
    }

    ClearAllPixels();
}


// ----------------------------------------------------------------------------

inline void PixelRefresh()
{
    left_strip.show();
    right_strip.show();
}

void SetPixelColor(byte channel, byte led, byte red, byte green, byte blue)
{
	switch (channel)
	{
	case CHANNEL_BOTH:
        right_strip.setPixelColor(led, red, green, blue);
        left_strip.setPixelColor(led, red, green, blue);
		break;

	case CHANNEL_RIGHT:
        right_strip.setPixelColor(led, red, green, blue);
		break;

	case CHANNEL_LEFT:
        left_strip.setPixelColor(led, red, green, blue);
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
		SetPixelColor(CHANNEL_BOTH, i, 0, 0, 255);
		PixelRefresh();
		delay(20);
		SetPixelColor(CHANNEL_BOTH, i, 0, 0, 0);
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
