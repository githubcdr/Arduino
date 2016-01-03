#include <SPI.h>
#include <DHT11.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <RemoteTransmitter.h>
#include <NewRemoteTransmitter.h>

// Network
byte MAC_ADDRESS[] = { 
  0x90, 0xA2, 0xDA, 0x0D, 0x31, 0xB8 };
char server[] = "mqtt.eigenhuis.lan";

// declarations
// pins
int DHT11Pin = 7;
int transmitterPin = 8;
int ledPin = 9;
int LDR_Pin = A0; //analog pin 0

// timing
unsigned long currentTime;
unsigned long loopTimeA;
unsigned long loopTimeB;

// internals
int LDRReading;
float temperature, humidity;
char message_buff[100];

// Classes
PubSubClient client;
EthernetClient ethClient;
DHT11 dht11(DHT11Pin); 

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
}

void loop()
{
  currentTime = millis();

  if (!client.connected())
  {
    // clientID, username, MD5 encoded password
    client.connect("arduino");
    client.publish("/log/arduino", "starting arduino kaku gateway");
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
    //String unitID = getValue(payload, '/', 1);
    String command = getValue(payload, '/', 2);

    int unitID = 0;

    if( topic == "/control/arduino/actiontransmitter"){
      // /control/arduino/actiontransmitter 12/c/on
      ActionTransmitter actionTransmitter(transmitterPin);
      if( command == "on" ){
        digitalWrite(ledPin, HIGH);
        actionTransmitter.sendSignal(addressID, unitID, true);
      }

      if( command == "off" ){
        actionTransmitter.sendSignal(addressID, unitID, false);
      }
    }

    if(topic == "/control/arduino/kakutransmitter"){
      Serial.println(addressID + unitID+command);

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

      /*
     if( payload == "231337/0/on" ){
       digitalWrite(ledPin, HIGH);
       NewRemoteTransmitter transmitter(231337, tranmitterPin, 260, 3);
       transmitter.sendUnit(0, true);
       }
       
       if( payload == "231337/0/off" ){
       digitalWrite(ledPin, LOW);
       NewRemoteTransmitter transmitter(231337, tranmitterPin, 260, 3);
       transmitter.sendUnit(0, false);
       }
       
       if( payload == "231337/0/5" ){
       NewRemoteTransmitter transmitter(231337, tranmitterPin, 260, 3);
       transmitter.sendDim(0, 5);
       }
       */

      /*
    NewRemoteTransmitter transmitter(231337, tranmitterPin, 260, 3);
       transmitter.sendUnit(0, false);
       delay(3000);
       transmitter.sendDim(0, 0);
       delay(3000);
       transmitter.sendGroup(false);
       delay(3000);
       transmitter.sendDim(0, 14);
       delay(3000);
       */
    }
    /*
  }
     
     
     
    /*
     if (msgString.equals("switch/12/c/on")) {
     digitalWrite(ledPin, HIGH);
     actionTransmitter.sendSignal(12,'C',true);
     } 
     
     if (msgString.equals("switch/12/c/off")) {
     digitalWrite(ledPin, LOW);
     actionTransmitter.sendSignal(12,'C',false);
     */
  }
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {
    0, -1              };
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












