/*  OctoWS2811 VideoSDcard.ino - Video on LEDs, played from SD Card
    http://www.pjrc.com/teensy/td_libs_OctoWS2811.html
    Copyright (c) 2014 Paul Stoffregen, PJRC.COM, LLC h
    
    wed 9:41pm - code works perfectly on Tc but on Tb, the button doesn't work. it isn't an issue with the 
    files on the sd card, or with the file playing (if change first file playing it works), or physical button
    bc works on C and no serial prints display when the button is pressed, so it must be that the spare pins on 
    octo board don't work (but they worked at home, so maybe it is insulation problem under the teensy. 
    not sure exactly about why tA wasn't registering a button press, maybe it was bc the SD cards were switched.
    
    when home, check that all output pins work on teensys
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

// button counter for Preset 
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 1;     // previous state of the button

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

  //FIXXXXX should consisently be high or low
  digitalWrite(buttonPresetPinPwr, HIGH);

  Serial.begin(9600);
  while (!Serial){
  //wait for serial to initalize
  };
  delay(50);
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
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
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


