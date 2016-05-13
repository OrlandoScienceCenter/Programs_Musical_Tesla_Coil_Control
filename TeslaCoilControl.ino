#include <Adafruit_NeoPixel.h>
#define BUTTON_PIN 2
#define RING_PIN 16
#define PIXELCOUNT 16
#define SSR_PIN 4

Adafruit_NeoPixel ring = Adafruit_NeoPixel (PIXELCOUNT, RING_PIN, NEO_GRB + NEO_KHZ800);

uint8_t timeLockout = 0;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
volatile uint8_t buttonState = 0;

int maxTimeOn = 8000; //max time coil can be energized for, in MS (makes math easier if divisble by 16)
int timeTilOff = 0;    // increases cool down time as coil is on for longer

void setup() {
pinMode(BUTTON_PIN, INPUT_PULLUP);
attachInterrupt(0, coilButtonOn, RISING);
attachInterrupt(0, coilButtonOff, FALLING);
pinMode(SSR_PIN, OUTPUT);
digitalWrite(SSR_PIN, LOW);

ring.begin();
ring.show();

}

void loop() {

buttonState = digitalRead(BUTTON_PIN);

if(buttonState && !timeLockout){
    timerLockControl();
    digitalWrite(SSR_PIN, HIGH); // energizing the coil circuit
    // loop animation
    // increase timer
    timeTilOff = (currentMillis - previousMillis); // does this need a multiplier?
}
}

void recharge(){} // animation and recharge timer thing

void timerLockControl() {
currentMillis = millis();
  if (currentMillis - previousMillis >= maxTimeOn ) {
    previousMillis = currentMillis;
    timeLockout = 1; // time is locked out, coil cannot be fired
    }
}
void timerUnlockControl() {
    currentMillis = millis();
    if (currentMillis - previousMillis >= timeTilOff ) {
      previousMillis = currentMillis;  
      timeLockout = 0; // unlock the coil to fire again
}
}




//Interrupt code below
void coilButtonOn() {
  return; //  may leave this function here, not sure it'll be needed
  }

void coilButtonOff() {
  digitalWrite(SSR_PIN, LOW);
  recharge(); 
  }
