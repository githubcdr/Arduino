const int transistorPin = 9;    // connected to the base of the transistor

int led = 9;           // the pin that the LED is attached to
int brightness = 25;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by


void setup() {
  // set  the transistor pin as output:
  //Serial.begin(9600);
  pinMode(transistorPin, OUTPUT);
}

void loop() {
  // read the potentiometer:
  //int sensorValue = analogRead(A0);
  //Serial.println(sensorValue);
  // map the sensor value to a range from 0 - 255:
  //int outputValue = map(sensorValue, 0, 1023, 0, 255);
  // use that to control the transistor:

  analogWrite(transistorPin, brightness);
  brightness = brightness + fadeAmount;
  if (brightness == 25 || brightness == 255) {
    fadeAmount = -fadeAmount ; 
  }
  delay(30);   
}

