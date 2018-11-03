#define LED 13 
#define IN 2

void setup ()
{
  Serial.begin(9600);
  pinMode(IN, INPUT);
  pinMode(LED, OUTPUT);
}

void loop ()
{
  int val = digitalRead(IN);
  Serial.print("val = ");
  Serial.println(val);

  if (val > 0)
  {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }
  
  delay(100);
}
