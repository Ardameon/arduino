#include <FastLED.h>

#define SENSORS_CNT 5
#define STATES_CNT 6
#define LEDS_IN_STRIP 26

#define LED_COUNT (LEDS_IN_STRIP - SENSORS_CNT - STATES_CNT)
#define LED_PIN 3
#define BRIGHTNESS 30

#define CNT_A 6  /* CLK */
#define CNT_B 7  /* DT */
#define BUTTON 8  

#define MAX_VAL LED_COUNT /* 15 */
#define MIN_VAL 0

#define SENSOR_CHECK_PERIOD 100

CRGB strip[LEDS_IN_STRIP];

int sensor_led_pos[SENSORS_CNT] = {25, 24, 23, 22, 21};
int states_led_pos[STATES_CNT]  = {5, 4, 3, 2, 1, 0};

int cntA;
int cntB;
int button;
int prevCntA;

int sensor[SENSORS_CNT];
int sensor_led [] = {A1, A2, A3, A4, A5};

unsigned long currentTime;
unsigned long loopTime;
unsigned long lastSensorCheckTime;

int value;

void setup ()
{
  int i;
  
  Serial.begin(9600);
  pinMode(CNT_A, INPUT);
  pinMode(CNT_B, INPUT);
  pinMode(BUTTON, INPUT);
  FastLED.addLeds<NEOPIXEL, LED_PIN >(strip, LEDS_IN_STRIP);
  FastLED.setBrightness( BRIGHTNESS );

  for (i = 0; i < STATES_CNT; i++)
  {
    strip[states_led_pos[i]] = 0x0000FF;
  }
  
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

      Serial.print("val = ");
      Serial.println(value);

      for (i = 0; i < value; i++)
      {
        strip[i + STATES_CNT] = 0xFF0000 | ((((int)(255 / MAX_VAL)) * value) << 8);
      }

      for (; i < LED_COUNT; i++)
      {
        strip[i + STATES_CNT] = 0x000000;
      }

      FastLED.show();
    }

    prevCntA = cntA;

    loopTime = currentTime;
  }

  if (currentTime >= lastSensorCheckTime + SENSOR_CHECK_PERIOD)
  {
    int i;
    
    for (i = 0; i < SENSORS_CNT; i++)
    {
      sensor[i] = analogRead(sensor_led[i]);

      if (sensor[i] > 100)
      {
        strip[sensor_led_pos[i]] = 0x00FF00;
      } else {
        strip[sensor_led_pos[i]] = 0x000000;
      }
    }

    FastLED.show();

    lastSensorCheckTime = currentTime;
  }
}
