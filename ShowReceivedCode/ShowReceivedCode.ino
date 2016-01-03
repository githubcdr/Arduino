#include <DHT11.h>
#include <NewRemoteReceiver.h>
#include <RemoteTransmitter.h>
#include <NewRemoteTransmitter.h>

int DHT11Pin = 7;
int transmitterPin = 8;
float temperature, humidity;
DHT11 dht11(DHT11Pin); 

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup() {
  Serial.begin(115200);
  inputString.reserve(200);
  Serial.println("booting skynet...");

  // Initialize receiver on interrupt 0 (= digital pin 2), calls the callback "showCode"
  // after 2 identical codes have been received in a row. (thus, keep the button pressed
  // for a moment)
  //
  // See the interrupt-parameter of attachInterrupt for possible values (and pins)
  // to connect the receiver.
  NewRemoteReceiver::init(0, 2, showCode);
}

void loop() {
  dht11.read(humidity, temperature);
  delay(DHT11_RETRY_DELAY);

  Serial.println( "T=" + String(temperature, 2) );
  Serial.println( "H=" + String(humidity, 2) );

  if (stringComplete) {
    Serial.println(inputString); 
    // clear the string:
    inputString = "";
    stringComplete = false;
  }

  action( 231337, 0, 0 );
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } 
  }
}

void action(unsigned long int addressID, int unitID, int dim ){
  NewRemoteTransmitter transmitter(addressID, transmitterPin, 260, 3);

  NewRemoteReceiver::disable();
  transmitter.sendDim(unitID, dim);
  delay(1000);
  NewRemoteReceiver::enable();

}

// Callback function is called only when a valid code is received.
void showCode(NewRemoteCode receivedCode) {
  // Note: interrupts are disabled. You can re-enable them if needed.

  // Print the received code.
  Serial.print("Addr ");
  Serial.print(receivedCode.address);

  if (receivedCode.groupBit) {
    Serial.print(" group");
  } 
  else {
    Serial.print(" unit ");
    Serial.print(receivedCode.unit);
  }

  switch (receivedCode.switchType) {
    case NewRemoteCode::off:
    Serial.print(" off");
    break;
    case NewRemoteCode::on:
    Serial.print(" on");
    break;
    case NewRemoteCode::dim:
    Serial.print(" dim level ");
    Serial.print(receivedCode.dimLevel);
    break;
    case NewRemoteCode::on_with_dim:
    Serial.print(" on with dim level ");
    Serial.print(receivedCode.dimLevel);
    break;
  }

  Serial.print(", period: ");
  Serial.print(receivedCode.period);
  Serial.println("us.");
}




