#include <Adafruit_NeoPixel.h>
#define PIN_LEDstripe 22  // The ESP32 pin GPIO22 connected to WS2812B
#define NUM_PIXELS 100   // The number of LEDs (pixels) on WS2812B LED strip


Adafruit_NeoPixel LEDstripe(NUM_PIXELS, PIN_LEDstripe, NEO_GRB + NEO_KHZ800);


void ledInitalize() {

     // initialize WS2812B 
   
   LEDstripe.begin(); 
   // start simple test
        // turn pixels on
    for (int pixel = 0; pixel < NUM_PIXELS; pixel++) {         // for each pixel
    LEDstripe.setPixelColor(pixel, LEDstripe.Color(0, 120, 0));  // it only takes effect if pixels.show() is called
    LEDstripe.show();                                          // update to the WS2812B Led Strip
    delay(50);  // 500ms pause between each pixel
  }
  delay(500);
  LEDstripe.clear();
  LEDstripe.show();

}

void lightUpHitLED(int shotX, int shotY){


   int ledIndex;

  // Calculate the LED index based on the serpentine pattern
  if (shotY % 2 == 0) {
    // Even row (=shotY) (counting from right to left)
    ledIndex = shotY * 10 + shotX;
  } else {
    // Odd row (=shotY) (counting from left to right)
    ledIndex = shotY * 10 + (9 - shotX);
  }

  // Turn on the specified LED
  LEDstripe.setPixelColor(ledIndex, LEDstripe.Color(120, 0, 0));  // RGB color (in this case, red)
  LEDstripe.show();
  delay(1000);  // Delay for 1 second

}

void lightUpPreviewLED(int shotX, int shotY){


   int ledIndex;

  // Calculate the LED index based on the serpentine pattern
  if (shotY % 2 == 0) {
    // Even row (=shotY) (counting from right to left)
    ledIndex = shotY * 10 + shotX;
  } else {
    // Odd row (=shotY) (counting from left to right)
    ledIndex = shotY * 10 + (9 - shotX);
  }

  // blink 4 times
  for (int i=0; i<10; i++ ){
       // Turn on the specified LED
    LEDstripe.setPixelColor(ledIndex, LEDstripe.Color(120, 120, 0)); //yellow
    LEDstripe.show();
    delay(150);  // Delay for 1 second
    LEDstripe.setPixelColor(ledIndex, LEDstripe.Color(0, 0, 0)); //yellow
    LEDstripe.show();
    delay(150);  
  }

}

void lightUpMissedLED(int shotX, int shotY){


   int ledIndex;

  // Calculate the LED index based on the serpentine pattern
  if (shotY % 2 == 0) {
    // Even row (=shotY) (counting from right to left)
    ledIndex = shotY * 10 + shotX;
  } else {
    // Odd row (=shotY) (counting from left to right)
    ledIndex = shotY * 10 + (9 - shotX);
  }


  // Turn on the specified LED
  LEDstripe.setPixelColor(ledIndex, LEDstripe.Color(0, 120, 0));  //green
  LEDstripe.show();
  delay(1000);  // Delay for 1 second

}