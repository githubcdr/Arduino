#include <Bounce2.h>

#define BUTTON_PIN 2
#define LED_PIN 13
#define PULSE_FACTOR 1000 

// Instantiate a Bounce object
Bounce debouncer = Bounce();

volatile unsigned long pulseCount = 0;   
volatile unsigned long lastBlink = 0;
volatile unsigned long watt = 0;

double ppwh = ((double)PULSE_FACTOR)/1000;
void setup() {

  Serial.begin(57600);

  // Setup the button
  pinMode(BUTTON_PIN, INPUT);
  // Activate internal pull-up
  digitalWrite(BUTTON_PIN, HIGH);

  // After setting up the button, setup debouncer
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);

  //Setup the LED
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Update the debouncer and get the changed state
  boolean changed = debouncer.update();

  if ( changed ) {
    int value = debouncer.read();

    if ( value == HIGH ) {
      unsigned long newBlink = micros();
      unsigned long interval = newBlink - lastBlink;
      if (interval < 10000L) { // Sometimes we get interrupt on RISING
        return;
      }
      watt = (3600000000.0 / interval) / ppwh;
      lastBlink = newBlink;

      digitalWrite(LED_PIN, HIGH );
      pulseCount++;
      
      Serial.println(watt);
    } else {
      digitalWrite(LED_PIN, LOW );
    }
  }
  
  if( pulseCount == 10 ){
    Serial.println("1kWh");
    pulseCount = 0;
  }
}

