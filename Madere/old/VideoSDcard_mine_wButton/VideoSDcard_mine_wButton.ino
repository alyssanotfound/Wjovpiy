/*  OctoWS2811 VideoSDcard.ino - Video on LEDs, played from SD Card
    http://www.pjrc.com/teensy/td_libs_OctoWS2811.html
    Copyright (c) 2014 Paul Stoffregen, PJRC.COM, LLC
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
const int  buttonPresetPin = 17;    // the pin that the pushbutton is attached to
const int  buttonResetPin = 18;

// button counter for Preset 
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 1;     // previous state of the button

// button counter for Preset 
int buttonPushCounterReset = 0;   // counter for the number of button presses
int buttonStateReset = 0;         // current state of the button
int lastButtonStateReset = 1;     // previous state of the button

//setup for filename 
//will eventually replace below with counting num of presets from SD card and storing names
int numPresets = 5;
char *filenames[] = { "T1A.BIN", "VIDEO.BIN","SYCHO.BIN","CURL.BIN","SWIPE.BIN" };
int curFileNum = 0;

//for SD card read
File root;

// ***************************************************************************

void setup() {
  //added from multiButtonPress
  pinMode(buttonPresetPin, INPUT_PULLUP);
  pinMode(buttonResetPin, INPUT_PULLUP);
  //FILENAME = filenames[ curFileNum ];
  
  Serial.begin(9600);
  while (!Serial){
  //wait for serial to initalize
  };
  delay(50);
  Serial.println("VideoSDcard");
  leds.begin();
  leds.show();
  //added delay
  delayMicroseconds(3600);
  if (!SD.begin(3)) stopWithErrorMessage("Could not access SD card");
  Serial.println("SD card ok");
  //startNew("start from setup");
  videofile = SD.open(filenames[ curFileNum ], FILE_READ);
  if (!videofile) stopWithErrorMessage("Could not read file");
  Serial.println("File opened");
  playing = true;
  elapsedSinceLastFrame = 0;
  //this works below to close and open new file, but doesnt work near button area
  //close currently playing vid, increment vid number, then in loop it should start playing
  //error("does this work");
  //curFileNum++;
}

// read from the SD card, true=ok, false=unable to read
// the SD library is much faster if all reads are 512 bytes
// this function lets us easily read any size, but always
// requests data from the SD library in 512 byte blocks.
//
bool sd_card_read(void *ptr, unsigned int len) {
  static unsigned char buffer[512];
  static unsigned int bufpos = 0;
  static unsigned int buflen = 0;
  unsigned char *dest = (unsigned char *)ptr;
  unsigned int n;

  while (len > 0) {
    if (buflen == 0) {
      n = videofile.read(buffer, 512);
      if (n == 0) return false;		
      buflen = n;
      //Serial.println(buflen);
      bufpos = 0;
    }
    unsigned int n = buflen;
    if (n > len) n = len;
    memcpy(dest, buffer + bufpos, n);
//    Serial.print("DATA: ");
//    Serial.println(dest[0]);
//    Serial.println(bufpos);
//    Serial.println(buflen);
//    Serial.println(len);
    dest += n;
    bufpos += n;
    buflen -= n;
    len -= n;
    
  }
  return true;
}

// skip past data from the SD card
void sd_card_skip(unsigned int len) {
  unsigned char buf[256];

  while (len > 0) {
    unsigned int n = len;
    if (n > sizeof(buf)) n = sizeof(buf);
    sd_card_read(buf, n);
    len -= n;
  }
}


void loop() {
  if (incrementVid == true){
    error("increment video");
    curFileNum++;
    incrementVid = false;
  }
  unsigned char header[5];
  
  Serial.print("is anything playing? y/n: ");
  Serial.println(playing);
  delay(1000);
  if (playing) {
    Serial.print("playing is true, playing file num:");
    //this number isn't updating after button press
    Serial.println(curFileNum);
    if (sd_card_read(header, 5)) {
          Serial.print("5 byte header can be read. first byte: ");
          Serial.println(header[0]);
      if (header[0] == '*') {
        //Serial.println("first byte is an *");
        // found an image frame
        //Serial.println(header[]);
//        Serial.println(header[0]);
//        Serial.println(header[1]);
//        Serial.println(header[2]);
//        Serial.println(header[3]);
//        Serial.println(header[4]);
        unsigned int size = (header[1] | (header[2] << 8)) * 3;
        unsigned int usec = header[3] | (header[4] << 8);
        unsigned int readsize = size;
	 //Serial.printf("v: %u %u", size, usec);
        if (readsize > sizeof(drawingMemory)) {
          readsize = sizeof(drawingMemory);
        }
        if (sd_card_read(drawingMemory, readsize)) {
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
        error("unknown header, starts with ");
        Serial.println(header[0]);
        return;
      }
    } else {
      error("unable to read 5-byte header");
//      Serial.println(header[0]);
//      Serial.println(header[1]);
//      Serial.println(header[2]);
//      Serial.println(header[3]);
//      Serial.println(header[4]);
      return;
    }
  } else {
    delay(500);
    Serial.print("starting playing new file now:");
    //this number updates fine
    Serial.println(filenames[ curFileNum ]);
    videofile = SD.open(filenames[ curFileNum ], FILE_READ);
    //Serial.print("name of file: ");
    //Serial.println(filenames[ curFileNum ]);
    if (videofile) {
      //Serial.println("< is opened bc wasnt");
      playing = true;
      elapsedSinceLastFrame = 0;
    }
  }
  
  // *************************added from multiButtonPress *************************
  //FOR PRESET BUTTON
  // read the pushbutton input pin:
  buttonState = digitalRead(buttonPresetPin);
  //Serial.println(buttonState);
  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button
      // went from off to on:
      Serial.println(filenames[ curFileNum ]);
      
      Serial.println(SD.exists( filenames[ curFileNum ] ));
      buttonPushCounter++;
      if (curFileNum == (numPresets-1) ) {
        //start over at 0
        Serial.println("start over counter");
        
        //error("new preset");
        curFileNum = 0;
        //return;
        
        //error("new preset");
        //startNew(filenames[ curFileNum ]);
      } else {
        //increment one preset
        incrementVid = true;
        //error("new preset");
        //curFileNum++;
        //return;
        //startNew(filenames[ curFileNum ]);
      }
      //Serial.println("on");
      Serial.print("number of button pushes:  ");
      Serial.println(buttonPushCounter);
      
    } else {
      // if the current state is LOW then the button
      // wend from on to off:
      //Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    //error("preset changed");
    //startNew(filenames[ curFileNum ]);
    delay(50);
    //error("new preset");
    //return;
  }
  // save the current state as the last state,
  //for next time through the loop
  lastButtonState = buttonState;
 
  // ***************************************************************************
}

// when any error happens during playback, close the file and restart
void error(const char *str) {
  Serial.print("error: ");
  Serial.println(str);
  Serial.print("closing file: ");
  Serial.println(videofile.name());
  videofile.close();
  playing = false;
}

// when an error happens during setup, give up and print a message
// to the serial monitor.
void stopWithErrorMessage(const char *str) {
  while (1) {
    Serial.println(str);
    delay(1000);
  }
}

// start playing new one
//void startNew(const char *str) {
//  Serial.print("start new preset: ");
//  Serial.println(str);
//  if (!SD.begin(3)) stopWithErrorMessage("Could not access SD card");
//  Serial.println("SD card ok");
//  
//  videofile = SD.open(filenames[ curFileNum ], FILE_READ);
//  if (!videofile) stopWithErrorMessage("Could not read file");
//  
//  Serial.print("File opened, does it exist? ");
//  Serial.println(SD.exists( filenames[ curFileNum ] ));
//  playing = true;
//  elapsedSinceLastFrame = 0;
//}



