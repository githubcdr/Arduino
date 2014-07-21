/*
cDR Skynet Arduino mqtt module
*/

#include <SPI.h>
#include <DHT11.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <RemoteTransmitter.h>
#include <NewRemoteTransmitter.h>
#include <NewRemoteReceiver.h>

// Network
byte MAC_ADDRESS[] = { 
  0x90, 0xA2, 0xDA, 0x0D, 0x31, 0xB8 };
char server[] = "mqtt.eigenhuis.lan";

// pins
int DHT11Pin = 7;
int receiverPin = 2;
int transmitterPin = 8;
int ledPin = 9;
int LDR_Pin = A0;

// timing
unsigned long currentTime;
unsigned long loopTimeA;
unsigned long loopTimeB;

// internals
int LDRReading;
float temperature, humidity;
char message_buff[128];

// Classes
DHT11 dht11(DHT11Pin);
EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

void setup()
{
  currentTime = millis();
  loopTimeA = currentTime;
  loopTimeB = currentTime;

  Serial.begin(115200);
  Serial.println("booting skynet...");

  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);

  if (Ethernet.begin(MAC_ADDRESS) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    return;
  }
  Serial.print(Ethernet.localIP());
  client = PubSubClient(server, 1883, callback, ethClient );

  NewRemoteReceiver::init(0, receiverPin, showCode);
}

void loop()
{
  currentTime = millis();

  if (!client.connected())
  {
    // clientID, username, MD5 encoded password
    client.connect("arduino");
    client.publish("/log/arduino", "starting arduino mqtt gateway");
    client.subscribe("/control/arduino/#");
  }

  if(currentTime >= (loopTimeA + 20000)){
    // DHT11 is 1000 milis, cache for 1 sec
    // temperature and humidity measuring
    dht11.read(humidity, temperature);
    //delay(DHT11_RETRY_DELAY); //delay for reread

    // MQTT client loop processing
    LDRReading = analogRead(LDR_Pin);
    loopTimeA = currentTime;
  }

  if(currentTime >= (loopTimeB + 30000)){
    // publish on MQTT topic every 5 minutes
    /*
    c'est ne pas tres elegant et tu?
    */

    String pubStringtemperature = String(temperature, 2);
    pubStringtemperature.toCharArray(message_buff, pubStringtemperature.length()+1);
    client.publish("/value/arduino/temperature/", message_buff);

    String pubStringhumidity = String(humidity, 2);
    pubStringhumidity.toCharArray(message_buff, pubStringhumidity.length()+1);
    client.publish("/value/arduino/humidity/", message_buff);

    String pubStringlight = String(LDRReading);
    pubStringlight.toCharArray(message_buff, pubStringlight.length()+1);
    client.publish("/value/arduino/light/", message_buff);

    loopTimeB = currentTime;
  }

  client.loop();
  if (!client.connected())
    Serial.println("mqtt client died :(");
}

// handles message arrived on subscribed topic(s)
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

    // dont confuse interrupts here!
    NewRemoteReceiver::disable();

    if( topic == "/control/arduino/actiontransmitter"){
      // /control/arduino/actiontransmitter 12/c/on

      ActionTransmitter actionTransmitter(transmitterPin);
      if( command == "on" ){
        digitalWrite(ledPin, HIGH);
        actionTransmitter.sendSignal(addressID, unitID, true);
      }

      if( command == "off" ){
        digitalWrite(ledPin, LOW);
        actionTransmitter.sendSignal(addressID, unitID, false);
      }
    }

    if(topic == "/control/arduino/kakutransmitter"){
      NewRemoteTransmitter transmitter(addressID, transmitterPin, 260, 3);

      if( command == "on" ){
        digitalWrite(ledPin, HIGH);
        transmitter.sendUnit(unitID, true);
      }

      if( command == "off" ){
        digitalWrite(ledPin, LOW);
        transmitter.sendUnit(unitID, false);
      }

      if( command != "off" && command != "on" ){
        transmitter.sendDim(unitID, command.toInt());
      }
    }

    NewRemoteReceiver::enable();
  }
}

String getValue(String data, char separator, int index){
  int found = 0;
  int strIndex[] = {
    0, -1                                };
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

void showCode(NewRemoteCode receivedCode) {
  // Note: interrupts are disabled. You can re-enable them if needed.

  // Print the received code.
//  client.publish("/log/arduino", receivedCode.address );
//noInterrupts();
    String pubStringlight = String( receivedCode.address + receivedCode.unit + receivedCode.switchType );
    pubStringlight.toCharArray(message_buff, pubStringlight.length()+1);
    client.publish("/log/arduino/", message_buff);

/*  
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
      Serial.print(" dim");
      break;
  }

  if (receivedCode.dimLevelPresent) {
    Serial.print(", dim level: ");
    Serial.print(receivedCode.dimLevel);
  }

  Serial.print(", period: ");
  Serial.print(receivedCode.period);
  Serial.println("us.");
*/
}
