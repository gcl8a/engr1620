/*
Adapted from some crappy code at reibot.org. Lots of cleaning up...
 Demo program for TCS3200 from parallax and the parallax daughterboard
 Call detectColor(out) where 'out' is pinC on the daughterboard. The detectColor will return a 0 if there is nothing color in front of sensor,
 1 if red is in front, 2 if blue is in front, or 3 if blue is in front. You can comment out all the serial.print.
 If you're tight on pins, remove the taosMode(int) method and all references to it. Remove pins from the TCS3200setup too.
 If these wires are disconnected the TCS3200 will run on the highest frequency due to internal pullup resistors on S0 and S1
 If you have multiple TCS3200 you may tie all the pins together except the outputs (pinC). Then just use detectColor(TCS3200's output) to
 detect color on the selected TCS3200's output pin.
 7/6/2011 works on arduino 0022
 Taos pins connect to arduino pins 8-13. There is no order and should work on any digital i/o
 */
#include <Arduino.h>

#include <vector.h>
#include <matrix.h>

void PrintVector(const TVector<double>& v)
{
  for(int i = 0; i < v.Length(); i++)
  {
      Serial.print(v[i]);
      Serial.print('\t');
  }
    Serial.print('\n');
}

void PrintMatrix(const TMatrix<double>& m)
{
  for(int i = 0; i < m.CountRows(); i++)
  {
    for(int j = 0; j < m.CountColumns(); j++)
    {
      Serial.print(m[i][j]);
      Serial.print('\t');
    }
    Serial.print('\n');
  }
}

#define PRESENCE_TOLERANCE 5
#define DEFAULT_THRESHOLD (3.0)

enum LED_STATE {DARK, LIGHT};
enum COLOR {NOT_PRESENT = -1, RED, GREEN, BLUE, WHITE};
enum TCS_MODE {OFF, FULL, FIVE_TO_ONE, TWENTY_TO_ONE};

#define SENSOR_READS 10
#define SENSOR_DELAY 10

#define TRAINING_SAMPLES 10
#define TESTING_SAMPLES 20

class TCS3200
{
  //pins on the TCS3200 breakout:
  int pS0, pS1, pS2, pS3;
  int outPin;
  int ledPin;

  TMatrix<double> rgbInverse;
  SampleMatrix trainingData;
  
  double threshold;

public:
  TCS3200(int s0, int s1, int s2, int s3, int out, int led) :
  rgbInverse(3,3), trainingData(3,TRAINING_SAMPLES)
  {
    pS0 = s0;
    pS1 = s1;
    pS2 = s2;
    pS3 = s3;
    outPin = out;
    ledPin = led;
    
    threshold = DEFAULT_THRESHOLD;
  }

  void Setup(void);
  void CalibrateRGBMatrix(void);
  void CollectTrainingData(void);
  
  COLOR DetectColor(TVector<double>&);
  COLOR ClassifyColor(TVector<double>&);
  void ClassifyColors(void);

private:  
  COLOR ReadRGB(dvector&);
  double ReadColor(COLOR, LED_STATE, TCS_MODE = TWENTY_TO_ONE);
  void SetTCSMode(TCS_MODE);
};

COLOR TCS3200::ReadRGB(dvector& rgb)
{
  rgb.Zero(); //set to black, but check status of return COLOR to check success
  
  //isPresentTolerance will need to be something small if used in high light environment, large if used in dark environment.
  //the color detection will work either way, but the larger isPresentTolerance is, the closer the object will need to be in front of sensor
  double isPresent = ReadColor(WHITE, DARK) / ReadColor(WHITE, LIGHT);//number gets large when something is in front of sensor.
  if(isPresent < PRESENCE_TOLERANCE)
  {
    Serial.println(F("Nothing present."));
    return NOT_PRESENT;
  }

  double white = ReadColor(WHITE, LIGHT);
  double red = ReadColor(RED, LIGHT);
  double green = ReadColor(GREEN, LIGHT);
  double blue = ReadColor(BLUE, LIGHT);

  rgb[RED] = red / white;
  rgb[GREEN] = green / white;
  rgb[BLUE] = blue / white;
  
  Serial.println(F("Red\tGreen\tBlue"));
  Serial.print(rgb[RED]);
  Serial.print('\t');
  Serial.print(rgb[GREEN]);
  Serial.print('\t');
  Serial.println(rgb[BLUE]);

  //return color based on the raw scores (lowest is brightest)
  if(rgb[RED] < rgb[BLUE] && rgb[RED] < rgb[GREEN])
  {
    return RED;
  }
  else if(rgb[BLUE] < rgb[GREEN])
  {
    return BLUE;
  }
  else
  {
    return GREEN;
  }
}

COLOR TCS3200::DetectColor(TVector<double>& rgb)
{
  if(ReadRGB(rgb) != NOT_PRESENT)  
  {
  TVector<double> guess = rgbInverse * rgb;
  Serial.print(guess[RED]);
  Serial.print('\t');
  Serial.print(guess[GREEN]);
  Serial.print('\t');
  Serial.print(guess[BLUE]);
  Serial.print('\n');
  
  if(guess[RED] > guess[BLUE] && guess[RED] > guess[GREEN])
  {
    Serial.println(F("Looks RED to me!"));
    return RED;
  }
  else if(guess[BLUE] > guess[GREEN])
  {
    Serial.println(F("Looks BLUE to me!"));
    return BLUE;
  }
  else
  {
    Serial.println(F("Looks GREEN to me!"));
    return GREEN;
  }
  }
  
  else return NOT_PRESENT;
}

COLOR TCS3200::ClassifyColor(TVector<double>& rgb)
{
  if(ReadRGB(rgb) != NOT_PRESENT)
  {  
    double mahal = trainingData.CalcMahalanobis(rgb);
    Serial.println(mahal);
  
    if(mahal < threshold) 
    {
      Serial.println("It's BLUE!");
      return BLUE;
    }
  
    else
    {
      Serial.println(F("Sorry. It's not blue."));  
      return WHITE;
    }
  }
  
  else return NOT_PRESENT;
}

/*
This method will return the pulseIn reading of the selected color.
 Since frequency is proportional to light intensity of the selected color filter,
 the smaller pulseIn is, the more light there is of the selected color filter.
 It will turn on the sensor at the start taosMode(1), and it will power off the sensor at the end taosMode(0)
 color: 0=white, 1=red, 2=blue, 3=green
 if LEDstate is 0, LED will be off. 1 and the LED will be on.
 taosOutPin is the ouput of the TCS3200. If you have multiple TCS3200, all wires can be combined except the out pin
 */
double TCS3200::ReadColor(COLOR color, LED_STATE LEDstate, TCS_MODE tcsMode)
{
  //make sure that the pin is set to input
  pinMode(outPin, INPUT);

  //turn on sensor with highest frequency setting
  SetTCSMode(tcsMode);
  
  //set the pins to select the color
  if(color == WHITE){
    digitalWrite(pS3, LOW); //S3
    digitalWrite(pS2, HIGH); //S2
  }
  else if(color == RED){//red
    digitalWrite(pS3, LOW); //S3
    digitalWrite(pS2, LOW); //S2
  }
  else if(color == GREEN){//blue
    digitalWrite(pS3, HIGH); //S3
    digitalWrite(pS2, HIGH); //S2
  }
  else if(color == BLUE){//green
    digitalWrite(pS3, HIGH); //S3
    digitalWrite(pS2, LOW); //S2
  }
  
  if(LEDstate == DARK){
    digitalWrite(ledPin, LOW);
  }
  else{
    digitalWrite(ledPin, HIGH);
  }

  delay(SENSOR_DELAY);

  double pulseLength = 0;  
  for(int i = 0; i < SENSOR_READS; i++)
  {
    double readPulse = pulseIn(outPin, LOW, 80000);
    //if the pulseIn times out, it returns 0 and that throws off numbers. just cap it at 80k if it happens
    if(readPulse < .1){
      readPulse = 80000;
    }
    
    pulseLength += readPulse;
  }
  
  pulseLength /= SENSOR_READS;

  //turn off color sensor and white LED to save power
  SetTCSMode(OFF);

  return pulseLength;
}

//setting mode to zero will put taos into low power mode. taosMode(0);
void TCS3200::SetTCSMode(TCS_MODE mode)
{
  if(mode == FULL){
    //this will put in 1:1
    digitalWrite(pS0, HIGH); //S0
    digitalWrite(pS1, HIGH); //S1
    // Serial.println("m1:1m");
  }
  else if(mode == FIVE_TO_ONE){
    //this will put in 1:5
    digitalWrite(pS0, HIGH); //S0
    digitalWrite(pS1, LOW); //S1
    //Serial.println("m1:5m");
  }
  else if(mode == TWENTY_TO_ONE){
    //this will put in 1:50
    digitalWrite(pS0, LOW); //S0
    digitalWrite(pS1, HIGH); //S1
    //Serial.println("m1:50m");
  }
  else{
    //power OFF
    digitalWrite(ledPin, LOW);
    digitalWrite(pS0, LOW); //S0
    digitalWrite(pS1, LOW); //S1
    // Serial.println("mOFFm");
  }
}

void TCS3200::Setup(void){
  //initialize pins
  pinMode(ledPin,OUTPUT); //LED pinD
  
  //color mode selection
  pinMode(pS2,OUTPUT); //S2 pinE
  pinMode(pS3,OUTPUT); //s3 pinF
  
  //color response pin (only actual input from taos)
  //pinMode(out, INPUT); //out pinC
  //communication freq output divider
  pinMode(pS0,OUTPUT); //S0 pinB
  pinMode(pS1,OUTPUT); //S1 pinA
}

void TCS3200::CalibrateRGBMatrix(void)
{
  TMatrix<double> rgbMatrix(3,3);
  TVector<double> rgb(3);
  
  while(DetectColor(rgb) == NOT_PRESENT)
  {
    Serial.println(F("Place RED in front of the sensor."));
    delay(500);
  }

  rgbMatrix.SetColumn(RED, rgb);

  delay(3000);

  while(DetectColor(rgb) == NOT_PRESENT)
  {
    Serial.println(F("Place GREEN in front of the sensor."));
    delay(500);
  }

  rgbMatrix.SetColumn(GREEN, rgb);

  delay(3000);

  while(DetectColor(rgb) == NOT_PRESENT)
  {
    Serial.println(F("Place BLUE in front of the sensor."));
    delay(500);
  }

  rgbMatrix.SetColumn(BLUE, rgb);

  delay(3000);

  Serial.print("\n\n");

  PrintMatrix(rgbMatrix);
  Serial.print("\n\n");
  
  rgbInverse = rgbMatrix.FindInverse(); 
  PrintMatrix(rgbInverse);
  
  Serial.println("Calibration complete.");
  delay(1000);
}

void TCS3200::ClassifyColors(void)
{
  TVector<double> rgb(3);

  threshold = -1;
  Serial.println("\n\nEntering testing mode.");
  while(threshold < 0.1)
  {
    Serial.println(F("\nEnter a threshold value. The lower the threshold, the more selective the program is."));
    Serial.println(F("In general, 5-10 is low; > 50 is high."));

  while (Serial.available() == 0) ;  // Wait here until input buffer has a character
  {
    threshold = Serial.parseFloat();        // new command in 1.0 forward
    
    Serial.print("New threshold = "); 
    Serial.println(threshold);

    while (Serial.available() > 0)  // .parseFloat() can leave non-numeric characters
    { char junk = Serial.read() ; }      // clear the keyboard buffer
  }
  
    if(threshold < 0.1) Serial.println("\nPlease re-enter a threshold value.");
  }

  Serial.println("\nTesting 20 samples.");
  for(int i = 0; i < TESTING_SAMPLES; i++)
  {

    COLOR color = NOT_PRESENT;
    while(color == NOT_PRESENT)
    {
      Serial.print("\nTrial ");
      Serial.print(i+1);
      Serial.println(". Place a sample in front of the sensor and press <return>.");
      while(Serial.read() != '\n'){}
      color = ClassifyColor(rgb);
      delay(500);
    }
  }
}

void TCS3200::CollectTrainingData(void)
{
  TVector<double> rgb(3);
  
  Serial.println("Entering training mode.");
  Serial.println("Please have 10 samples of BLUE ready.");
  //Serial.println("Please have 10 samples of BLUE ready.");

  for(int i = 0; i < TRAINING_SAMPLES; i++)
  {
    COLOR color = NOT_PRESENT;
    while(color == NOT_PRESENT)
    {
      Serial.print("\nTrial ");
      Serial.print(i+1);
      Serial.println(". Place something BLUE in front of the sensor and press <return>.");
      while(Serial.read() != '\n'){}
      color = ReadRGB(rgb);
      delay(500);
    }
    
  trainingData.SetColumn(i, rgb);
  }
  
  Serial.println(F("Calculating stats."));
  
  dvector mean(3);
  dmatrix covar(3, 3);
  
  trainingData.CalcMultiStats(mean, covar); 
 
  Serial.println("Average:");
  PrintVector(mean);
  
  Serial.println("\n Covariance matrix:");
  PrintMatrix(covar); 

  Serial.println("Done.");
}
