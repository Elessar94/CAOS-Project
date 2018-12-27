/*
this program taken from arduino Example .
modified by By Mohannad Rawashdeh
http://www.genotronex.com
http://www.instructables.com/

This code used to control the digital potentiometer
MCP41100 connected to  arduino Board
CS >>> D10
SCLK >> D13
DI  >>> D11
PA0 TO VCC
PBO TO GND
PW0 TO led with resistor 100ohm .
*/ 
#include <SPI.h> //übertragungsprotokoll, if no more jobs to do, we can replace this with i^2C  
#include <Arduino.h>
#include <EEPROM.h>

#define CALOUT 3            //part of calibration extension
#define CALTAP 7            //part of calibration extension
#define LED2 8              //LED for the actual Delay Time TODO aditionally use the two colors to show whether switch or tap mode  
#define LED1 9              //LED for tapped Quarter Notes  TODO change this to show if the effect is activated or not. 

#define TAPPIN 12           //Tap Button
#define ANALOGPOT 14        //Delay Potentiometer
#define DIVISION1 16       //Toggle switch for Quarter, Eight or Sixteen
#define DIVISION2 17       //Toggle switch for pure Note, dotted Note or Triplet TODO cut this toggle switch 
#define MODULATIONPIN 18    //Modulation Input TODO cut this modulation input
#define CALIN 19            //part of calibration extension
#define CS 10 //clock source for SPI 

   



#define MINTIME 38000      //Ideal Delay Time with 1k Resistance, values in microseconds (alternative to calibration)
#define MAXTIME 560000     //Ideal Delay Time with 51k Resistance

//#define RANGE 50000         //unused 
#define MINTAPS 3           //Minimum of Taps required to set the Interval.
#define BLINKDURATION 1
#define MAXMODULATION 30 //TODO cut as we cut modulation part 
#define MINMODULATION -30   //TODO cut as we cut modulation part 



///////////VARIABLES/////////////////////////
byte address = 0x11;
int i=0;
int LastDigPotValue = 0;
int DigPotValue = 0;

long int now;

long int interval;
long int intervalDiv;

int mappedInterval;
int mappedIntervalDiv;

int analogPotCurrVal;
int analogPotLastVal;

long int minInterval;
long int maxInterval;

int timesTapped;
int stillTapping;
long int firstTapTime;
long int lastTapTime;
int tapState;
int lastTapState;

int calState;
int lastCalState;

int interrupt;

long int nextSync;
long int nextBlinkTime1;
long int nextBlinkTime2;
long int nextDimTime1;
long int nextDimTime2;
int divDeact;

/////////////////////////Functions/////////////////

void timeOut();
long int getInterval();
int digitalPotWrite(int value);
int mapInterval(long int interval);
int AnalogPotTurned();
int AnalogPotGiveValue();
void flashLeds(int, int, int);
void dimLeds(int);
int TapButtonPressed();
void firstPress();
int getModulation();
int CalButtonPressed();
void calibration();


void eepromWriteLong(long, int);
long eepromReadLong(int);


 

////////////////////////////////////SETUP////////////////////////////////////

void setup()

{


////////////////////////////////////DEBUG////////////////////////////////////
//Serial.begin(9600);

////////////////////////////////////PIN SETUP////////////////////////////////////


pinMode (CS, OUTPUT); //TODO check why the CS-pin is defined while others are not. 
pinMode (ANALOGPOT, INPUT);
pinMode (TAPPIN, INPUT);
pinMode (MODULATIONPIN, INPUT);
pinMode (DIVISION1, INPUT);
pinMode (LED1, OUTPUT);
pinMode (LED2, OUTPUT);
pinMode (DIVISION1, INPUT);
pinMode (DIVISION2, INPUT);
pinMode (CALIN, INPUT);
pinMode (CALOUT, OUTPUT);
pinMode (CALTAP, INPUT);



//Set unused Pins as Output and set them to low.
/*pinMode (1, OUTPUT);
pinMode (2, OUTPUT);
pinMode (4, OUTPUT);
pinMode (5, OUTPUT);
pinMode (6, OUTPUT);
pinMode (15, OUTPUT);
digitalWrite(1, LOW);
digitalWrite(3, LOW);
digitalWrite(2, LOW);
digitalWrite(4, LOW);
digitalWrite(5, LOW);
digitalWrite(6, LOW);
digitalWrite(15, LOW);*/


SPI.begin();

//Read EEPROM if the Arduino was calibrated to the excat Delay time provided by the P2399 and the MCP41050 

if(eepromReadLong(0)== 42){
  minInterval = eepromReadLong(4); 
  maxInterval = eepromReadLong(8);
  /*
  Serial.print("MIN: ");
  Serial.println(minInterval);
  Serial.print("MAX: ");
  Serial.println(maxInterval);
  */
}else { //TODO we will work in the beginning via this else method, as we will not calibrate in the beginning. 
  minInterval = MINTIME;
  maxInterval = MAXTIME;
}

reset();
analogPotCurrVal = analogRead(ANALOGPOT);
analogPotLastVal = analogPotCurrVal;
nextSync = micros();
//Serial.println(analogPotCurrVal);

interval = map(analogPotCurrVal, 0, 1023, minInterval, maxInterval); //value gets projected onto different scale. 0-1023 is new scale, minInterval-maxInterval is old scale
divDeact = 1;

}


////////////////////////////////////MAINLOOP///////////////////////////////////////////////////////


void loop()
  {
    now = micros();
    timeOut();
    if(timesTapped >= MINTAPS){
      interval = getInterval();
      divDeact = 0;
    }else if(AnalogPotTurned())
    {
      interval = map(analogPotCurrVal, 0, 1023, minInterval, maxInterval);
      divDeact = 1;      
    }
    if (TapButtonPressed())
    {
      digitalWrite(LED1, HIGH);
      nextDimTime1 = now + BLINKDURATION;
      firstPress();

    lastTapTime = now;
    }

if (CalButtonPressed())
{
  calibration();
}

if (divDeact == 0){
intervalDiv = (long int)((interval * (getDivision(interval))+0.5));
}else if(divDeact == 1){
  intervalDiv = interval;
}

mappedInterval = map(interval, minInterval, maxInterval, 0, 255);
mappedIntervalDiv = map(intervalDiv, minInterval, maxInterval, 0,255);

DigPotValue = mappedIntervalDiv;

digitalPotWrite(DigPotValue);

flashLeds(mappedInterval, mappedIntervalDiv, stillTapping);
dimLeds(stillTapping);
}


////////////////////////////////////FUNCTIONS//////////////////////////////////////////

////////////////////////////////7
void reset()
{
  timesTapped = 0;
  stillTapping = 0;
}

///////////////////////////////////

void calibration(){ //TODO try to include this in code, but prioritize functioning prototype.
    digitalPotWrite(0);
    digitalWrite(CALOUT, HIGH);
    long int startTime = micros();
    delay(5);
    digitalWrite(CALOUT, LOW);
    do
    {
      /*Serial.println(analogRead(CALIN));
      Serial.println(micros() - startTime);
      */}while (analogRead(CALIN) < 600);
    long int minDiff = micros() - startTime;
    
    delay(1000);
    digitalPotWrite(255);
         
    digitalWrite(CALOUT, HIGH);
    startTime = micros();
    delay(5);
    digitalWrite(CALOUT, LOW);
    do
    {
      //Serial.println(analogRead(CALIN));
      }while (analogRead(CALIN) < 600);
    long int maxDiff = micros() - startTime;

    
    eepromWriteLong(42, 0);
    eepromWriteLong(minDiff, 4); 
    eepromWriteLong(maxDiff, 8);

  /*Serial.print("42?: ");
    Serial.println(eepromReadLong(0));
    Serial.println(minDiff);
    Serial.println(eepromReadLong(4));
    Serial.println(maxDiff);
    Serial.println(eepromReadLong(8));*/
}

/////////////////////////////////////////////////////////////////////

void eepromWriteLong(long lo, int adr) {
// long Wert in das EEPROM schreiben  
// Eingabe : adr Speicherplatz 
// Eingabe : lo Zahl, Wertebereich -2.147.483.648 bis 2.147.483.647
//
// Matthias Busse 23.5.2014 Version 1.0

byte by;

  for(int i=0;i< 4;i++) {
    by = (lo >> ((3-i)*8)) & 0x000000ff; 
    EEPROM.write(adr+i, by);
  }
} // eepromWriteLong

//////////////////////////////////////////////////////////////////////////////

long eepromReadLong(int adr) {
// long int Wert aus 4 Byte EEPROM lesen
// Eingabe : adr bis adr+3
// Ausgabe : long Wert
// 
// Matthias Busse 23.5.2014 Version 1.0

long lo=0;

  for(int i=0;i< 3;i++){
    lo += EEPROM.read(adr+i);
    lo = lo << 8;
  }
  lo += EEPROM.read(adr+3);
  return lo;
} // eepromReadLong


//////////////////////////////////////////////////////////////////////////////


double getDivision(long int interval) //TODO change this to multiplication 
{
  int currentDiv1; //TODO Div1 is 1/4-1/16
  int currentDiv2; //TODO cut this
  int lastDiv1;
  int lastDiv2;
  float division1;
  float division2;
  float divisionGes;

  currentDiv1 = analogRead(DIVISION1);
  currentDiv2 = analogRead(DIVISION2);
  
    if (currentDiv1 > 600){
      division1 = 1;

    }
    if(currentDiv1 < 100){
      division1 = 0.5;
    }
    if(currentDiv1 > 100 && currentDiv1 < 700){
      division1 = 0.25;
    }
         
    if (currentDiv2 > 700){
      division2 = 1.5;
    }
    if(currentDiv2 < 100){
      division2 = 1;
    }
    if(currentDiv2 > 100 && currentDiv2 < 700){
      
      division2 = 0.666;
    }
    


  lastDiv1 = currentDiv1;
  lastDiv2 = currentDiv2;
  divisionGes = division1 * division2;

  return divisionGes;
  }
//////////////////////////////////////////////////////////////7

int getModulation()//TODO cut 

{
    int modInput = analogRead(MODULATIONPIN);
    
    if (modInput > 600){modInput = 600;}
    if (modInput < 430) { modInput = 430;}
    return map(modInput, 430,595, MINMODULATION, MAXMODULATION);
  
}

///////////////////////////////////////////

int digitalPotWrite(int value)
{
  if (value > 255){value = 255;}else if (value < 0){value = 0;}
  if (value != LastDigPotValue)
  { 
    
    digitalWrite(CS, LOW);
    SPI.transfer(address);
    SPI.transfer(value);
    digitalWrite(CS, HIGH);
    LastDigPotValue = value;
  }
  
}
//////////////////////////////////////////////////////

void timeOut()
{
  if(timesTapped > 0 && (now - lastTapTime) > maxInterval*1.5)
  { //Check to prevent SingleTaps
  reset();
 
  }
}

//////////////////////////

long int getInterval()//TODO maybe add method to exclude unfitting taps. 
{
  long int avgTapInterval = (lastTapTime - firstTapTime) / (timesTapped - 1);

  if (avgTapInterval > maxInterval + 10000){
    avgTapInterval = maxInterval;
    }
  if (avgTapInterval < minInterval - 10000){
    avgTapInterval = minInterval;
    }
  return avgTapInterval;
}

//////////////////////////////

int AnalogPotTurned()
{
  analogPotCurrVal = analogRead(ANALOGPOT);
  if(analogPotCurrVal > analogPotLastVal + 3 || analogPotCurrVal < analogPotLastVal - 3 ) //debounce
  {
    analogPotLastVal = analogPotCurrVal;

    return 1;
  } else return 0;
}

///////////////////////////////////

int TapButtonPressed() //TODO change this method. Include library bounce2
  {

    tapState = digitalRead(TAPPIN);
    if(tapState == HIGH && now - lastTapTime > 30000 && tapState != lastTapState ){ //Debounce
      lastTapState = tapState;
      return 1;
    }
    lastTapState = tapState;
    return 0;
  }


///////////////////////////////////
int CalButtonPressed() //belongs to calibrate-package
  {

    calState = digitalRead(CALTAP);
    if(calState == HIGH && now - lastTapTime > 30000 && calState != lastCalState ){ //Debounce
      
      lastCalState = calState;
      return 1;
    }
    lastCalState = calState;
    return 0;
  }
////////////////////////////////////
void firstPress()
{

  if(timesTapped == 0)
  {
    firstTapTime = micros();
    stillTapping = 1;
  }
  timesTapped++;
}

/////////////////////////////// TODO check this method as it has to be to complicated. Include coloring via state.
void flashLeds(int intQuarter, int intDiv, int interrupt)
{
  long int interval = map(intQuarter, 0, 255, minInterval, maxInterval);
  long int intervalDiv = map(intDiv, 0, 255, minInterval, maxInterval);
  if (interrupt == 0)
  {
    if (now >= nextSync)
    {
      nextSync = now + interval*6;
      nextBlinkTime2 = now;
      nextBlinkTime1 = now;      
    }
    if (now >= nextBlinkTime1)
    {      
      digitalWrite(LED1, HIGH);
      nextDimTime1 = now + BLINKDURATION;
      nextBlinkTime1 = now + interval;
    }
    if (now >= nextBlinkTime2)
    {
      digitalWrite(LED2, HIGH);
      nextDimTime2 = now + BLINKDURATION;
      nextBlinkTime2 = now + intervalDiv;
    }   
  }
  return;
}
///////////////////////////////

void dimLeds(int interrupt)
{
  if (interrupt == 0)
  {
    if (now >= nextDimTime1)
    {
      digitalWrite(LED1, LOW);
         
    }
    if (now >= nextDimTime2)
    {
      digitalWrite(LED2, LOW);
     }
  }else if(interrupt == 1)
  {
  if (now >= nextDimTime1){
    digitalWrite(LED1, LOW);
  }
    digitalWrite(LED2, LOW);
  }
  return;
}

