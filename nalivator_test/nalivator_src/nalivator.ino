#include <FastLED.h>
#include <OLED_I2C.h>
#include <Servo.h>


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

#define PUMP_OUTPUT 40 /* ml/s */ 

typedef enum {
  E_STATE_IDLE,
  E_STATE_RUNNING,
  E_STATE_FINISH,
  E_STATE_CLEANING
} nalivator_state_e;

nalivator_state_e state;

CRGB strip[LEDS_IN_STRIP];

int sensor_led_pos[SENSORS_CNT] = {25, 24, 23, 22, 21};
int states_led_pos[STATES_CNT]  = {5, 4, 3, 2, 1, 0};

int cntA;
int cntB;
int prevCntA;

int button_cur;
int button_prev;

int sensor[SENSORS_CNT];
int sensor_led [] = {A5, A4, A3, A2, A1};

unsigned char shotglass_checked[SENSORS_CNT];

unsigned long currentTime;
unsigned long loopTime;
unsigned long lastSensorCheckTime;

unsigned long pumpWorkTime;

int value;

OLED  myOLED(SDA, SCL);

Servo myservo;
int servo_shot_pos[] = {25, 45, 60, 75, 90};
int servo_idle_pos   = 10;
int servo_max_pos    = 92;

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
}

/*================================================================================================*/

void pump_stop()
{
  Serial.println("PUMP STOP");
}

/*================================================================================================*/

void update_display_idle()
{
  char text[] = "YFKBNM DJLZHS";
  char ml[] = "vk";
  
  myOLED.clrScr();
  myOLED.setFont(RusFont);
  // myOLED.setFont(SmallFont);
  myOLED.print(text, CENTER, 5);
  myOLED.setFont(BigNumbers);
  myOLED.printNumI(value * 10, CENTER, 20);
  myOLED.setFont(RusFont);
  // myOLED.setFont(SmallFont);
  myOLED.print(ml, CENTER, 50);
  myOLED.update();
}

/*================================================================================================*/

void update_display_fill_shot(int shot_idx)
{
  char text[] = "YFKBDFTV IJN";
  
  myOLED.clrScr();
  myOLED.setFont(RusFont);
  // myOLED.setFont(SmallFont);
  myOLED.print(text, CENTER, 5);
  myOLED.setFont(BigNumbers);
  myOLED.printNumI(shot_idx, CENTER, 20);
  myOLED.update();
}

/*================================================================================================*/

void update_display_finish()
{
  char text[] = "DJPMVB <E{KBIRJ";
  char text2[] = "(^-^)";
  
  myOLED.clrScr();
  myOLED.setFont(RusFont);
  // myOLED.setFont(SmallFont);
  myOLED.print(text, CENTER, 5);
  myOLED.setFont(SmallFont);
  myOLED.print(text2, CENTER, 20);
  myOLED.update();
}

/*================================================================================================*/

int start_button_pressed()
{
  int res = 0;
  
  button_cur = digitalRead(START_BUTTON);

  if (button_prev && button_cur) res = 1;

  button_prev = button_cur;

  return res;
}

/*================================================================================================*/

void setup ()
{
  int i;
  
  Serial.begin(9600);
  pinMode(CNT_A, INPUT);
  pinMode(CNT_B, INPUT);
  pinMode(START_BUTTON, INPUT);
  FastLED.addLeds<NEOPIXEL, LED_PIN >(strip, LEDS_IN_STRIP);
  FastLED.setBrightness( BRIGHTNESS );
  myOLED.begin();

  for (i = 0; i < STATES_CNT; i++)
  {
    strip[states_led_pos[i]] = 0x0000FF;
  }
  
  FastLED.show();

  update_display_idle();

  myservo.attach(SERVO_PIN);

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

      Serial.print("val = ");
      Serial.println(value);

      update_display_idle();

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
    
    if (start_button_pressed())
    {
      for (i = 0; i < SENSORS_CNT; i++)
      {
          strip[sensor_led_pos[i]] = 0x000000;
      }

      set_state(E_STATE_RUNNING);

    } else {

      for (i = 0; i < SENSORS_CNT; i++)
      {
        sensor[i] = analogRead(sensor_led[i]);

        // if (i == 0)
        // {
        //   Serial.print("sensor 0 = ");
        //   Serial.println(sensor[i]);
        // }

        if (sensor[i] > SENSOR_TRESHOLD)
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
  update_display_fill_shot(shot_idx);

  myservo.write(servo_shot_pos[shot_idx]);

  strip[sensor_led_pos[shot_idx]] = 0x0000FF;
  FastLED.show();

  pump_start();

  delay(pump_work_time);

  pump_stop();

  strip[sensor_led_pos[shot_idx]] = 0xFF0000;
  FastLED.show();
}

/*================================================================================================*/

void loop_running()
{
  int i, delay_val;

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

  delay(100);

  myservo.write(servo_idle_pos);

  delay(1000);

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

  update_display_finish();

  while (any_shotglass_checked())
  {
    for (i = 0; i < SENSORS_CNT; i++)
    {
      {
        sensor[i] = analogRead(sensor_led[i]);

        if (sensor[i] < SENSOR_TRESHOLD)
        {
          strip[sensor_led_pos[i]] = 0x000000;
          shotglass_checked[i] = 0;
        }
      }
    }
    
    FastLED.show();

    delay(100);
  }

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
    case E_STATE_IDLE:     loop_idle();     break;
    case E_STATE_RUNNING:  loop_running();  break;
    case E_STATE_FINISH:   loop_finish();   break;
    case E_STATE_CLEANING: loop_cleaning(); break;
    default: Serial.println("ERROR. Wrong state!");
  }
}

/*================================================================================================*/

