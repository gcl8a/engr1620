#include <vector.h>
#include <matrix.h>

#include "tcs3200.h"

int S0 = 8;//pinB
int S1 = 9;//pinA
int S2 = 10;//pinE
int S3 = 11;//pinF
int out = 12;//pinC
int LED = 13;//pinD

TCS3200 tcs(S0, S1, S2, S3, out, LED);

void setup() {
  Serial.begin(115200);

  tcs.Setup();
  delay(1000);
  //tcs.CalibrateRGBMatrix();
  tcs.CollectTrainingData();
  delay(3000);
}

//dvector rgb(3);

void loop() 
{
  tcs.ClassifyColors();
  
  delay(1000);
}

