/*
 * Author: githubcdr
 * Description: Send and receive 433 messages via MySensors framework
 */

#include <SPI.h>
#include <MySensor.h>
#include <NewRemoteReceiver.h>
#include <NewRemoteTransmitter.h>

//project
#define SN "RF433Tranceiver"
#define SV "1.3.0"

//children
#define CHILD_ID_RF433_RECEIVER 0
#define CHILD_ID_RF433_TRANSMITTER 1

//pins
#define RF433_TRANSMITTER_DIGITAL_PIN 8
#define RF433_RECEIVER_DIGITAL_PIN 3
#define INTERRUPT RF433_RECEIVER_DIGITAL_PIN-2

String combined_received_code;
String lastcombined_received_code;

MySensor gw;
MyMessage msgReceive(CHILD_ID_RF433_RECEIVER, V_IR_SEND);
MyMessage msgTransmit(CHILD_ID_RF433_TRANSMITTER, V_VAR1);

void setup() {
  gw.begin(incomingMessage);
  gw.sendSketchInfo(SN, SV);
  gw.present(CHILD_ID_RF433_RECEIVER, S_IR);
  gw.present(CHILD_ID_RF433_TRANSMITTER, S_IR);

  NewRemoteReceiver::init(INTERRUPT, RF433_RECEIVER_DIGITAL_PIN, showCode);
}

void loop() {
  gw.process();
}

void incomingMessage(const MyMessage &message) {
  //char* getString(char *buffer) const;
  // NewRemoteTransmitter transmitter(231337, TRANSMITTER_PIN, 260, 3);
  // address: 231337
  // unit: 0
  // payload: on,off,0-15
  // shutter 11854542 unit 0 on, period: 251us

  if (message.type == V_VAR1 ) {
    noInterrupts();
    unsigned long address = getValue(message.data, '/', 0).toInt();
    byte unit = getValue(message.data, '/', 1).toInt();
    String payload = getValue(message.data, '/', 2);

    //Serial.println(address);
    //Serial.println(unit);
    //Serial.println(payload);

    NewRemoteTransmitter transmitter(address, RF433_TRANSMITTER_DIGITAL_PIN, 260, 5);
    if (payload.equalsIgnoreCase( "off" ) || payload.equalsIgnoreCase( "on" ) ) {
      transmitter.sendUnit(unit, (payload.equalsIgnoreCase( "on" ) ) ? true : false);
    } else {
      transmitter.sendDim(unit, payload.toInt());
    }

    // transmitter.sendGroup(false);

    interrupts();
  }
}

// Callback function is called only when a valid code is received.
void showCode(NewRemoteCode receivedCode) {
  // Note: interrupts are disabled. You can re-enable them if needed.
  combined_received_code = "";
  combined_received_code += receivedCode.address;

  combined_received_code += "/";
  if (receivedCode.groupBit) {
    combined_received_code += "group";
  } else {
    combined_received_code += receivedCode.unit;
  }

  combined_received_code += "/";
  switch (receivedCode.switchType) {
    case NewRemoteCode::off:
      combined_received_code += "off";
      break;
    case NewRemoteCode::on:
      combined_received_code += "on";
      break;
    case NewRemoteCode::dim:
      combined_received_code += receivedCode.dimLevel;
      break;
    case NewRemoteCode::on_with_dim:
      combined_received_code += receivedCode.dimLevel;
      break;
  }

  // Length (with one extra character for the null terminator)
  int str_len = combined_received_code.length() + 1;

  // Prepare the character array (the buffer)
  char char_array[str_len];

  // Copy it over
  combined_received_code.toCharArray(char_array, str_len);

  if (combined_received_code != lastcombined_received_code) {
    gw.send(msgReceive.set(char_array));
    lastcombined_received_code = combined_received_code;
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

