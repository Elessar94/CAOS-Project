#inclube <Bounce2.h>
int tasterPin = 2; 
int switchLED = 7;
int tapmodeLED = 8;
int effectLED = 9; //Check this pin-choice
int tappingLED = 10;
bool switchmode = HIGH;     // HIGH = Switchmode / LOW = Tapmode
bool buttonPressed;
bool effectRunning = LOW; 
int startTime; //this stores the micros, when the button was started to be pressed.
int currentTime; 
int changeCutoff = 1000000; //this constant stores the time we wait until we switch the mode in micros.  
int tapCutoff = 3;
int timesTapped = 0;
int lastTapTime;
int firstTapTime;
bool tapping = false;  
int maxInterval;
int minInterval;
long int interval;
int currentTime2;
int nextDimTime;
int blinkTime = 1;
bool stillTapping = false ;
int mappedInterval;
int mappedIntervalDiv;
int intervalDiv;      
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

void setup() {
  /// SETUP SWITCHMODE /// 
  pinMode(tasterPin, INPUT);
  pinMode(switchLED, OUTPUT);
  pinMode(tapmodeLED, OUTPUT);
  pinMode(effectLED, OUTPUT);
  pinMode(tappingLED, OUTPUT);
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
      //TODO add divDeact code, divDeact = 0;
    }//TODO add AnalogPot Code , else if(AnalogPotTurned())
    //{
     // interval = map(analogPotCurrVal, 0, 1023, minInterval, maxInterval);
     // divDeact = 1;      
    //}
    
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

/*if (divDeact == 0){
intervalDiv = (long int)((interval * (getDivision(interval))+0.5));
}else if(divDeact == 1){
  intervalDiv = interval;
}*/

mappedInterval = map(interval, minInterval, maxInterval, 0, 255); //TODO figure out we these values need to be rescaled 
mappedIntervalDiv = map(intervalDiv, minInterval, maxInterval, 0,255); //same as above 

DigPotValue = mappedIntervalDiv;

digitalPotWrite(DigPotValue);

flashLeds(mappedInterval, mappedIntervalDiv, stillTapping);
dimLeds(stillTapping);
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
  //TODO: This method should switch the effect on/off. the effectRunning variable stores the current status, the effectLED should be on HIGH when effect activated. 
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
