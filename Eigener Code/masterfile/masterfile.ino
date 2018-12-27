#inclube <Bounce2.h>
int tasterPin = 2; 
int switchLED = 7;
int tapLED = 8;
int effectLED = 9; //Check this pin-choice
bool switchmode = HIGH;     // HIGH = Switchmode / LOW = Tapmode
bool buttonPressed;
bool effectRunning = LOW; 
int startTime; //this stores the milis, when the button was started to be pressed.
int currentTime; 
int changeCutoff = 1000; //this constant stores the time we wait until we switch the mode in milis.  
Bounce debouncer = Bounce(); //TODO we have to check what this debouncer does EXACTLY. All debouncing lines have to be reconsidered. 

void switchOnOff();
void switchLoop();
void tapLoop();

void setup() {
  pinMode(tasterPin, INPUT);
  pinMode(switchLED, OUTPUT);
  pinMode(tapLED, OUTPUT);
  pinMode(effectLED, OUTPUT);
  debouncer.attach(tasterPin);
  debouncer.interval(25);
  digitalWrite(switchLED, switchmode);
  digitalWrite(tapLED, !switchmode);
  digitalWrite(effectLED, effectRunning);
}

void loop() {
  debouncer.update();
  currentTime = millis();

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
  digitalWrite(tapLED, !switchmode);
  buttonPressed = false; //avoid switching back and forth
  }
}


void tapLoop(){
  //TODO copy monthslater code in here; 
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
