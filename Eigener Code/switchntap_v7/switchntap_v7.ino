#include <Bounce2.h>
#include <SPI.h> //Übertragungsprotokoll
#include <EEPROMex.h>

///////PINS///////

int tasterPin = 2;
int redLED = 7;
int blueLED = 8;
int effectLED = 9; //Check this pin-choice
int chipSelect = 10; 
int relayPin = 4; //if this is high, the effect is on. 
int analogPot = A3; 
int factor = 6; //Toggle switch for slowing down the input 
int CALOUT = 3;
int CALIN = A5; 

//////Overall variables///////

Bounce debouncer = Bounce(); 
bool switchmode = LOW;     // HIGH = Switchmode / LOW = Tapmode
bool buttonPressed;

int startTime; //this stores the micros, when the button was started to be pressed.
int currentTime; 
int changeCutoff = 1000; //this constant stores the time we wait until we switch the mode in millis
int blinkDuration = 6; //stores how long a blink takes.
int nextDimTime1;

int nextBlinkTime1;
 
int nextSync;

//////Switch mode//////

bool effectRunning = LOW; 


//////Tap mode //////

int currentTapLoopTime;

int tapCutoff = 3;
int timesTapped = 0;
int lastTapTime;
int firstTapTime;
bool tapping = false;  

int maxInterval = 560; 
int minInterval = 38; 
long int delayInterval;
long int intervalWithFactor;
long int intervalWithoutFactor;
int mappedInterval;



int analogPotValue;
byte address = 0x11; 

//int nextDimTime;

void switchLoop();
void tapLoop();
void reset();

void switchOnOff();

void checkSingles();
long int getInterval();
void fireTap();
bool AnalogPotTurned();
void calibrateDelay();
void digitalPotWrite (int value);
int considerFactor();

void calculateFactorValue();
/////////////////////////////////////////////////////////////

void setup() {
  
  //////SETUP Overall////// 
  
  Serial.begin(9600);
  SPI.begin();
  
  pinMode(tasterPin, INPUT);
  debouncer.attach(tasterPin);
  debouncer.interval(25);
  
  if (digitalRead(tasterPin)){ //if button is pressed during the start, we calibrate the device.
    calibrateDelay();
  }
  
  if (EEPROM.readLong(0)==1){ //if something is stored in the eeprom, we read these values. 
    minInterval = EEPROM.readLong(4);
    maxInterval = EEPROM.readLong(8);
  }
  
  pinMode(redLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  
  digitalWrite(blueLED, switchmode);
  digitalWrite(redLED, !switchmode);
  
  pinMode(CALOUT, OUTPUT); //calibration pins 
  pinMode(CALIN, INPUT);
  
  pinMode (factor, INPUT); //toggleswitch pin
  
  //////SETUP Switchmode//////
  
  pinMode(effectLED, OUTPUT);
  digitalWrite(effectLED, effectRunning);
  
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, effectRunning);
  
  pinMode(analogPot, INPUT);
  
  //////SETUP Tapmode//////
  
  pinMode(chipSelect, OUTPUT); //belonging to the digitalPotentiometer
}

void loop() {
  
  debouncer.update();
  currentTime = millis();
  
  // change between modes 
  if (buttonPressed && (currentTime - startTime >= changeCutoff)) { 
    switchmode = !switchmode;
    buttonPressed = false; //avoid switching back and forth
    digitalWrite(blueLED, switchmode);
    digitalWrite(redLED, !switchmode);
    reset();
  }
  
  if (switchmode == HIGH) {   // Switchmode ON
    switchLoop();
  }
  if (switchmode == LOW){ //Tapmode activated
    tapLoop(); 
  }
  
  calculateFactorValue(); //factor stuff
  delayInterval = considerFactor();
  
  mappedInterval = map(delayInterval, minInterval, maxInterval, 0,255); //send interval to the digPot and blink the LEDs
  digitalPotWrite(mappedInterval);
  flashLEDs(delayInterval);
  dimLEDs();

}


void tapLoop(){
    
    currentTapLoopTime = millis();
    checkSingles();
    
    if(timesTapped >= tapCutoff){
      intervalWithoutFactor = getInterval();
      
    } else if(AnalogPotTurned())
      {
        intervalWithoutFactor = map(analogPotValue, 0, 1023, minInterval, maxInterval);
             
    }
    if (debouncer.rose() ) {  // Button is pressed 
    startTime = currentTime;
    buttonPressed = true;
    }

    if (debouncer.fell()) { // Button is released, count this as Tap.
     buttonPressed = false;
     if (currentTime - startTime < changeCutoff) {
     //     digitalWrite(tappingLED, HIGH);
          //nextDimTime = currentTapLoopTime + blinkTime;
          lastTapTime = currentTapLoopTime;
          fireTap();
      }  
    } 
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
  effectRunning = !effectRunning;
  digitalWrite(effectLED, effectRunning);
  digitalWrite(relayPin, effectRunning);
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
  long int toReturn = (lastTapTime - firstTapTime)/(timesTapped -1);

  if (toReturn > maxInterval) { //in the following two if statements we commit to count time in the tapping methods in millis
    toReturn = maxInterval; //toReturn too high 
  }
  if (toReturn < minInterval) {
    toReturn = minInterval; //toReturn too low 
  }
  reset();
  return toReturn;
}
void fireTap(){
  if (timesTapped == 0) {
    firstTapTime = millis();
    tapping = true; 
    }
    timesTapped++; 
}

bool AnalogPotTurned(){
  int current = analogRead(analogPot);
  if (current > analogPotValue + 3 || current < analogPotValue -3){ 
     analogPotValue = current;
     return true; 
  }
  return false;
}

void flashLEDs(long int delayInterval){

  if (tapping){
    return; 
  }
  if (currentTime >= nextSync){
      nextSync = currentTime + delayInterval*6;
      nextBlinkTime1 = currentTime;  
  }
  if (currentTime >= nextBlinkTime1)
    {      
     digitalWrite(blueLED, switchmode);
     digitalWrite(redLED, !switchmode);
     nextDimTime1 = currentTime + blinkDuration;
     nextBlinkTime1 = currentTime + delayInterval;

    }
  if (currentTime > nextBlinkTime1){
     digitalWrite(blueLED, switchmode);
     digitalWrite(redLED, !switchmode);
     nextDimTime1 = currentTime + blinkDuration;
     nextBlinkTime1 = currentTime + delayInterval;
  }
  return; 
 
 }
 

 void dimLEDs(){
  if (tapping){
     digitalWrite(redLED, LOW);
     digitalWrite(blueLED, LOW);
     return;
  }
  if (currentTime >= nextDimTime1){
     digitalWrite(redLED, LOW);
     digitalWrite(blueLED, LOW);
     nextDimTime1 = nextBlinkTime1+1; //this so the blink happens first and the DimTime gets overwritten in the flashLED method
  }
 return;
 }

void digitalPotWrite(int value){
  digitalWrite(chipSelect, LOW);
  SPI.transfer(address);
  SPI.transfer(value);
  digitalWrite(chipSelect, HIGH);

}

void calibrateDelay(){
    digitalPotWrite(255);
    digitalWrite(CALOUT, HIGH);
    startTime = micros();
    delay(5);
    digitalWrite(CALOUT, LOW);  
    do{
      Serial.println(analogRead(CALIN));
    }
    while (analogRead(CALIN) < 600);
    Serial.println("Survived While-Loop");
    long int maxDiff = micros() - startTime;

    delay(1000);
    
    digitalPotWrite(0);
    digitalWrite(CALOUT, HIGH);
    long int startTime = micros();
    delay(5); //in milli seconds 
    
    digitalWrite(CALOUT, LOW);
    do{
      Serial.println(analogRead(CALIN));
    }
    while (analogRead(CALIN) < 600);
      
    long int minDiff = micros() - startTime;    


       
    EEPROM.writeLong(1, 0);
    Serial.println("Sent confirmation to EEPROM");
    EEPROM.writeLong(4, minDiff); 
    Serial.println("Sent following minvalue to EEPROM:"); 
    Serial.println(minDiff);
    EEPROM.writeLong(8, maxDiff);
    Serial.println("Sent following maxvalue to EEPROM:");
    Serial.println(maxDiff);
}
int considerFactor(){

  if (digitalRead(factor)){ 
    return intervalWithFactor;
  }

  return intervalWithoutFactor;
}

void calculateFactorValue(){
  intervalWithFactor = 0.5*intervalWithoutFactor;
  if (intervalWithFactor>maxInterval){
    intervalWithFactor = maxInterval;
  }
  if (intervalWithFactor<minInterval){
    intervalWithFactor = minInterval;
  }
}
  
