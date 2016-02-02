/*  OctoWS2811 VideoSDcard.ino - Video on LEDs, played from SD Card
    http://www.pjrc.com/teensy/td_libs_OctoWS2811.html
    Copyright (c) 2014 Paul Stoffregen, PJRC.COM, LLC h
    
    wed 9:41pm - code works perfectly on Tc but on Tb, the button doesn't work. it isn't an issue with the 
    files on the sd card, or with the file playing (if change first file playing it works), or physical button
    bc works on C and no serial prints display when the button is pressed, so it must be that the spare pins on 
    octo board don't work (but they worked at home, so maybe it is insulation problem under the teensy. 
    not sure exactly about why tA wasn't registering a button press, maybe it was bc the SD cards were switched.
    
    when home, check that all output pins work on teensys
    
    update: the issue was that for tA and tB, the other end of the button needed to be at high instead of low
*/

#include <OctoWS2811.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>

#define LED_WIDTH    300   // number of LEDs horizontally
#define LED_HEIGHT   8   // number of LEDs vertically (must be multiple of 8)

//#define FILENAME     

const int ledsPerStrip = LED_WIDTH * LED_HEIGHT / 8;
DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[ledsPerStrip*6];
elapsedMicros elapsedSinceLastFrame = 0;
bool playing = false;
bool incrementVid = false;

OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, WS2811_800kHz);
File videofile;

// *************************added from multiButtonPress *************************
const int  buttonPresetPin = 17;    // the pin that the pushbutton is attached to. other side to PWR
const int  buttonPresetPinPwr = 18;
//ADDED
const int ledPin = 13;

// button counter for Preset 
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 1;         // current state of the button
int lastButtonState = 0;     // previous state of the button

//setup for filename 
//will eventually replace below with counting num of presets from SD card and storing names
int numPresets = 10;
//change for TA, TB, or TC
char *filenames[] = { "T1C.BIN","VIDEO.BIN","T2C.BIN","T3C.BIN","T4C.BIN","T5C.BIN","TC17.BIN","TC18.BIN","TC19.BIN","TC20.BIN" };

int curFileNum = 0;

//for SD card read
File root;

// ***************************************************************************

void setup() {
  pinMode(buttonPresetPin, INPUT_PULLUP);
  pinMode(buttonPresetPinPwr, OUTPUT);

  //FIXXXXX should consisently be low, but Ta and Tb work on high
  digitalWrite(buttonPresetPinPwr, LOW);

  Serial.begin(9600);
  while (!Serial){
  //wait for serial to initalize
  };
  delay(50);
  Serial.println("VideoSDcard");

  //ADDED
  pinMode(ledPin, OUTPUT);
  
}


void loop() {
  digitalWrite(ledPin, HIGH);

  // *************************added from multiButtonPress FOR PRESET BUTTON *******
  buttonState = digitalRead(buttonPresetPin);
  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == LOW) {
      buttonPushCounter++;
      incrementVid = true;
      if (curFileNum == (numPresets-1) ) {
          curFileNum = 0;
      } else {
          curFileNum++;
      }
      //Serial.println("on");
      Serial.print("number of button pushes:  ");
      Serial.println(buttonPushCounter); 
    } else {
      //Serial.println("off");
    }
    delay(50);
  }
  lastButtonState = buttonState;
  // ********************************************************************************
}


