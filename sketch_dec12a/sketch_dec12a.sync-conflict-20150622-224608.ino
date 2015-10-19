#include <SPI.h>
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
int tranmitterPin = 8;

char message_buff[100];

// Classes
PubSubClient client;
EthernetClient ethClient;

ActionTransmitter actionTransmitter(tranmitterPin);
NewRemoteTransmitter transmitter(123, 11, 260, 3);


void setup()
{
  Serial.begin(9600);
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
    client.subscribe("/control/arduino");
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
void callback(char* topic, byte* payload, unsigned int length) {
  int i = 0;

  //Serial.println("Message arrived:  topic: " + String(topic));
  //Serial.println("Length: " + String(length,DEC));

  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';

  String msgString = String(message_buff);
//  Serial.println("Payload: " + msgString);

  if (msgString.equals("switch/12/c/on")) {
    digitalWrite(ledPin, HIGH);
    actionTransmitter.sendSignal(12,'C',true);
  } 

  if (msgString.equals("switch/12/c/off")) {
    digitalWrite(ledPin, LOW);
    actionTransmitter.sendSignal(12,'C',false);
  }

  /*
  if (msgString.equals("/control/arduino-1/temperature")) {
   String pubString = String(temperature, 2);
   pubString.toCharArray(message_buff, pubString.length()+1);
   client.publish("/value/arduino-1/temperature/", message_buff);
   }
   
   if (msgString.equals("/control/arduino-1/humidity")) {
   String pubString = "/sensor/humidity/" + String(humidity, 2);
   pubString.toCharArray(message_buff, pubString.length()+1);
   client.publish("arduino", message_buff);
   }
   
   if (msgString.equals("/control/arduino-1/light")) {
   String pubString = "/sensor/light/" + String( LDRReading );
   pubString.toCharArray(message_buff, pubString.length()+1);
   client.publish("arduino", message_buff);
   }
   */
}






