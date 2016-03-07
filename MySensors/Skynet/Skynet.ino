// Enable debug prints to serial monitor
//#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

// Enable repeater functionality for this node
//#define MY_REPEATER_FEATURE

#include <SPI.h>
#include <MySensor.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

// project
#define SN "Skynet"
#define SV "2.0.1"

// children
#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_LIGHT 2
#define CHILD_ID_IS_DARK 3
#define CHILD_ID_MOTION 4
#define CHILD_ID_MOISTURE 5
#define CHILD_ID_UV 6
#define CHILD_ID_BARO 7
#define CHILD_ID_SOUND 8

// pins
#define MOTION_SENSOR_DIGITAL_PIN 3
#define HUMIDITY_SENSOR_DIGITAL_PIN 4
#define LIGHT_SENSOR_DIGITAL_PIN 5
#define LIGHT_SENSOR_ANALOG_PIN 0
#define SOIL_SENSOR_ANALOG_PIN 1
#define UV_SENSOR_ANALOG_PIN 2
#define SOUND_SENSOR_ANALOG_PIN 6
#define INTERRUPT MOTION_SENSOR_DIGITAL_PIN-2

// misc
int uvIndexValue [13] = { 50, 227, 318, 408, 503, 606, 696, 795, 881, 976, 1079, 1170, 3000};
int uvIndex;
boolean metric = true;
float lastHum;
float lastTemp;
boolean lastTripped = true;
boolean lastDark = true;
int lastUV = -1;
int lastLightLevel;
int lastSoilValue;
int lastSoundLevel;
float lastPressure = -1;

// init
DHT dht;
//unsigned long SLEEP_TIME = dht.getMinimumSamplingPeriod(); // Sleep time between reads (in milliseconds)
unsigned long SLEEP_TIME = 60000;
Adafruit_BMP085 bmp = Adafruit_BMP085();

//MySensor gw;
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgLight(CHILD_ID_LIGHT, V_LIGHT_LEVEL);
MyMessage msgDark(CHILD_ID_IS_DARK, V_LIGHT);
MyMessage msgMotion(CHILD_ID_MOTION, V_TRIPPED);
MyMessage msgMoisture(CHILD_ID_MOISTURE, V_VAR1);
MyMessage msgUv(CHILD_ID_UV, V_UV);
MyMessage msgBaro(CHILD_ID_BARO, V_PRESSURE);
MyMessage msgSound(CHILD_ID_SOUND, V_VOLUME);

void setup()
{
  metric = getConfig().isMetric;
  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN);
  bmp.begin();
  pinMode(MOTION_SENSOR_DIGITAL_PIN, INPUT);
  pinMode(LIGHT_SENSOR_DIGITAL_PIN, INPUT);
}

void presentation() {
  sendSketchInfo(SN, SV);
  present(CHILD_ID_HUM, S_HUM);
  present(CHILD_ID_TEMP, S_TEMP);
  present(CHILD_ID_LIGHT, S_LIGHT_LEVEL);
  present(CHILD_ID_IS_DARK, S_LIGHT);
  present(CHILD_ID_MOTION, S_MOTION);
  present(CHILD_ID_MOISTURE, S_CUSTOM);
  present(CHILD_ID_UV, S_UV);
  present(CHILD_ID_BARO, S_BARO);
  present(CHILD_ID_SOUND, S_POWER);
}

void loop()
{
  //unsigned long currentMillis = millis();
  //delay(dht.getMinimumSamplingPeriod());

  boolean tripped = digitalRead(MOTION_SENSOR_DIGITAL_PIN);
  if (tripped != lastTripped) {
    send(msgMotion.set(tripped ? "1" : "0")); // Send tripped value to gw
    lastTripped = tripped;
  }

  boolean dark = digitalRead(LIGHT_SENSOR_DIGITAL_PIN);
  if (dark != lastDark) {
    send(msgDark.set(dark ? "0" : "1")); // Send tripped value to gw INVERSE since DARKNESSS MUAHAHHahahahahahahaha
    lastDark = dark;
  }

  //  if (currentMillis - previousMillis > SLEEP_TIME) {
  //    previousMillis = currentMillis;

  int lightLevel = (1023 - analogRead(LIGHT_SENSOR_ANALOG_PIN)) / 10.23;
  if (lightLevel != lastLightLevel) {
    send(msgLight.set(lightLevel));
    lastLightLevel = lightLevel;
  }

  int soundLevel = analogRead(SOUND_SENSOR_ANALOG_PIN);
  //Serial.println(soundLevel);
  if (soundLevel != lastSoundLevel) {
    send(msgSound.set(soundLevel));
    lastSoundLevel = soundLevel;
  }

  float temperature = dht.getTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed reading temperature from DHT");
  } else if (temperature != lastTemp) {
    lastTemp = temperature;
    if (!metric) {
      temperature = dht.toFahrenheit(temperature);
    }
    send(msgTemp.set(temperature, 1));
  }

  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
    Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum) {
    send(msgHum.set(humidity, 1));
    lastHum = humidity;
  }

  int soilsensorValue = analogRead(SOIL_SENSOR_ANALOG_PIN);
  if (soilsensorValue != lastSoilValue) {
    send(msgMoisture.set(soilsensorValue));
    lastSoilValue = soilsensorValue;
  }
  //  }

  uint16_t uv = analogRead(UV_SENSOR_ANALOG_PIN);
  for (int i = 0; i < 13; i++) {
    if (uv <= uvIndexValue[i])
    {
      uvIndex = i;
      break;
    }
  }
  if (uvIndex != lastUV) {
    send(msgUv.set(uvIndex));
    lastUV = uvIndex;
  }

  float pressure = bmp.readSealevelPressure(-2) / 100; // -2 meters above sealevel
  if (pressure != lastPressure) {
    send(msgBaro.set(pressure, 0));
    lastPressure = pressure;
  }

  smartSleep(INTERRUPT, CHANGE, SLEEP_TIME); //sleep a bit
}
