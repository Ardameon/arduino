
#define CNT_A 6  /* CLK */
#define CNT_B 7  /* DT */
#define BUTTON 8  

#define MAX_VAL 30
#define MIN_VAL 0

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
}

void loop ()
{
  currentTime = millis();

  if (currentTime >= (loopTime + 5))
  {
    cntA = digitalRead(CNT_A);
    cntB = digitalRead(CNT_B);

    if (!cntA && (prevCntA))
    {
      if (cntB)
      {
        if (value < MAX_VAL) value++;
      } else {
        if (value > MIN_VAL) value--;
      }

      Serial.print("val = ");
      Serial.println(value);
    }

    prevCntA = cntA;

    loopTime = currentTime;
  }
}
