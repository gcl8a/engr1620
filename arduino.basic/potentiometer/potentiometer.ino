const int potPin = A0;

void setup() 
{
  pinMode(A0, INPUT);
}

void loop() 
{
  int adcValue = analogRead(potPin);
  analogWrite(ledPin, adcValue / 4);

  delay(100);
}
