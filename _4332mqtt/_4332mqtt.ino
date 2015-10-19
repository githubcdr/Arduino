#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <RemoteTransmitter.h>
#include <NewRemoteReceiver.h>

byte MAC_ADDRESS[] = { 
  0x90, 0xA2, 0xDA, 0x0D, 0x31, 0xB8 };
char server[] = "mqtt.eigenhuis.lan";
char message_buff[100];
int transmitterPin = 8;

PubSubClient client;
EthernetClient ethClient;

void setup() {
  Serial.begin(115200);
  Serial.println("booting skynet...");
  // Initialize receiver on interrupt 0 (= digital pin 2), calls the callback "showCode"
  // after 2 identical codes have been received in a row. (thus, keep the button pressed
  // for a moment)
  //
  // See the interrupt-parameter of attachInterrupt for possible values (and pins)
  // to connect the receiver.
  if (Ethernet.begin(MAC_ADDRESS) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    return;
  }
  Serial.print(Ethernet.localIP());
  client = PubSubClient(server, 1883, callback, ethClient );

  NewRemoteReceiver::init(0, 2, showCode);
}

void loop() {
  if (!client.connected())
  {
    // clientID, username, MD5 encoded password
    client.connect("arduino");
    client.publish("/log/arduino", "starting arduino kaku gateway");
    client.subscribe("/control/arduino/#");
  }

  client.loop();
}

// Callback function is called only when a valid code is received.
void showCode(NewRemoteCode receivedCode) {
  // Note: interrupts are disabled. You can re-enable them if needed.
  // Print the received code.
  //String pubCode = String(receivedCode.address + receivedCode.unit + receivedCode.switchType + receivedCode.dimLevel);
  String pubCode = String(receivedCode.address + receivedCode.unit );
  pubCode.toCharArray(message_buff, pubCode.length()+1);
  client.publish("/log/arduino", message_buff );
}

String getValue(String data, char separator, int index){
  int found = 0;
  int strIndex[] = {
    0, -1                        };
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
      found++;
      strIndex[0] = strIndex[1]+1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void callback(char* topic_raw, byte* payload_raw, unsigned int length_raw) {
  int i = 0;
  for(i=0; i<length_raw; i++){
    message_buff[i] = payload_raw[i];
  }
  message_buff[i] = '\0';

  String topic = String(topic_raw);
  String payload = String(message_buff);

  if( topic == "/control/arduino/actiontransmitter" || topic == "/control/arduino/kakutransmitter" ){
    /*
    /control/arduino/actiontransmitter 231337/0/on
     parse payload, exploded by '/', addressID = 231337, unitID = 0 and whatever = on
     */

    unsigned long int addressID = getValue(payload, '/', 0).toInt();
    String param2 = getValue(payload, '/', 1);
    String command = getValue(payload, '/', 2);

    param2.toUpperCase();
    char unitID = param2.charAt(0);

    /*
    Serial.println("--");
     Serial.println(addressID);
     Serial.println(unitID);
     Serial.println(command);
     Serial.println("--");
     */

    if( topic == "/control/arduino/actiontransmitter"){
      // /control/arduino/actiontransmitter 12/c/on
      ActionTransmitter actionTransmitter(transmitterPin);

      if( command == "on" ){
        actionTransmitter.sendSignal(addressID, unitID, true);
      }

      if( command == "off" ){
        actionTransmitter.sendSignal(addressID, unitID, false);
      }
    }
  }
}




