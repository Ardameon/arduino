#include <FastLED.h>

//FASTLED_USING_NAMESPACE

#define LED 13 
#define IN 2

#define LED_COUNT 60
#define LED_PIN 3
#define BRIGHTNESS 70

CRGB strip[LED_COUNT];

void setup ()
{
//  Serial.begin(9600);
//  pinMode(IN, INPUT);
//  pinMode(LED, OUTPUT);
  FastLED.addLeds<NEOPIXEL, LED_PIN >(strip, LED_COUNT);
  FastLED.setBrightness( BRIGHTNESS );
}

void loop ()
{
  int i;

  for (i = 0; i < LED_COUNT; i++)
  {
    strip[i] = 0xFF0000 | ((((int)(255 / 60)) * i) << 8);
    
    FastLED.show();
    delay(100);  
  }

  for (i = LED_COUNT - 1; i >= 0; i--)
  {
    strip[i] = 0x000000;
    FastLED.show();
    delay(100);  
  }  
}
