#define Serial SerialUSB

//#include <vector.h>
#include <adxl327.h>
#include <event_timer.h>

String inputString;

#define XPIN A0
#define YPIN A1
#define ZPIN A2

ADXL327 adxl(XPIN, YPIN, ZPIN);

Timer timer;

void setup() 
{
  //open serial communications:
  Serial.begin(115200);

  adxl.Initialize();
  timer.Start(20);
}

void loop() 
{
  if(timer.CheckExpired())
  {
    timer.Restart();
    SendAcceleration();
  }
}

void SendAcceleration(void)
{
  ivector reading = adxl.ReadAccelerometer();
  ivector acc = adxl.CalcAcceleration(reading);
  
  Serial.print(acc[0]/1000.);
  Serial.print(" ");
  Serial.print(acc[1]/1000.);
  Serial.print(" ");
  Serial.print(acc[2]/1000.);
  
  Serial.print('\n');
}

