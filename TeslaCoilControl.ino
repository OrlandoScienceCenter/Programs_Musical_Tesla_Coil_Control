#include <Adafruit_NeoPixel.h>
#define BUTTON_PIN 3 // needs to be on an interrupt pin
#define RING_PIN 6
#define PIXELCOUNT 16
#define SSR_PIN A0


/***************************************/
/*	       Global Vars / Inits         */
/***************************************/

Adafruit_NeoPixel ring = Adafruit_NeoPixel (PIXELCOUNT, RING_PIN, NEO_GRB + NEO_KHZ800);

uint8_t lockout = 0;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long timeHeldOn = 0;
volatile uint8_t buttonState = 0; // has to be volitile because of interrupts 
uint8_t pixelPosition = 0; //current pixel position. always <= PIXELCOUNT

int maxTimeOn = 4000; //max time coil can be energized for, in MS (make div 16)
int timeTilOff = 0;    // increases cool down time as coil is on for longer
int protectionDelay = 1000; // minimum delay on recharge


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
	// Initialize all neopixels to off
	ring.begin();
	ring.show();
	Serial.begin(9600);
}


/***************************************/
/*		  		 LOOP                  */
/***************************************/
void loop() {
	buttonState = digitalRead(BUTTON_PIN); // read the button state
	//
	if(!lockout && !buttonState){
		systemReady(); // sets up animations for ready/green
	}
	//
	if(buttonState && !lockout){ /// discharging coil -zzzzzzzzzap
		digitalWrite(SSR_PIN, HIGH); // energizing the coil circuit
		if ((currentMillis - previousMillis)< protectionDelay){
		timeHeldOn = protectionDelay;
		}
    else{
		timeHeldOn = (currentMillis - previousMillis);
		}
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
		for(pixelPosition = 0; pixelPosition < (ring.numPixels()); pixelPosition++){
		ring.setPixelColor(pixelPosition, 0, 180, 0);
		}
	ring.show();
	Serial.println("system ready");
	}
void recharge(){		
		Serial.println("recharging");
		for(int i=0; i < (ring.numPixels()); i++){
		ring.setPixelColor(i, 180, 0, 0);
		}
	ring.show();
	} // animation and recharge timer thing
void discharge(){	
	uint8_t dischargePixelPosition = 0;
		for(dischargePixelPosition; dischargePixelPosition < pixelPosition; dischargePixelPosition++){ // sets all active pixels to red
		ring.setPixelColor(dischargePixelPosition, 0, 0, 180); 
		}
		dischargePixelPosition = pixelPosition;
		for (int i = dischargePixelPosition; i < ring.numPixels(); i++){
		ring.setPixelColor(i, 0,0,0); // handles the blackout portion 
		}
		ring.show(); // show the full circle now
		Serial.println("firing");
		}

void pixelIntervalTimer (){
	uint8_t intervalDivisor = 0;
		intervalDivisor = maxTimeOn / (ring.numPixels()); // sets up the divisor for time to determine pixel position value
		//
		currentMillis = millis();
		if (currentMillis - previousMillis >= intervalDivisor ) {
		  //previousMillis = currentMillis; taken care of in other parts of program  
		pixelPosition = ring.numPixels() -(timeHeldOn / intervalDivisor);
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
