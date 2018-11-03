
#define CNT_A 6  
#define CNT_B 7  
#define BUTTON 8  

void setup ()
{
  Serial.begin(9600);
  pinMode(CNT_A, INPUT);
  pinMode(CNT_B, INPUT);
  pinMode(BUTTON, INPUT);
}

void loop ()
{
  int cntA = digitalRead(CNT_A);
  int cntB = digitalRead(CNT_B);
  int button = digitalRead(BUTTON);
  
  Serial.print("val = (");
  Serial.print(cntA);
  Serial.print(", ");
  Serial.print(cntB);
  Serial.print(", ");
  Serial.print(button);
  Serial.println(")");
  
  delay(10);
}
