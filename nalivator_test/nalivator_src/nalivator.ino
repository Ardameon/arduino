#include <FastLED.h>
#include <Servo.h>
#include <OLED_I2C.h>


#define SENSORS_CNT 5
#define STATES_CNT 6
#define LEDS_IN_STRIP 26

#define LED_COUNT (LEDS_IN_STRIP - SENSORS_CNT - STATES_CNT)
#define LED_PIN 3
#define BRIGHTNESS 30

#define CNT_A 6  /* CLK */
#define CNT_B 7  /* DT */

#define MAX_VAL LED_COUNT /* 15 */
#define MIN_VAL 0

#define SENSOR_CHECK_PERIOD 100
#define SENSOR_TRESHOLD 200

#define SERVO_PIN 5
#define START_BUTTON 4 

#define RELEY_PIN 8

#define PUMP_OUTPUT 18 /* ml/s */ 

typedef enum {
  E_STATE_IDLE,
  E_STATE_RUNNING,
  E_STATE_FINISH,
  E_STATE_CLEANING
} nalivator_state_e;

nalivator_state_e state;

CRGB strip[LEDS_IN_STRIP];

int sensor_led_pos[SENSORS_CNT] = {25, 24, 23, 22, 21};
int states_led_pos[STATES_CNT]  = {0, 1, 2, 3, 4, 5};

int cntA;
int cntB;
int prevCntA;

int button_cur;
int button_prev;

int sensor[SENSORS_CNT];
// int sensor_led [] = {A1, A2, A3, A4, A5};
int sensor_led [] = {9, 10, 11, 12, 13};


unsigned char shotglass_checked[SENSORS_CNT];

unsigned long currentTime;
unsigned long loopTime;
unsigned long lastSensorCheckTime;

unsigned long pumpWorkTime;

unsigned long drunkCounter;
int drunkCurrentPos;
unsigned int drunk_treshold[STATES_CNT] = {50, 100, 150, 200, 250, 300};

int value;

Servo myservo;
int servo_shot_pos[] = {40, 70, 100, 130, 160};
int servo_idle_pos   = 20;
int servo_max_pos    = 170;

OLED  myOLED(SDA, SCL);

extern uint8_t RusFont[];//русский шрифт 
extern uint8_t MediumNumbers[];//шрифт средних чисел
extern uint8_t MegaNumbers[];//шрифт средних чисел
extern uint8_t BigNumbers[];//шрифт средних чисел
extern uint8_t SmallFont[];//шрифт средних чисел

const char *state_str(nalivator_state_e st)
{
  switch (st)
  {
    case E_STATE_IDLE:     return "IDLE";
    case E_STATE_RUNNING:  return "RUNNING";
    case E_STATE_FINISH:   return "FINISH";
    case E_STATE_CLEANING: return "CLEANING";
    default:               return "UNKNOWN";
  }
}

/*================================================================================================*/

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

/*================================================================================================*/

void set_state(nalivator_state_e st)
{
  Serial.print(state_str(state));
  Serial.print(" => ");
  Serial.println(state_str(st));
  state = st;
}

/*================================================================================================*/

void pump_start()
{
  Serial.println("PUMP START");
  digitalWrite(RELEY_PIN, 1);
}

/*================================================================================================*/

void pump_stop()
{
  Serial.println("PUMP STOP");
  digitalWrite(RELEY_PIN, 0);
}

/*================================================================================================*/

void update_drunk_state()
{
  int i;

  for (i = 0; i < STATES_CNT; i++)
  {
    strip[i] = 0x000000;
  }

  drunkCounter += value * 10;

  if (drunkCounter > drunk_treshold[STATES_CNT - 1])
  {
    for (i = 0; i < STATES_CNT; i++)
    {
      strip[i] = 0xFF0000;
    }
  } else {
    for (i = STATES_CNT - 1; i >= 0; i--)
    {
      if (drunkCounter >= drunk_treshold[i])
      {
        strip[i] = 0x0000FF;
        break;
      }
    }
  }
}

/*================================================================================================*/


/*================================================================================================*/

int strip_leds_off()
{
  int i;

  for (i = STATES_CNT; i < LED_COUNT; i++)
  {
    strip[i] = 0x000000;
  }
}

/*================================================================================================*/

int strip_leds_on()
{
  int i;

  for (i = 0; i < value; i++)
  {
    strip[i + STATES_CNT] = 0xFF0000 | ((((int)(255 / MAX_VAL)) * value) << 8);
  }
}

/*================================================================================================*/

int start_button_pressed()
{
  int res = 0;
  
  button_cur = digitalRead(START_BUTTON);

//  Serial.print("button_prev = ");
 // Serial.print(button_prev);
 // Serial.print(" button_cur = ");
 // Serial.println(button_cur);

  if (button_prev && button_cur) res = 1;


  Serial.print("res = ");
  Serial.println(res);

  button_prev = button_cur;

  return res;
}

/*================================================================================================*/

void setup()
{
  int i;
  
  Serial.begin(9600);
  pinMode(CNT_A, INPUT);
  pinMode(CNT_B, INPUT);
  pinMode(START_BUTTON, INPUT);
  pinMode(RELEY_PIN, OUTPUT);
  FastLED.addLeds<NEOPIXEL, LED_PIN >(strip, LEDS_IN_STRIP);
  FastLED.setBrightness( BRIGHTNESS );

  // for (i = 0; i < STATES_CNT; i++)
  // {
  //   strip[states_led_pos[i]] = 0x0000FF;
  // }

  for (i = 0; i < SENSORS_CNT; i++)
  {
    pinMode(sensor_led[i], INPUT);
  }
  
  FastLED.show();

  myservo.attach(SERVO_PIN);
  myservo.write(servo_idle_pos);

  digitalWrite(RELEY_PIN, 0);

  delay(1000);

  myOLED.begin();
  updateDisplay();

  myservo.detach();

  set_state(E_STATE_IDLE);
}

/*================================================================================================*/

void loop_idle()
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

      //Serial.print("val = ");
     // Serial.println(value);

      for (i = 0; i < value; i++)
      {
        strip[i + STATES_CNT] = 0xFF0000 | ((((int)(255 / MAX_VAL)) * value) << 8);
      }

      for (; i < LED_COUNT; i++)
      {
        strip[i + STATES_CNT] = 0x000000;
      }

      FastLED.show();

      updateDisplay();
    }

    prevCntA = cntA;

    loopTime = currentTime;
  }

  if (currentTime >= lastSensorCheckTime + SENSOR_CHECK_PERIOD)
  {
    int i;
    
    if (start_button_pressed())
    {
      for (i = 0; i < SENSORS_CNT; i++)
      {
          strip[sensor_led_pos[i]] = 0x000000;
      }

      strip_leds_off();

      set_state(E_STATE_RUNNING);

    } else {

      for (i = 0; i < SENSORS_CNT; i++)
      {
        // sensor[i] = analogRead(sensor_led[i]);
        sensor[i] = digitalRead(sensor_led[i]);

        // if (i == 0)
        // {
        //   Serial.print("sensor 0 = ");
        //   Serial.println(sensor[i]);
        // }

        // if (!sensor[i] < SENSOR_TRESHOLD)
        if (!sensor[i])
        {
          strip[sensor_led_pos[i]] = 0x00FF00;
          shotglass_checked[i] = 1;
        } else {
          strip[sensor_led_pos[i]] = 0x000000;
          shotglass_checked[i] = 0;
        }
      }
    }

    FastLED.show();

    lastSensorCheckTime = currentTime;
  }
}

/*================================================================================================*/

void fill_the_shot(int shot_idx, int pump_work_time)
{
//  myservo.attach(SERVO_PIN);
  myservo.write(servo_shot_pos[shot_idx]);
  

  strip[sensor_led_pos[shot_idx]] = 0x0000FF;
  FastLED.show();
  
  
  delay(1300);
//  myservo.detach();

  pump_start();

  delay(pump_work_time);

  pump_stop();

  delay(2000);

  strip[sensor_led_pos[shot_idx]] = 0xFF0000;
  FastLED.show();
}

/*================================================================================================*/

void loop_running()
{
  int i, delay_val;

  myservo.attach(SERVO_PIN);

  delay_val = (value * 10) / PUMP_OUTPUT * 1000;                  /* int value */
  delay_val += ((value * 10) % PUMP_OUTPUT) * 1000 / PUMP_OUTPUT; /* remainder */

  for (i = 0; i < SENSORS_CNT; i++)
  {
    if (shotglass_checked[i])
    {
      Serial.print("Shot checked ");
      Serial.println(i);

      fill_the_shot(i, delay_val);
    }
  }

  myservo.write(servo_max_pos);

  delay(700);

  myservo.write(servo_idle_pos);

  delay(1500);
  myservo.detach();

  set_state(E_STATE_FINISH);
}

/*================================================================================================*/

int any_shotglass_checked()
{
  int checked = 0;
  int i;

  for (i = 0; i < SENSORS_CNT; i++)
  {
    checked += shotglass_checked[i];
  }

  return checked;
}

/*================================================================================================*/

void loop_finish()
{
  int i;

  update_drunk_state();

  while (any_shotglass_checked())
  {
    for (i = 0; i < SENSORS_CNT; i++)
    {
      {
        // sensor[i] = analogRead(sensor_led[i]);
        sensor[i] = digitalRead(sensor_led[i]);

        // if (sensor[i] < SENSOR_TRESHOLD)
        if (sensor[i])
        {
          strip[sensor_led_pos[i]] = 0x000000;
          shotglass_checked[i] = 0;
        }
      }
    }
    
    FastLED.show();

    delay(100);
  }

  strip_leds_on();
  FastLED.show();

  set_state(E_STATE_IDLE);
}

/*================================================================================================*/

void loop_cleaning()
{

}

/*================================================================================================*/

void loop ()
{
 switch (state)
 {
 case E_STATE_IDLE: loop_idle(); break;
 case E_STATE_RUNNING: loop_running(); break;
 case E_STATE_FINISH: loop_finish(); break;
 case E_STATE_CLEANING: loop_cleaning(); break;
 default: Serial.println("ERROR. Wrong state!");
 }

}

/*================================================================================================*/