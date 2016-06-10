#include<FastLED.h>
//PinDefines
#define BUTTON_PIN 		3 // needs to be on an interrupt pin
#define SSR_PIN A0
//FastLED
#define RING_PIN 		6
#define PIXELCOUNT 		16
#define	BRIGHTNESS 		20 // sets fastLED global brightness
//
#define DEBOUNCE_TIME 	50 // how long the button needs to be pressed before activate
						  // if it feels like there is a "delay" before the coil activates
				          // lower this value

						  
#define MAX_TIME_ON 	4000  // maximum tmie coil can be held on (ms)

#define MIN_RECHG_DELAY 1000 // mininum time for recharge before button can be hit again
							 // helps prevent button mashers 

/***************************************/
/*	       Global Vars / Inits         */
/***************************************/

CRGB ring[PIXELCOUNT]; // FastLED setup

volatile uint8_t lockout = 0;
volatile unsigned long previousMillis = 0;
volatile unsigned long currentMillis = 0;
volatile uint16_t timeHeldOn = 0;
volatile int buttonState = 0; // has to be volitile because of interrupts 
uint8_t pixelPosition = 16; //current pixel position. always <= PIXELCOUNT

uint16_t timeTilOff = MAX_TIME_ON;    // increases cool down time as coil is on for longer



;
/***************************************/
/*				 SETUP                 */
/***************************************/
void setup() {
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	attachInterrupt(0, coilButtonOff, FALLING);
//	attachInterrupt(0, coilButtonOn, RISING);
	pinMode(SSR_PIN, OUTPUT);
	digitalWrite(SSR_PIN, LOW);
	Serial.begin(9600);
//Initialize FastLED
	FastLED.addLeds<NEOPIXEL, RING_PIN>(ring, PIXELCOUNT);
	FastLED.setBrightness(BRIGHTNESS);
	}


/***************************************/
/*		  		 LOOP                  */
/***************************************/
void loop() {
	//delay(5); // remove before implementation 
	buttonState = digitalRead(BUTTON_PIN); // read the button state
							// normally pulled high by pullup resitor internally
							// but also connected to the NC portion of switch
							// 0 - not pressed 1 - pressed 
	
	Serial.print ("ButtonState - ");
	Serial.println(buttonState);
	Serial.print ("Lockout - ");
	Serial.println(lockout);
	currentMillis = millis();
	//
	if(!lockout && !buttonState){
		systemReady(); // sets up animations for ready/green
		previousMillis = currentMillis;
		pixelPosition = 16;
	}
	//
	if(buttonState && !lockout){ /// discharging coil -zzzzzzzzzap
		timeHeldOn = (currentMillis - previousMillis);
			if (timeHeldOn > DEBOUNCE_TIME){ // set up in combination with coilButtonOff interrupt for debounce 
			digitalWrite(SSR_PIN, HIGH); // energizing the coil circuit
			pixelIntervalTimer();
			discharge(); // animation for discharging
			}
		timerLockControl();
			}
	// 
	if(buttonState && lockout){ // coil off
		digitalWrite(SSR_PIN, LOW);
		}
	// If the button is not pressed, and system is locked out, count down timer
	if (!buttonState && lockout){
      digitalWrite(SSR_PIN, LOW);
      recharge(); // animation for recharging
		if (timeHeldOn < MIN_RECHG_DELAY){
		timeTilOff = MIN_RECHG_DELAY;
		}
	  else{
	  timeTilOff = timeHeldOn + DEBOUNCE_TIME;
	  }	
	    pixelIntervalTimer();
		timerUnlockControl(); // watches the discharge timer lock
	}
}
/***************************************/
/*		    timerLockControl           */
/***************************************/
void timerLockControl() {
	currentMillis = millis();
	Serial.println(timeHeldOn);
	  if ((timeHeldOn > DEBOUNCE_TIME) && (timeHeldOn - DEBOUNCE_TIME) >= MAX_TIME_ON ) {
		timeHeldOn = MAX_TIME_ON; // sets time on to max anyway if reached or over max
		//previousMillis = currentMillis;
		lockout = 1; // time is locked out, coil cannot be fired
		}
}

/***************************************/
/*		   timerUnlockControl          */
/***************************************/
	void timerUnlockControl() {
		currentMillis = millis();
		if (currentMillis - previousMillis >= (timeTilOff * 2) && pixelPosition == 16) { 
					//check the time, and then check the pixel position for a dirty second check
		  //previousMillis = currentMillis;  
		  lockout = 0; // unlock the coil to fire again
		  timeTilOff = MAX_TIME_ON;
		  timeHeldOn = MAX_TIME_ON;
		 
	}
}

/***************************************/
/*	      NeoPixel Ring Control        */
/***************************************/
void systemReady(){
		for(int p = 0; p < (PIXELCOUNT); p++){
		ring[p].setRGB(0, 255, 0);
		}
	FastLED.show();
	//Serial.println("system ready");
	}
void recharge(){		
	int p = pixelPosition;
		for(int i = 0; i < (PIXELCOUNT); i++){
		ring[i].setRGB(255, 0, 0);
		}
		for(p; p <= (PIXELCOUNT); p++){
		ring[p].setRGB(0, 0, 0);
		}
	FastLED.show();
	//Serial.print("recharging");
	//Serial.print("   ");
	//Serial.println(pixelPosition);
		} // animation and recharge timer thing
void discharge(){	
	int p = pixelPosition;
		for(int i = 0; i <= (PIXELCOUNT); i++){
			ring[i].setRGB(0, 0, 0);
			}
		for(p; p > (0); p--){
			ring[p].setRGB(0, 0, 255);
			}
		FastLED.show();
		//Serial.print("firing -- TIME ON  ");
		//Serial.print(timeHeldOn);
		//Serial.print("   ");
		//Serial.println(pixelPosition);
		}

		
/***************************************/
/*	        pixelIntervalTimer         */
/***************************************/		
void pixelIntervalTimer (){
	uint8_t intervalDivisor = 0;
		intervalDivisor = MAX_TIME_ON / (PIXELCOUNT); // sets up the divisor for time to determine pixel position value
		//
		currentMillis = millis();
		if (!lockout && pixelPosition > 0) {
		  delay (intervalDivisor);
		  pixelPosition--; 
		  Serial.print ("Pixel Position - ");
		  Serial.println(pixelPosition);
		}
		if (lockout && (pixelPosition < 16)){
		  delay(intervalDivisor * 2);  /// this is bad must remove and fix. not safety timing, but should fix. 
		  pixelPosition++;
		  Serial.print ("Pixel Position - ");
		  Serial.println(pixelPosition);
		}
	}
		
		
/***************************************/
/*	        Interrupt Handling         */
/***************************************/
void coilButtonOff() {
  if (timeHeldOn > 2){ // essentially performing a debounce check and 
  buttonState = 0; // forces buttonstate off, so green isn't re-triggerd  
  lockout = 1; // locks coil from being re-fired once button is released
  previousMillis = currentMillis;
  }
 }
