const int tmp36pin = A1;

void setup() 
{
  Serial.begin(9600);

  pinMode(A1, INPUT);
}

void loop() 
{
  int adcValue = analogRead(tmp36pin);
  
  float voltage = 0;
  float temperature = 0;

  Serial.print(adcValue);
  Serial.println();
}
