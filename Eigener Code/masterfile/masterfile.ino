#include <Bounce2.h>
#include <SPI.h> //übertragungsprotokoll, if no more jobs to do, we can replace this with i^2C  

//TODO include eeprom code to store old values. Maybe check monthslater, but improve. 
//TODO add that LED blinks in delay speed. 

///////PINS///////

int tasterPin = 2;
int redLED = 7;
int blueLED = 8;
int effectLED = 9; //Check this pin-choice
int tappingLED = 10;
int clockSource = 11; 
int offPin = 12; //if the effect is off, a pin is needed. TODO check this pinchoice
int analogPot = 14; 
int factor = 16; //Toggle switch for slowing down the input 

//////Overall variables///////
SPI.begin();
Bounce debouncer = Bounce(); //TODO we have to check what this debouncer does EXACTLY. All debouncing lines have to be reconsidered. 
bool switchmode = HIGH;     // HIGH = Switchmode / LOW = Tapmode
bool buttonPressed;

int startTime; //this stores the micros, when the button was started to be pressed.
int currentTime; 
int changeCutoff = 1000000; //this constant stores the time we wait until we switch the mode in micros.  

//////Switch mode//////

bool effectRunning = LOW; 


//////Tap mode //////

int currentTapLoopTime;

int tapCutoff = 3;
int timesTapped = 0;
int lastTapTime;
int firstTapTime;
bool tapping = false;  

int maxInterval = 560000; //TODO these two values should be read from eeprom if possible. 
int minInterval = 38000;
long int interval;
int mappedInterval;
int mappedIntervalDiv;
int intervalDiv;
bool divisionStatus; //TODO there is no way all these variables are necessary. Find better/cheaper solution
 
int analogPotValue;
byte address = 0x11; //TODO change this? What does this mean exactly

//int nextDimTime;
//int blinkTime = 1; //TODO depending on how we implement the flashing of the LEDs this might be unnecessary
      

//////Functions//////

void switchLoop();
void tapLoop();
void reset();

void switchOnOff();

void checkSingles();
long int getInterval();
void fireTap();
double getDivision(long int interval);
bool AnalogPotTurned();

/////////////////////////////////////////////////////////////

void setup() {

  //////SETUP Overall////// 
  
  pinMode(tasterPin, INPUT);
  debouncer.attach(tasterPin);
  debouncer.interval(25);
  
  pinMode(redLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  digitalWrite(redLED, switchmode);
  digitalWrite(blueLED, !switchmode);
  
  //////SETUP Switchmode//////
  
  pinMode(effectLED, OUTPUT);
  digitalWrite(effectLED, effectRunning);
  pinMode(offPin, OUTPUT);
  pinMode(analogPot, INPUT);
  
  //////SETUP Tapmode//////
  
  pinMode(tappingLED, OUTPUT);  
  pinMode(clockSource, OUTPUT); //belonging to the digitalPotentiometer
  pinMode (factor, INPUT);
    
}

void loop() {
  debouncer.update();
  currentTime = micros();

if (switchmode == HIGH) {   // Switchmode ON
  switchLoop();
  }
if (switchmode == LOW){ //Tapmode activated
  tapLoop(); 
  }
  
// change between modes 
if (buttonPressed && (currentTime - startTime >= changeCutoff)) { 
  switchmode = !switchmode;
  digitalWrite(redLED, switchmode);
  digitalWrite(blueLED, !switchmode);
  buttonPressed = false; //avoid switching back and forth
  }
}


void tapLoop(){
  currentTapLoopTime = micros();
    checkSingles();
    if(timesTapped >= tapCutoff){
      interval = getInterval();
      divisionStatus = false; //This variable stores whether division is activated or not. 
    } else if(AnalogPotTurned())
      {
        interval = map(analogPotCurrVal, 0, 1023, minInterval, maxInterval);
        divionStatus = true;      
    }
    
    if (debouncer.rose() ) {  // Button is pressed 
    startTime = currentTime;
    buttonPressed = true;
    }
    if (debouncer.fell()) { // Button is released, count this as Tap.
     buttonPressed = false;
     if (currentTime - startTime < changeCutoff) {
          digitalWrite(tappingLED, HIGH);
          //nextDimTime = currentTapLoopTime + blinkTime;
          lastTapTime = currentTapLoopTime;
          fireTap();
      }  
    } //TODO add further calibration methods, maybe look into monthslater example 

    if (!divisionStatus){
      intervalDiv = (long int)((interval / (getDivision(interval))+0.5)); //why + 0.5? 
      }else if(divisionStatus == true){
        intervalDiv = interval;
    }

mappedInterval = map(interval, minInterval, maxInterval, 0, 255); //Rescale values so they are writeable to the digPot
mappedIntervalDiv = map(intervalDiv, minInterval, maxInterval, 0,255); //same as above 

digitalWrite(clockSource, LOW);
SPI.transfer(address);
SPI.transfer(mappedIntervalDiv);
digitalWrite(clockSourse, HIGH);

//flashLeds(mappedInterval, mappedIntervalDiv, tapping);
//dimLeds(tapping);
}
void switchLoop(){
    if (debouncer.rose() ) {  // Button is pressed 
    startTime = currentTime;
    buttonPressed = true;
    }
    if (debouncer.fell()) { // Button is released
     buttonPressed = false;
     if (currentTime - startTime < changeCutoff) {
       switchOnOff();
      }
      
    }
}
void switchOnOff(){
  //TODO tell system to switch to the off pin, also consider this in the hardware situation. 
  effectRunning = !effectRunning;
  digitalWrite(effectLED, effectRunning);
  // do something with offPin
}

void checkSingles(){
  if (timesTapped > 0 && (currentTime - lastTapTime) > maxInterval * 1.5 ) {
    reset();   
  }
}

void reset(){
  timesTapped = 0; 
  tapping = false; 
}

long int getInterval() { //TODO maybe add method to exclude outlier taps
  long int toReturn = (lastTaptime - firstTapTime)/(timesTapped -1);
  if (toReturn > maxInterval + 10000) { //in the following two if statements we commit to count time in the tapping methods in micros 
    toReturn = maxInterval; //toReturn too high 
  }
  if (toReturn < minInterval - 10000) {
    toReturn = minInterval; //toReturn too low 
  }
  return toReturn;
}
void fireTap(){
  if (timesTapped == 0) {
    firstTapTime = micros();
    tapping = true; //TODO catch bug when we switch mode while tapping
    }
    timesTapped++; 
}
double getDivision(long int interval){
  int toReturn;
  int factor = analogRead(value);
  if (value > 600){
    toReturn = 1; 
  }
  if (value<100){
    toReturn = 2; 
  }
  if (value>100 && value < 700){ //this overrides values 600 - 700 from the above if condition. TODO which one should be changed. 
    toReturn = 2/3;
  }
  return toReturn;
}
bool AnalogPotTurned(){
  int current = analogRead(analogPot);
  if (current > analogPotValue + 3 || current < analogPotValue + -3){ // TODO debounce this more elegantly
     analogPotValue = current;
     return true; 
  }
  return false;
}
