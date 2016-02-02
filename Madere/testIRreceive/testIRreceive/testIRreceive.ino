#include <IRremote.h>

const int RECV_PIN = 6;

IRrecv irrecv(RECV_PIN);

decode_results results;
const int ledPin = 13;
void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  irrecv.blink13(true);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  delay(200);                 
  digitalWrite(ledPin, LOW);   
  delay(200);
  Serial.print("start receiver");
}

void loop() {

  if (irrecv.decode(&results)) {
    //Serial.print("decoding");
    digitalWrite(ledPin, HIGH);
    delay(200);                 
    digitalWrite(ledPin, LOW);   
    delay(200);
    if (results.decode_type == NEC) {
      Serial.print("NEC: ");
    } else if (results.decode_type == SONY) {
      Serial.print("SONY: ");
    } else if (results.decode_type == RC5) {
      Serial.print("RC5: ");
    } else if (results.decode_type == RC6) {
      Serial.print("RC6: ");
    } else if (results.decode_type == UNKNOWN) {
      Serial.print("UNKNOWN: ");
    }
    Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
  }
}
