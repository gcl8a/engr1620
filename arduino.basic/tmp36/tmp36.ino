const int tmp36pin = A1;

void setup() 
{
  Serial.begin(9600);

  pinMode(tmp36pin, INPUT);
}

void loop() 
{
  int adcValue = analogRead(tmp36pin);
  
  float voltage = 0;      //to be edited by the student!
  float temperature = 0;  //to be edited by the student!

  Serial.print(adcValue);
  Serial.println();
}
