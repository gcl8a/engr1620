const int potPin = A0;
const int ledPin = 11;

void setup() 
{
  pinMode(ledPin, OUTPUT);
  pinMode(potPin, INPUT);
}

void loop() 
{
  int adcValue = analogRead(potPin);
  analogWrite(ledPin, adcValue / 4);

  delay(100);
}
