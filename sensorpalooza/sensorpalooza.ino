#include <event_timer.h>

int ultraPin = A0;
int IRpin = A1; //proximity
int PIRpin = A2;
int tempPin = A3;
int forcePin = A4;
int lightPin = A5;

int touchPin = 2;

Timer timer;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
  timer.Start(500);
  
  //all pins default to input
}

int count;

// the loop routine runs over and over again forever:
void loop() 
{
  if(timer.CheckExpired())
  {
  timer.Restart();
  
  if(!(count%10))
  {
    Serial.println();
    Serial.print("Time (s)\tTouch\tLight\tForce\tProx. (cm)\tUltrasonic (cm)\tPIR\tTemperature (C)\n");
    Serial.print("--------\t-----\t-----\t-----\t----------\t---------------\t---\t---------------\n");
  }
  
  count++;
  
  Serial.print(count);
  Serial.print("\t\t");
  Serial.print(ReadTouch(touchPin));
  Serial.print('\t');
  Serial.print(ReadLight(lightPin));
  Serial.print('\t');
  Serial.print(ReadForce(forcePin));
  Serial.print('\t');
  Serial.print(ReadIRdistance(IRpin));
  Serial.print("\t\t");
  Serial.print(ReadUltrasonic(ultraPin));
  Serial.print("\t\t");
  Serial.print(ReadPIR(PIRpin));
  Serial.print('\t');
  Serial.print(ReadTemperature(tempPin));
  Serial.println();
  }  
}

float ReadUltrasonic(int pin)
{
  return analogRead(pin);
}

float ReadTemperature(int pin)
{
  int adc = analogRead(pin);
  return adc * 0.2222 - 61.111;
}

int ReadLight(int pin)
{
  return analogRead(pin);
}

int ReadPIR(int pin)
{
  return analogRead(pin);
}

float ReadIRdistance(int pin)
{
  int adc = analogRead(pin);
  float dist = 4800.0 / (adc - 20.0);
  return dist;
}

int ReadTouch(int pin)
{
  return digitalRead(pin);
}

int ReadForce(int pin)
{
  return analogRead(pin);
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}






