#include <OctoWS2811.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>

#define LED_WIDTH    300   // number of LEDs horizontally
#define LED_HEIGHT   8   // number of LEDs vertically (must be multiple of 8)

const int ledsPerStrip = LED_WIDTH * LED_HEIGHT / 8;
DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[ledsPerStrip*6];
elapsedMicros elapsedSinceLastFrame = 0;
bool playing = false;

OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, WS2811_800kHz);
File videofile;

// *************************added from multiButtonPress *************************
const int  buttonPresetPin = 17;    // the pin that the pushbutton is attached to
const int  buttonResetPin = 18;

// button counter for Preset 
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button

// button counter for Preset 
int buttonPushCounterReset = 0;   // counter for the number of button presses
int buttonStateReset = 0;         // current state of the button
int lastButtonStateReset = 0;     // previous state of the button

//setup for filename 
//will eventually replace below with counting num of presets from SD card and storing names
int numPresets = 5;
char *filenames[] = { "video.bin", "video.bin","presetPsycho","presetCurl","presetSwipe" };
int curFileNum = 0;

//for SD card read
File root;

// ***************************************************************************

int modepin = 1;
int mode = 1;
char const * filename = "VIDEO.BIN";

void setup() {
  //added from multiButtonPress
  pinMode(buttonPresetPin, INPUT_PULLUP);
  pinMode(buttonResetPin, INPUT_PULLUP);
  //FILENAME = filenames[ curFileNum ];
  
  Serial.begin(9600);
  while (!Serial){};
  delay(50);
  Serial.println("VideoSDcard");
  leds.begin();
  leds.show();
  //added delay
  delayMicroseconds(3600);
  if (!SD.begin(3)) stopWithErrorMessage("Could not access SD card");
  Serial.println("SD card ok");
  //startNew("start from setup");
  videofile = SD.open(filename, FILE_READ);
  if (!videofile) stopWithErrorMessage("Could not read file");
  Serial.println("File opened");
  playing = true;
  elapsedSinceLastFrame = 0;
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
  unsigned char header[5];

  if (playing) {
    if (sd_card_read(header, 5)) {
          Serial.println(header[0]);
      if (header[0] == '*') {
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
        error("unknown header");
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
    videofile = SD.open(filenames[ curFileNum ], FILE_READ);
    if (videofile) {
      Serial.println("File opened");
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
      //Serial.println(filenames[ curFileNum ]);
      
      //Serial.println(SD.exists( filenames[ curFileNum ] ));
      buttonPushCounter++;
      if (curFileNum == (numPresets-1) ) {
        //start over at 0
        Serial.println("start over counter");
        curFileNum = 0;
        butn();
        //error("new preset");
        //startNew(filenames[ curFileNum ]);
      } else {
        //increment one preset
        curFileNum++;
        butn();
        //error("new preset");
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
  }
  // save the current state as the last state,
  //for next time through the loop
  lastButtonState = buttonState;
  
  
  //FOR RESET BUTTON
  // read the pushbutton input pin:
  buttonStateReset = digitalRead(buttonResetPin);
  //Serial.println(buttonState);
  // compare the buttonState to its previous state
  if (buttonStateReset != lastButtonStateReset) {
    // if the state has changed, increment the counter
    if (buttonStateReset == HIGH) {
      // if the current state is HIGH then the button
      // wend from off to on:
      Serial.println("reset to first preset");
      curFileNum = 0;
      buttonPushCounterReset++;
      //Serial.println("on");
      Serial.print("number of reset pushes:  ");
      Serial.println(buttonPushCounterReset);
      
    } else {
      // if the current state is LOW then the button
      // wend from on to off:
      //Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  // save the current state as the last state,
  //for next time through the loop
  lastButtonStateReset = buttonStateReset;
  //error("reset presets");
  //startNew(filenames[ curFileNum ]);
  // ***************************************************************************
}

// when any error happens during playback, close the file and restart
void error(const char *str) {
  Serial.print("error: ");
  Serial.println(str);
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

void butn(){
mode ++;
   if(mode >= 7){mode = 1;}
     switch (mode) {
    case 1:
    filename = "VIDEO.BIN";
    break;
    case 2:
    filename = "VIDEO2.BIN";
    break;
    case 3:
   filename = "VIDEO3.BIN";
    break;
    case 4:
 filename = "VIDEO4.BIN";
    break;
    case 5:
  filename = "VIDEO5.BIN";
    break;
    case 6:
  filename = "VIDEO6.BIN";
    break;
     } 
      videofile.close();
  playing = false;
 return;
}


