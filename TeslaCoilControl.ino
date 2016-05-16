#include <Adafruit_NeoPixel.h>
#define BUTTON_PIN 3
#define RING_PIN 16
#define PIXELCOUNT 16
#define SSR_PIN 13

Adafruit_NeoPixel ring = Adafruit_NeoPixel (PIXELCOUNT, RING_PIN, NEO_GRB + NEO_KHZ800);

uint8_t lockout = 0;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long timeHeldOn = 0;
volatile uint8_t buttonState = 0;

int maxTimeOn = 8000; //max time coil can be energized for, in MS (makes math easier if divisble by 16)
int timeTilOff = 0;    // increases cool down time as coil is on for longer

void setup() {
pinMode(BUTTON_PIN, INPUT_PULLUP);
//attachInterrupt(0, coilButtonOn, RISING); 
attachInterrupt(0, coilButtonOff, FALLING);
pinMode(SSR_PIN, OUTPUT);
digitalWrite(SSR_PIN, LOW);

Serial.begin(9600);

ring.begin();
ring.show();

}

void loop() {
if(!lockout){
Serial.println("System Ready");
Serial.println(buttonState);
Serial.println(lockout);
}
buttonState = digitalRead(BUTTON_PIN);

if(buttonState && !lockout){
    digitalWrite(SSR_PIN, HIGH); // energizing the coil circuit
	Serial.println("Firing coil");
	timeHeldOn = (currentMillis - previousMillis); // does this need a multiplier?
	timerLockControl();
    // loop animation
}
if(lockout){
	digitalWrite(SSR_PIN, LOW);
	Serial.println("System locked out");
	timerUnlockControl();
	}
}
void recharge(){} // animation and recharge timer thing

void timerLockControl() {
currentMillis = millis();
  if (timeHeldOn >= maxTimeOn ) {
    previousMillis = currentMillis;
    lockout = 1; // time is locked out, coil cannot be fired
    }
}
void timerUnlockControl() {
    currentMillis = millis();
    if (currentMillis - previousMillis >= timeHeldOn ) {
      previousMillis = currentMillis;  
      lockout = 0; // unlock the coil to fire again
	  timeHeldOn = 0; // reset hold timer
}
}




//Interrupt code below
//void coilButtonOn() {
//  buttonState = digitalRead(BUTTON_PIN);
//  }

void coilButtonOff() {
  digitalWrite(SSR_PIN, LOW);
  Serial.println ("button released interrupt ***************************");
  previousMillis = currentMillis;
  lockout = 1; // locks coil from being re-fired once button is released
  //recharge(); 
  }
