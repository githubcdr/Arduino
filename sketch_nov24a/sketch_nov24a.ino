#include <FastLED.h>
 // change these to match your hardware setup
 
#define LED_PIN 6                          // hardware SPI pin SCK
#define NUM_LEDS 16
#define COLOR_ORDER RGB
#define LED_TYPE NEOPIXEL
#define MAX_BRIGHTNESS 255                   // watch the power!
 
struct CRGB leds[NUM_LEDS];
 
void setup()
{
    LEDS.addLeds<LED_TYPE, LED_PIN>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(MAX_BRIGHTNESS);
    FastLED.clear();
    // fill_solid(leds, NUM_LEDS, CRGB::Yellow);
    fill_rainbow( leds, NUM_LEDS, 128, 7);
}
 
void loop () {
  
 float breath = (exp(sin(millis()/2000.0*PI)) - 0.36787944)*108;
 //FastLED.setBrightness(breath);
 fill_rainbow( leds, NUM_LEDS, breath, 7 );
 //if( random8() < 95) {
    //leds[ random16(NUM_LEDS) ] += CRGB::White;
  //}  

 FastLED.show();
 //delay(30);
}

