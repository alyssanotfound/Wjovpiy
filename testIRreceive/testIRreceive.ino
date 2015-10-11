#include <IRremote.h>

const int RECV_PIN = 6;
const int NEW_PRESET = 7; //output pin to send low to teensy
const int RESET_PIN = 8;

IRrecv irrecv(RECV_PIN);

decode_results results;
//const int ledPin = 13;
void setup()
{
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
      digitalWrite(NEW_PRESET, LOW);
      delay(500);
      digitalWrite(NEW_PRESET, HIGH);
    } else if (hexVal == "FF906F"){
      digitalWrite(RESET_PIN, LOW);
      delay(500);
      digitalWrite(RESET_PIN, HIGH);  
    }  
      irrecv.resume(); // Receive the next value
  }
}
