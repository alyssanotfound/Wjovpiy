/* Pushbutton with Pullup, Teensyduino Tutorial #3
   http://www.pjrc.com/teensy/tutorial3.html

   This example code is in the public domain.
*/
const int buttonPin = 17;

void setup() {                
  Serial.begin(38400);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop()                     
{
  if (digitalRead(buttonPin) == HIGH) {
    Serial.println("Button is not pressed...");
  } else {
    Serial.println("Button pressed!!!");
  }
  delay(250);
}

