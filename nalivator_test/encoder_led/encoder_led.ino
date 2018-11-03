#include <FastLED.h>

//FASTLED_USING_NAMESPACE

#define LED 13 
#define IN 2

#define LED_COUNT 60
#define LED_PIN 3
#define BRIGHTNESS 70

#define CNT_A 6  /* CLK */
#define CNT_B 7  /* DT */
#define BUTTON 8  

#define MAX_VAL LED_COUNT
#define MIN_VAL 0

CRGB strip[LED_COUNT];

int cntA;
int cntB;
int button;
int prevCntA;

unsigned long currentTime;
unsigned long loopTime;
int value;

void setup ()
{
  Serial.begin(9600);
  pinMode(CNT_A, INPUT);
  pinMode(CNT_B, INPUT);
  pinMode(BUTTON, INPUT);
  FastLED.addLeds<NEOPIXEL, LED_PIN >(strip, LED_COUNT);
  FastLED.setBrightness( BRIGHTNESS );
  FastLED.show();
}

void loop ()
{
  currentTime = millis();

  if (currentTime >= (loopTime + 2))
  {
    int i;
    
    cntA = digitalRead(CNT_A);
    cntB = digitalRead(CNT_B);

    if (!cntA && (prevCntA))
    {
      if (cntB)
      {
        if (value > MIN_VAL) value--;
      } else {
        if (value < MAX_VAL) value++;
      }

//      Serial.print("val = ");
//      Serial.println(value);

      for (i = 0; i < value; i++)
      {
        strip[i] = 0xFF0000 | ((((int)(255 / LED_COUNT)) * value) << 8);
      }
    
      for (; i < LED_COUNT; i++)
      {
        strip[i] = 0x000000;
      }  

      FastLED.show();
    }

    prevCntA = cntA;

    loopTime = currentTime;
  }
}
