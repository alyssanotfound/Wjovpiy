#include <IRremote.h>
#include <LiquidCrystal.h>
//rs,e,d4,d5,d6,d7
LiquidCrystal lcd(17, 18, 19, 20, 21, 22);

const int RECV_PIN = 6;
const int NEW_PRESET = 7; //output pin to send low to teensy
const int RESET_PIN = 8;

int numPresets = 10;
char *filenames[] = { "VIDEO.BIN","T1A.BIN","T2A.BIN","T3A.BIN","T4A.BIN","T5A.BIN","TA17.BIN","TA18.BIN","TA19.BIN","TA20.BIN" };
int curFileNum = 0;

IRrecv irrecv(RECV_PIN);

decode_results results;
//const int ledPin = 13;
void setup()
{
  lcd.begin(16, 2);
  lcd.print("MADERE 2015");
  delay(4000);
  lcd.setCursor(0, 0);
  lcd.print("now playing:");
  lcd.setCursor(0, 2);
  lcd.print(filenames[ curFileNum ]);
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  irrecv.blink13(true);
  //pinMode(ledPin, OUTPUT);
  pinMode(NEW_PRESET, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(NEW_PRESET, HIGH);
  digitalWrite(RESET_PIN, HIGH);
}

void loop() {
  if (irrecv.decode(&results)) {
    //Serial.print("decoding");
//    digitalWrite(ledPin, HIGH);
//    delay(200);                 
//    digitalWrite(ledPin, LOW);   
//    delay(200);
//    if (results.decode_type == NEC) {
//      Serial.print("NEC: ");
//    } else if (results.decode_type == SONY) {
//      Serial.print("SONY: ");
//    } else if (results.decode_type == RC5) {
//      Serial.print("RC5: ");
//    } else if (results.decode_type == RC6) {
//      Serial.print("RC6: ");
//    } else if (results.decode_type == UNKNOWN) {
//      Serial.print("UNKNOWN: ");
//    }
    String hexVal = String(results.value, HEX);
    Serial.println(hexVal);
    
    if (hexVal == "FFA05F") {
      if (curFileNum == (numPresets-1) ) {
          curFileNum = 0;
      } else {
          curFileNum++;
      }
      lcd.clear();
      lcd.print("new preset");
      lcd.setCursor(0, 2);
      lcd.print(filenames[ curFileNum ]);
      digitalWrite(NEW_PRESET, LOW);
      delay(500);
      digitalWrite(NEW_PRESET, HIGH);
    } else if (hexVal == "FF906F"){
      curFileNum = 0;
      lcd.clear();
      lcd.print("reset presets");
      lcd.setCursor(0, 2);
      lcd.print(filenames[ curFileNum ]);
      digitalWrite(RESET_PIN, LOW);
      delay(500);
      digitalWrite(RESET_PIN, HIGH);  
    }  
      irrecv.resume(); // Receive the next value
  }
}
