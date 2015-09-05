#include <EnableInterrupt.h>

#include <MySigningNone.h>
#include <MyTransportRFM69.h>
#include <MyTransportNRF24.h>
#include <MyHwATMega328.h>
#include <MySigningAtsha204Soft.h>
#include <MySigningAtsha204.h>

#include <MySensor.h>
#include <SPI.h>
#include <DHT.h>
#include <Bounce2.h>
#include <SFE_BMP180.h>    //get it here: https://github.com/LowPowerLab/SFE_BMP180
#include <SI7021.h>        //get it here: https://github.com/LowPowerLab/SI7021
#include <Wire.h>

#define REPORT_INTERVAL 9000
#define ALTITUDE 220
#define BUTTON 3 //cannot change because of interrupt
#define MOISTURE A4
#define DOOR 5
#define WINDOW_ONE 6
#define WINDOW_TWO 7

#define CHILD_TEMPERATURE 1
#define CHILD_HUMIDITY 2
#define CHILD_PRESSURE 3
#define CHILD_MOISTURE 4
#define CHILD_DOOR 5
#define CHILD_WINDOW_ONE 6
#define CHILD_WINDOW_TWO 7
#define CHILD_POWER 9

SI7021 sensor;
SFE_BMP180 pressure;
int BATTERY_SENSE_PIN  = A7;  // select the input pin for the battery sense point

MyTransportRFM69 transport;
// Hardware profile
MyHwATMega328 hw;

MySensor gw(transport, hw /*, signer*/);

// Change to V_LIGHT if you use S_LIGHT in presentation below
MyMessage temperatureMessage(CHILD_TEMPERATURE, V_TEMP);
MyMessage humidityMessage(CHILD_HUMIDITY, V_HUM);
MyMessage pressureMessage(CHILD_PRESSURE, V_PRESSURE);
MyMessage moistureMessage(CHILD_MOISTURE, V_HUM);
MyMessage doorMessage(CHILD_DOOR, V_TRIPPED);
MyMessage windowOneMessage(CHILD_WINDOW_ONE, V_TRIPPED);
MyMessage windowTwoMessage(CHILD_WINDOW_TWO, V_TRIPPED);
MyMessage powerMessage(CHILD_POWER, V_VOLTAGE);

unsigned long lastReport = 0, time, buttonStart = 0, buttonFinish = 0, lastCheck = 0, lastReduce = 0;

void setup()
{
  pinMode(BATTERY_SENSE_PIN, INPUT);
  pinMode(DOOR, INPUT_PULLUP);
  pinMode(WINDOW_ONE, INPUT_PULLUP);
  pinMode(WINDOW_TWO, INPUT_PULLUP);
  sensor.begin();
  pressure.begin();
  gw.begin(NULL, 7, false);
  gw.sendSketchInfo("Water control", "1.0");
  delay(250);
  gw.present(CHILD_TEMPERATURE, S_TEMP);
  delay(250);
  gw.present(CHILD_HUMIDITY, S_HUM);
  delay(250);
  gw.present(CHILD_MOISTURE, S_HUM);
  delay(250);
  gw.present(CHILD_PRESSURE, S_BARO);
  delay(250);
  gw.present(CHILD_DOOR, S_DOOR);
  delay(250);
  gw.present(CHILD_WINDOW_ONE, S_DOOR);
  delay(250);
  gw.present(CHILD_WINDOW_TWO, S_DOOR);
  delay(250);
  gw.present(CHILD_POWER, S_POWER);
  enableInterrupt(DOOR, notifyDoor, CHANGE);
  enableInterrupt(WINDOW_ONE, notifyWindowOne, CHANGE);
  enableInterrupt(WINDOW_TWO, notifyWindowTwo, CHANGE);
}
volatile int doorInterrupt = 0, windowOneInterrupt = 0, windowToInterrupt = 0;

void notifyDoor() {
  //  gw.send(doorMessage.set(digitalRead(DOOR)));
  doorInterrupt++;
}

void notifyWindowOne() {
  //  gw.send(windowOneMessage.set(digitalRead(WINDOW_ONE)));
  windowOneInterrupt++;
}

void notifyWindowTwo() {
  //  gw.send(windowTwoMessage.set(digitalRead(WINDOW_TWO)));
  windowToInterrupt++;
}
//  Check if digital input has changed and send in new value
void loop()
{
  // Get the update value
  gw.process();
  if (doorInterrupt > 0) {
    gw.send(doorMessage.set(digitalRead(DOOR)));
    doorInterrupt = 0;
  }
  if (windowOneInterrupt > 0) {
    gw.send(windowOneMessage.set(digitalRead(WINDOW_ONE)));
    windowOneInterrupt = 0;
  }
  if (windowToInterrupt > 0) {
    gw.send(windowTwoMessage.set(digitalRead(WINDOW_TWO)));
    windowToInterrupt = 0;
  }

  //*************** READING BATTERY VOLTAGE *********************
  //turn MOSFET ON and read voltage, should give a valid reading
  pinMode(A3, OUTPUT);
  digitalWrite(A3, LOW);
  Serial.print("  BATT: ");
  int sensorValue = analogRead(A7);
  Serial.println(sensorValue);
  float batteryV  = sensorValue / 10 * 0.00520430107;
  int batteryPcnt = sensorValue / 100;
  gw.send(powerMessage.set(batteryV, 1));
  gw.sendBatteryLevel(batteryPcnt);
  pinMode(A3, INPUT); //put A3 in HI-Z mode (to allow mosfet gate pullup to turn it OFF)
  //*************** READING BATTERY VOLTAGE *********************
  float moistureReading = analogRead(MOISTURE);
  Serial.println(moistureReading);
  gw.send(moistureMessage.set(100 * (1024 - moistureReading) / 1024.0, 1));

  Serial.println("************ BMP180 *********************************");
  Serial.print("provided altitude: ");
  Serial.print(ALTITUDE, 0);
  Serial.print(" meters, ");
  Serial.print(ALTITUDE * 3.28084, 0);
  Serial.println(" feet");
  /*
    float temperature = sensor.getCelsiusHundredths() / 100;
    Serial.println("************ Si7021 *********************************");
    Serial.print("C: "); Serial.print(temperature);
    int humidity = sensor.getHumidityPercent();
    Serial.print("   H: "); Serial.print(humidity); Serial.print("%   ");
    gw.send(temperatureMessage.set(temperature, 1));
    gw.send(humidityMessage.set(humidity, 1));
    char status;
    double T, P, p0, a;
    status = pressure.startTemperature();
    if (status != 0)
    {
      // Wait for the measurement to complete:
      delay(status);

      // Retrieve the completed temperature measurement:
      // Note that the measurement is stored in the variable T.
      // Function returns 1 if successful, 0 if failure.

      status = pressure.getTemperature(T);
      if (status != 0) {
        Serial.print("C: ");
        Serial.print(T, 2);
        Serial.print("    F:");
        Serial.print((9.0 / 5.0)*T + 32.0, 2);
        Serial.println("");
        status = pressure.startPressure(3);

        if (status != 0)
        {
          delay(status);
          status = pressure.getPressure(P, T);
          if (status != 0)
          {
            // Print out the measurement:
            Serial.print("abs pressure: ");
            Serial.print(P, 2);
            Serial.print(" mb, ");
            Serial.print(P * 0.0295333727, 2);
            Serial.println(" inHg");

            // The pressure sensor returns abolute pressure, which varies with altitude.
            // To remove the effects of altitude, use the sealevel function and your current altitude.
            // This number is commonly used in weather reports.
            // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
            // Result: p0 = sea-level compensated pressure in mb

            p0 = pressure.sealevel(P, ALTITUDE); // we're at 1655 meters (Boulder, CO)
            Serial.print("relative (sea-level) pressure: ");
            Serial.print(p0, 2);
            Serial.print(" mb, ");
            Serial.print(p0 * 0.0295333727, 2);
            Serial.println(" inHg");
            gw.send(pressureMessage.set(p0, 1));
          }
        }
      }
    }
    */
  gw.sleep(REPORT_INTERVAL);
}
