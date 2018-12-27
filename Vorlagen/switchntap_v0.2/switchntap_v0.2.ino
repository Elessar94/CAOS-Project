/* *S*W*I*C*H*N*T*A*P*
    v 0.2 Date: 19.10.18
    Zusätzlich zur Version 0.1 wurde die Variable switchmode eingefügt. Bei HIGH sind wir im Switchmode, bei Low im Tapmode.
    Im Switchmode wird die LED am ledPin1 bei kurzen Tastimpulsen getoggelt.
    Im Tapmode (switchmode LOW) wird die LED ledPin3 bei kurzen Tastimpulsen getoggelt.*/



#include <Bounce2.h>

int tasterPin = 2;          // Pin für den Taster, kaum zu glauben
int ledPin1 = 8;            // LED für kurze Tastenimpulse im Switchmode
int ledPin2 = 7;            // LED zeigt Switchmode (On) oder Tapmode (Off)
int ledPin3 = 13;           // LED für kurze Tastenimpulse im Tapmode
int aktMillis;
int altMillis;
int langMillis = 1000;
bool status;
bool switchmode = HIGH;     // HIGH = Switchmode / LOW = Tapmode
bool led1Status = LOW;      // Kurze Tastenimpulse Switchmode
bool led2Status = HIGH;     // Switchmode = On / Tapmode = Off
bool led3Status = LOW;      // Kurze Tastenimpulse im Tapmode


Bounce debouncer = Bounce();

void setup() {
  //Serial.begin(9600);
  //Serial.println("Anfang");
  pinMode(tasterPin, INPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  debouncer.attach(tasterPin);
  debouncer.interval(25);

  digitalWrite(ledPin1, led1Status);
  digitalWrite(ledPin2, led2Status);
}

void loop() {
  debouncer.update();
  aktMillis = millis();

/////////// SWITCHMODE ///////////
  
  if (switchmode == HIGH) {   // Switchmode ON
    if (debouncer.rose() ) {  // Taste ist gedrückt
      altMillis = aktMillis;
      status = true;
    }
    if (debouncer.fell() ) {  // Taste wieder losgelassen
      status = false;
      if (aktMillis - altMillis < langMillis) {
        led1Status = !led1Status;
        digitalWrite(ledPin1, led1Status);
       }
    }
  }

/////////// TAPMODE ///////////
  if (switchmode == LOW) {     // Switchmode OFF = Tapmode
    if (debouncer.rose() ) {  // Taste ist gedrückt
      altMillis = aktMillis;
      status = true;
    }
    if (debouncer.fell() ) {  // Taste wieder losgelassen
      status = false;
      if (aktMillis - altMillis < langMillis) {
        led3Status = !led3Status;
        digitalWrite(ledPin3, led3Status);
        }
      }
    }   
    if (status && (aktMillis - altMillis >= langMillis)) {
      status = false;
      switchmode = !switchmode;
      led2Status = !led2Status;
      digitalWrite(ledPin2, led2Status);
    }
  }
