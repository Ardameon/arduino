#include <OLED_I2C.h>

#include <Servo.h>
#include <FastLED.h>

//FASTLED_USING_NAMESPACE

#define LED 13 
#define IN 2
#define SERVO_PIN 11

#define SENSORS_CNT 5
#define LEDS_IN_STRIP 60

#define LED_COUNT 55//(LEDS_IN_STRIP - SENSORS_CNT)
#define LED_PIN 3
#define BRIGHTNESS 30

#define CNT_A 6  /* CLK */
#define CNT_B 7  /* DT */
#define BUTTON 8  

#define MAX_VAL 55
#define MIN_VAL 0

#define SENSOR_CHECK_PERIOD 100

CRGB strip[LEDS_IN_STRIP];

int cntA;
int cntB;
int button;
int prevCntA;

int sensor[SENSORS_CNT];
int sensor_led [] = {A0, A1, A2, A3, A4};

unsigned long currentTime;
unsigned long loopTime;
unsigned long lastSensorCheckTime;

OLED  myOLED(SDA, SCL);

extern uint8_t RusFont[];//русский шрифт 
extern uint8_t MediumNumbers[];//шрифт средних чисел
extern uint8_t MegaNumbers[];//шрифт средних чисел
extern uint8_t BigNumbers[];//шрифт средних чисел
extern uint8_t SmallFont[];//шрифт средних чисел

Servo myservo;

int value;

void updateDisplay()
{
  char vod[] = "YFKBNM DJLZHS";
  char ml[] = "vk";
  
  myOLED.clrScr();
  myOLED.setFont(RusFont);
//  myOLED.setFont(SmallFont);
  myOLED.print(vod, CENTER, 5);
  myOLED.setFont(BigNumbers);
  myOLED.printNumI(value * 10, CENTER, 20);
  myOLED.setFont(RusFont);
  myOLED.print(ml, CENTER, 50);
  myOLED.update();
}

void setup ()
{
  int i;
  
  Serial.begin(9600);
  pinMode(CNT_A, INPUT);
  pinMode(CNT_B, INPUT);
  pinMode(BUTTON, INPUT);
  FastLED.addLeds<NEOPIXEL, LED_PIN >(strip, LEDS_IN_STRIP);
  FastLED.setBrightness( BRIGHTNESS );

  myOLED.begin();

  for (i = 0; i < SENSORS_CNT; i++)
  {
    strip[i] = 0x0000FF;
  }
  
  FastLED.show();

  updateDisplay();

  myservo.attach(SERVO_PIN);
}

void loop1 ()
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

      updateDisplay();

      for (i = 0; i < value; i++)
      {
        strip[i + SENSORS_CNT] = 0xFF0000 | ((((int)(255 / LED_COUNT)) * value) << 8);
      }
    
      for (; i < LED_COUNT; i++)
      {
        strip[i + SENSORS_CNT] = 0x000000;
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

      if (sensor[i] < 100)
      {
        strip[i] = 0x00FF00;
      } else {
        strip[i] = 0x0000FF;
      }
    }

    FastLED.show();

    lastSensorCheckTime = currentTime;
  }
}
int pos;

void loop ()
{
  myservo.write(10);
  delay(3000);

  myservo.write(40);
  delay(3000);

  myservo.write(60);
  delay(3000);

  myservo.write(90);
  delay(3000);
}
