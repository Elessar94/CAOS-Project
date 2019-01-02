#inclube <Bounce2.h>
#include <SPI.h> //übertragungsprotokoll, if no more jobs to do, we can replace this with i^2C  
//TODO include eeprom code to store old values. Maybe check monthslater, but improve. 

int tasterPin = 2;
int analogPotValue;
int factor = 16; //Toggle switch for slowing down the input 
int clockSource = 11; 
int switchLED = 7;
int tapmodeLED = 8;
int effectLED = 9; //Check this pin-choice
int tappingLED = 10;
int offPin = 12; //if the effect is off, a pin is needed. TODO check this pinchoice
int analogPot = 14; 
bool switchmode = HIGH;     // HIGH = Switchmode / LOW = Tapmode
bool buttonPressed;
bool effectRunning = LOW; 
bool effectOn = false;
int startTime; //this stores the micros, when the button was started to be pressed.
int currentTime; 
int changeCutoff = 1000000; //this constant stores the time we wait until we switch the mode in micros.  
int tapCutoff = 3;
int timesTapped = 0;
int lastTapTime;
int firstTapTime;
bool tapping = false;  
int maxInterval = 560000; //TODO these two values should be read from eeprom if possible. 
int minInterval = 38000;
long int interval;
int currentTime2;
int nextDimTime;
int blinkTime = 1;
bool stillTapping = false ;
int mappedInterval;
int mappedIntervalDiv;
int intervalDiv;
byte address = 0x11; //TODO change this? What does this mean exactly
bool divisionStatus;  
      
Bounce debouncer = Bounce(); //TODO we have to check what this debouncer does EXACTLY. All debouncing lines have to be reconsidered. 

void switchOnOff();
void switchLoop();
void tapLoop();
void setupSwitch();
void setupTap();
void checkSingles();
void reset();
long int getInterval();
void fireTap();
double getDivision(long int interval);
bool AnalogPotTurned();

SPI.begin(); //TODO check what this method does. 

void setup() {
  /// SETUP SWITCHMODE /// 
  pinMode(tasterPin, INPUT);
  pinMode(analogPot, INPUT);
  pinMode(switchLED, OUTPUT);
  pinMode(tapmodeLED, OUTPUT);
  pinMode(offPin, OUTPUT);
  pinMode(effectLED, OUTPUT);
  pinMode(tappingLED, OUTPUT);
  pinMode(clockSource, OUTPUT);
  pinMode (factor, INPUT);
  debouncer.attach(tasterPin);
  debouncer.interval(25);
  digitalWrite(switchLED, switchmode);
  digitalWrite(tapmodeLED, !switchmode);
  digitalWrite(effectLED, effectRunning);
 

  /// SETUP TAPMODE /// 
  
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
  digitalWrite(switchLED, switchmode);
  digitalWrite(tapmodeLED, !switchmode);
  buttonPressed = false; //avoid switching back and forth
  }
}


void tapLoop(){
  currentTime2 = micros();
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
          nextDimTime = currentTime2 + blinkTime;
          lastTapTime = currentTime2;
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
digitalPotWrite(DigPotValue);

//flashLeds(mappedInterval, mappedIntervalDiv, stillTapping);
//dimLeds(stillTapping);
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
  effectOn = !effectOn;
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
    stillTapping = True; //TODO catch bug when we switch mode while tapping
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
