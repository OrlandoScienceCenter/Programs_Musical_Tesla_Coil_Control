#include <Adafruit_NeoPixel.h>
#define BUTTON_PIN 3 // needs to be on an interrupt pin
#define RING_PIN 9
#define PIXELCOUNT 16
#define SSR_PIN 13


/***************************************/
/*	       Global Vars / Inits         */
/***************************************/

Adafruit_NeoPixel ring = Adafruit_NeoPixel (PIXELCOUNT, RING_PIN, NEO_GRB + NEO_KHZ800);

uint8_t lockout = 0;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long timeHeldOn = 0;
volatile uint8_t buttonState = 0; // has to be volitile because of interrupts 

int maxTimeOn = 8000; //max time coil can be energized for, in MS (make div 16)
int timeTilOff = 0;    // increases cool down time as coil is on for longer



/***************************************/
/*				 SETUP                 */
/***************************************/
void setup() {
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	//attachInterrupt(0, coilButtonOn, RISING); 
	attachInterrupt(0, coilButtonOff, FALLING);
	pinMode(SSR_PIN, OUTPUT);
	digitalWrite(SSR_PIN, LOW);
	// Initialize all neopixels to off
	ring.begin();
	ring.show();
}


/***************************************/
/*		  		 LOOP                  */
/***************************************/
void loop() {
	buttonState = digitalRead(BUTTON_PIN); // read the button state
	//
	if(!lockout){
		systemReady(); // sets up animations for ready/green
	}


	if(buttonState && !lockout){ /// discharging coil -zzzzzzzzzap
		digitalWrite(SSR_PIN, HIGH); // energizing the coil circuit
		timeHeldOn = (currentMillis - previousMillis);//does this need a multiplier
			timerLockControl();
			discharge(); // animation for discharging
	}
	// What to do when the coil becomes locked out, due to time or button release
	if(lockout){ // coil off
		digitalWrite(SSR_PIN, LOW);
		timerUnlockControl();
			if (timeHeldOn > maxTimeOn){
				timeHeldOn = maxTimeOn;
				}
		recharge(); // animation for recharging
	}
	// If the button is not pressed, and system not locked out, count down timer
	if (!buttonState && !lockout){
		timerUnlockControl(); // watches the discharge timer lock
	}
}
/***************************************/
/*		    timerLockControl           */
/***************************************/
void timerLockControl() {
	currentMillis = millis();
	  if (timeHeldOn >= maxTimeOn ) {
		previousMillis = currentMillis;
		lockout = 1; // time is locked out, coil cannot be fired
		}
}

/***************************************/
/*		   timerUnlockControl          */
/***************************************/
	void timerUnlockControl() {
		currentMillis = millis();
		if (currentMillis - previousMillis >= timeHeldOn ) {
		  previousMillis = currentMillis;  
		  lockout = 0; // unlock the coil to fire again
		  timeHeldOn = 0; // reset hold timer
	}
}

/***************************************/
/*	      NeoPixel Ring Control        */
/***************************************/
void systemReady(){
		for(int i = 0; i < (ring.numPixels()); i++){
		ring.setPixelColor(i, 0, 255, 0);
		}
	ring.show();
	}
void recharge(){		
		for(int i = 0; i < (ring.numPixels()); i++){
		ring.setPixelColor(i, 255, 0, 0);
		}
	ring.show();
	} // animation and recharge timer thing
void discharge(){		
		for(int i = 0; i < (ring.numPixels()); i++){
		ring.setPixelColor(i, 255, 255, 0);
		}
	}


/***************************************/
/*	        Interrupt Handling         */
/***************************************/
void coilButtonOff() {
  digitalWrite(SSR_PIN, LOW);
  previousMillis = currentMillis;
  lockout = 1; // locks coil from being re-fired once button is released
  //recharge(); 
  }
