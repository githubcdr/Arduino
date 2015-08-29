/*
 * Author: githubcdr
 * Description: Send and receive 433 messages via MySensors framework
 */

#include <SPI.h>
#include <MySensor.h>
#include <NewRemoteTransmitter.h>

//project
#define SN "RF433Sender"
#define SV "1.0.1"

//children
#define CHILD_ID_RF433_TRANSMITTER 0

//pins
#define RF433_TRANSMITTER_DIGITAL_PIN 8

MySensor gw;
MyMessage msgTransmit(CHILD_ID_RF433_TRANSMITTER, V_VAR1);

void setup() {
  gw.begin(incomingMessage);
  gw.sendSketchInfo(SN, SV);
  gw.present(CHILD_ID_RF433_TRANSMITTER, S_IR);
}

void loop() {
  gw.process();
}

void incomingMessage(const MyMessage &message) {
  if (message.type == V_VAR1 ) {
    unsigned long address = getValue(message.data, '/', 0).toInt();
    byte unit = getValue(message.data, '/', 1).toInt();
    String payload = getValue(message.data, '/', 2);

    Serial.println(address);
    Serial.println(unit);
    Serial.println(payload);

    NewRemoteTransmitter transmitter(address, RF433_TRANSMITTER_DIGITAL_PIN, 260, 5);
    if (payload.equalsIgnoreCase( "off" ) || payload.equalsIgnoreCase( "on" ) ) {
      transmitter.sendUnit(unit, (payload.equalsIgnoreCase( "on" ) ) ? true : false);
    } else {
      transmitter.sendDim(unit, payload.toInt());
    }

    // transmitter.sendGroup(false);
  }
}

String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {
    0, -1
  };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
