#include <SPI.h>
#include <SD.h>
// this constant won't change:
const int  buttonPresetPin = 17;    // the pin that the pushbutton is attached to
const int  buttonResetPin = 18;
//const int ledPin = 13;       // the pin that the LED is attached to

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
char *filenames[] = { "video.bin", "utah.bin","sycho.bin","curl.bin","swipe.bin" };
int curFileNum = 0;

//for SD card read
File root;

void setup() {
  
  // initialize the button pin as a input:
  pinMode(buttonPresetPin, INPUT_PULLUP);
  pinMode(buttonResetPin, INPUT_PULLUP);
  // initialize the LED as an output:
  //pinMode(ledPin, OUTPUT);
  // initialize serial communication:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(3)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  root = SD.open("/");

  printDirectory(root, 0);

  Serial.println("done!");
}


void loop() { 
  Serial.println("N");
  //FOR PRESET BUTTON
  // read the pushbutton input pin:
  buttonState = digitalRead(buttonPresetPin);
  //Serial.println(buttonState);
  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button
      // wend from off to on:
      Serial.println(filenames[ curFileNum ]);
      
      Serial.println(SD.exists( filenames[ curFileNum ] ));
      buttonPushCounter++;
      if (curFileNum == (numPresets-1) ) {
        //start over at 0
        Serial.println("start over counter");
        curFileNum = 0;
      } else {
        //increment one preset
        curFileNum++;
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
    delay(50);
  }
  // save the current state as the last state,
  //for next time through the loop
  lastButtonState = buttonState;
  Serial.println("E");
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
  
}

//incorporate later to read num files and store names
void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
