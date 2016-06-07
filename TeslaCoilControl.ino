#include<FastLED.h>


#define BUTTON_PIN 3 // needs to be on an interrupt pin
#define SSR_PIN A0
//FastLED
#define RING_PIN 6
#define PIXELCOUNT 16

/***************************************/
/*	       Global Vars / Inits         */
/***************************************/

CRGB ring[PIXELCOUNT];

uint8_t lockout = 0;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long timeHeldOn = 0;
volatile uint8_t buttonState = 0; // has to be volitile because of interrupts 
uint8_t pixelPosition = 0; //current pixel position. always <= PIXELCOUNT

int maxTimeOn = 4000; //max time coil can be energized for, in MS (make div 16)
int timeTilOff = 0;    // increases cool down time as coil is on for longer
int protectionDelay = 500; // minimum delay on recharge (value is *2 in code below)


;
/***************************************/
/*				 SETUP                 */
/***************************************/
void setup() {
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	//attachInterrupt(0, coilButtonOn, RISING); 
	attachInterrupt(0, coilButtonOff, FALLING);
	pinMode(SSR_PIN, OUTPUT);
	digitalWrite(SSR_PIN, LOW);
	Serial.begin(9600);
//Initialize FastLED
	FastLED.addLeds<NEOPIXEL, RING_PIN>(ring, PIXELCOUNT);
	}


/***************************************/
/*		  		 LOOP                  */
/***************************************/
void loop() {
	delay(5); // remove before implementation 
	buttonState = digitalRead(BUTTON_PIN); // read the button state
	pixelIntervalTimer();
	//
	if(!lockout && !buttonState){
		systemReady(); // sets up animations for ready/green
	}
	//
	if(buttonState && !lockout){ /// discharging coil -zzzzzzzzzap
		digitalWrite(SSR_PIN, HIGH); // energizing the coil circuit
		timeHeldOn = (currentMillis - previousMillis);
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
			if (timeHeldOn < protectionDelay){
				timeHeldOn = protectionDelay;
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
		if (currentMillis - previousMillis >= (timeHeldOn * 2)) {
		  previousMillis = currentMillis;  
		  lockout = 0; // unlock the coil to fire again
		  timeHeldOn = 0; // reset hold timer
	}
}

/***************************************/
/*	      NeoPixel Ring Control        */
/***************************************/
void systemReady(){
		for(pixelPosition = 0; pixelPosition < (PIXELCOUNT); pixelPosition++){
		ring[pixelPosition].setRGB(0, 255, 0);
		}
	FastLED.show();
	Serial.println("system ready");
	}
void recharge(){		
		Serial.print("recharging");
		Serial.print("   ");
		Serial.println(pixelPosition);
		for(int i=0; i < (PIXELCOUNT); i++){
		ring[i].setRGB(255, 0, 0);
		}
	FastLED.show();
	} // animation and recharge timer thing
void discharge(){	
		for(int i=0; i < (PIXELCOUNT); i++){
		ring[i].setRGB(0, 0, 255);
		}
		FastLED.show();
		Serial.print("firing -- TIME ON  ");
		Serial.print(timeHeldOn);
		Serial.print("   ");
		Serial.println(pixelPosition);
		}

		
/***************************************/
/*	        pixelIntervalTimer         */
/***************************************/		
void pixelIntervalTimer (){
	uint8_t intervalDivisor = 0;
		intervalDivisor = maxTimeOn / (PIXELCOUNT); // sets up the divisor for time to determine pixel position value
		//
		currentMillis = millis();
		if (!lockout && currentMillis - previousMillis >= intervalDivisor ) {
		  pixelPosition = PIXELCOUNT -(timeHeldOn / intervalDivisor);
		}
		if (lockout && (currentMillis - previousMillis >= intervalDivisor ) && (pixelPosition < 16)) {
		  delay(intervalDivisor);  /// this is bad must remove and fix because stupid. 
		  pixelPosition++;
		
		}
	}
		
		


/***************************************/
/*	        Interrupt Handling         */
/***************************************/
void coilButtonOff() {
  digitalWrite(SSR_PIN, LOW);
  //previousMillis = currentMillis;
  lockout = 1; // locks coil from being re-fired once button is released
  }
