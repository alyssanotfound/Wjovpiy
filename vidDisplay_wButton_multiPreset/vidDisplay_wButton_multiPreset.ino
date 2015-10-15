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

const int  buttonPresetPin = 19;    // the pin that the pushbutton is attached to. other side to GND
const int  buttonResetPin = 23;    // the pin that the pushbutton is attached to. other side to GND
//const int  remotePresetPin = 23;
//const int  remotePresetPinBack = 1;
//const int  remoteResetPin = 0;

// button counter for Preset 
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 1;         // current state of the button
int lastButtonState = 0;     // previous state of the button

// button counter for Reset 
int buttonPushCounterReset = 0;
int buttonStateReset = 1;
int lastButtonStateReset = 0;

//// remote counter for Preset 
//int remotePushCounter = 0;   
//int remoteState = 1;         
//int lastRemoteState = 0;        
//
//// remote counter for Reset 
//int remotePushCounterReset = 0;   
//int remoteStateReset = 1;         
//int lastRemoteStateReset = 0;

//********add after above buttons/remote work *****************************
// remote counter for back one Preset 
//int remotePushCounterBack = 0;   
//int remoteStateBack = 1;         
//int lastRemoteStateBack = 0;  

//setup for filename 
//will eventually replace below with counting num of presets from SD card and storing names
int numPresets = 44;
//change for TA, TB, or TC
char *filenames[] = { "T.bin","T1.bin","T2.BIN","T3.BIN","T4.BIN","T5.BIN","T6.BIN","T7.BIN","T8.BIN","T9.BIN","T10.BIN",
"T11.BIN","T12.BIN","T13.BIN","T14.BIN","T15.BIN","T16.BIN","T17.BIN","T18.BIN","T19.BIN","T20.BIN",
"T21.BIN","T22.BIN","T23.BIN","T24.BIN","T25.BIN","T26.BIN","T27.BIN","T28.BIN","T29.BIN","T30.BIN",
"T31.BIN","T32.BIN","T33.BIN","T34.BIN","T35.BIN","T26.BIN","T27.BIN","T28.BIN","T29.BIN","T40.BIN",
"T41.BIN","T42.BIN","T43.BIN" };

//char *filenames[] = { "T1A.bin","T1B.bin","T1C.bin","T2A.bin","T2B.bin","T4A.bin","T4B.bin","T5A.bin","T5B.bin","T5C.bin",
//"T7A.bin","T7B.bin","T8A.bin","T8B.bin","T8C.bin","T8D.bin","T10A.bin","T10B.bin","T10C.bin","T11A.bin","T11B.bin",
//"T11C.bin","T11D.bin","T12A.bin","T12B.bin","T12C.bin","T12D.bin","T13A.bin","T13B.bin","T14A.bin","T14B.bin","T15A.bin",
//"T15B.bin","T17A.bin","T17B.bin","T18A.bin","T18B.bin","T18C.bin","T18D.bin","T19A.bin","T19B.bin","T21A.bin","T21B.bin",
//"T23A.bin","T23B.bin","T25A.bin","T26A.bin","T30A.bin","T31A.bin","T32B.bin","T32C.bin","T32D.bin","T33A.bin","T33B.bin","T35A.bin" };

//char *filenames[] = { "gamma1_8.BIN","gamma1_0.BIN","gamma0_8.BIN","gamma0_2.BIN"};
int curFileNum = 0;

//for SD card read
File root;

// ***************************************************************************

void setup() {
  pinMode(buttonPresetPin, INPUT_PULLUP);
  pinMode(buttonResetPin, INPUT_PULLUP);
//  pinMode(remotePresetPin, INPUT_PULLUP);
//  pinMode(remotePresetPinBack, INPUT_PULLUP);
//  pinMode(remoteResetPin, INPUT_PULLUP);
  
  //********add after above buttons/remote work *****************************

//  pinMode(17, INPUT_PULLUP);
//  pinMode(19, INPUT_PULLUP);
//  pinMode(22, INPUT_PULLUP);
//  pinMode(23, INPUT_PULLUP);
//  pinMode(1, INPUT_PULLUP);
//  pinMode(0, INPUT_PULLUP);
  



  Serial.begin(9600);
//  while (!Serial){
//  //wait for serial to initalize
//  };
  delay(3000);
  Serial.println("VideoSDcard");
  leds.begin();
  leds.show();
  if (!SD.begin(3)) stopWithErrorMessage("Could not access SD card");
  Serial.println("SD card ok");
  videofile = SD.open(filenames[ curFileNum ], FILE_READ);
  if (!videofile) stopWithErrorMessage("Could not read file");
  Serial.println("File opened");
  playing = true;
  elapsedSinceLastFrame = 0;
  
}

// read from the SD card, true=ok, false=unable to read
// the SD library is much faster if all reads are 512 bytes
// this function lets us easily read any size, but always
// requests data from the SD library in 512 byte blocks.
bool sd_card_read(void *ptr, unsigned int len, bool resetBuf) {

  static unsigned char buffer[512];
  static unsigned int bufpos = 0;
  static unsigned int buflen = 0;
  unsigned char *dest = (unsigned char *)ptr;
  unsigned int n;
  
  if (resetBuf == true){
    bufpos = 0;
    buflen = 0;
  } else {
  }

  while (len > 0) {
    if (buflen == 0) {
      n = videofile.read(buffer, 512);
      if (n == 0) return false;		
      buflen = n;
      bufpos = 0;
    }
    unsigned int n = buflen;
    if (n > len) n = len;
    memcpy(dest, buffer + bufpos, n);
    dest += n;
    bufpos += n;
    buflen -= n;
    len -= n;
    
  }
  return true;
}

// skip past data from the SD card
//if we read less than frame size skip forward to next header
void sd_card_skip(unsigned int len) {
  unsigned char buf[256];

  while (len > 0) {
    unsigned int n = len;
    if (n > sizeof(buf)) n = sizeof(buf);
    sd_card_read(buf, n, 0);
    len -= n;
  }
}


void loop() {

  unsigned char header[5];
  
  if (playing) {
    //Serial.println("yes playing");
    if (sd_card_read(header, 5, incrementVid)) {
      if (header[0] == '*') {
        // found an image frame
        unsigned int size = (header[1] | (header[2] << 8)) * 3;
        unsigned int usec = header[3] | (header[4] << 8);
        unsigned int readsize = size;
	//Serial.printf("v: %u %u", size, usec);
        if (readsize > sizeof(drawingMemory)) {
          readsize = sizeof(drawingMemory);
        }
        if (sd_card_read(drawingMemory, readsize, 0)) {
          while (elapsedSinceLastFrame < usec) ; // wait
          elapsedSinceLastFrame -= usec;
          leds.show();
        } else {
          error("unable to read video frame data");
          return;
        }
        if (readsize < size) {
          sd_card_skip(size - readsize);
        }
      } else {
        error("unknown header");
        return;
      }
    } else {
      error("unable to read 5-byte header");
      return;
    }
    incrementVid = false;
  } else {
    delay(500);
    videofile = SD.open(filenames[ curFileNum ], FILE_READ);
    if (videofile) {
      playing = true;
      elapsedSinceLastFrame = 0;
    }
  }
  
  // *************************added from multiButtonPress FOR PRESET BUTTON *******
  buttonState = digitalRead(buttonPresetPin);
  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // ADDED - switched to be low, which would indicate pushed button
    if (buttonState == LOW) {
      buttonPushCounter++;
      incrementVid = true;
      error("new preset");
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
  //********************************************************************************
  
  // *************************added from multiButtonPress FOR RESET BUTTON *******
  buttonStateReset = digitalRead(buttonResetPin);
  // compare the buttonState to its previous state
  if (buttonStateReset != lastButtonStateReset) {
    // ADDED - switched to be low, which would indicate pushed button
    if (buttonStateReset == LOW) {
      buttonPushCounterReset++;
      incrementVid = true;
      error("reset");
      curFileNum = 0;
      //Serial.println("on");
      Serial.print("number of button pushes:  ");
      Serial.println(buttonPushCounterReset); 
    } else {
      //Serial.println("off");
    }
    delay(50);
  }
  lastButtonStateReset = buttonStateReset;
  //********************************************************************************
  // *************************added from multiButtonPress FOR PRESET Remote *******
//  remoteState = digitalRead(remotePresetPin);
//  // compare the buttonState to its previous state
//  if (remoteState != lastRemoteState) {
//    // ADDED - switched to be low, which would indicate pushed button
//    if (remoteState == LOW) {
//      remotePushCounter++;
//      incrementVid = true;
//      error("new preset");
//      if (curFileNum == (numPresets-1) ) {
//          curFileNum = 0;
//      } else {
//          curFileNum++;
//      }
//      //Serial.println("on");
//      Serial.print("number of remote forward pushes:  ");
//      Serial.println(remotePushCounter); 
//    } else {
//      //Serial.println("off");
//    }
//    delay(50);
//  }
//  lastRemoteState = remoteState;
//  //********************************************************************************
//  
//  // *************************added from multiButtonPress FOR RESET Remote *******
//  remoteStateReset = digitalRead(remoteResetPin);
//  // compare the buttonState to its previous state
//  if (remoteStateReset != lastRemoteStateReset) {
//    // ADDED - switched to be low, which would indicate pushed button
//    if (remoteStateReset == LOW) {
//      remotePushCounterReset++;
//      incrementVid = true;
//      error("reset");
//      curFileNum = 0;
//      //Serial.println("on");
//      Serial.print("number of remote reset pushes:  ");
//      Serial.println(remotePushCounterReset); 
//    } else {
//      //Serial.println("off");
//    }
//    delay(50);
//  }
//  lastRemoteStateReset = remoteStateReset;
//  //********************************************************************************
//  // *************************added from multiButtonPress FOR PRESET Back Remote *******
//  remoteStateBack = digitalRead(remotePresetPinBack);
//  // compare the buttonState to its previous state
//  if (remoteStateBack != lastRemoteStateBack) {
//    // ADDED - switched to be low, which would indicate pushed button
//    if (remoteStateBack == LOW) {
//      remotePushCounter++;
//      incrementVid = true;
//      error("back one preset");
//      if (curFileNum == 0 ) {
//          curFileNum = (numPresets-1);
//      } else {
//          curFileNum = curFileNum - 1;
//      }
//      //Serial.println("on");
//      Serial.print("number of back remote pushes:  ");
//      Serial.println(remotePushCounterBack); 
//    } else {
//      //Serial.println("off");
//    }
//    delay(50);
//  }
//  lastRemoteStateBack = remoteStateBack;
  //********************************************************************************
}

void error(const char *str) {
  Serial.println(str);
  videofile.close();
  playing = false;
}

void stopWithErrorMessage(const char *str) {
  while (1) {
    delay(1000);
  }
}


