#include "battleship_client.h"


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Setup");

  // connect to other player 
  initalizeCommunication();
  // Register for a callback function that will be called when data is received
  registerCallback();

  // test all LEDs
  ledInitalize();

  //Initialize Display / LVGL
  initializeDisplay();
  Serial.println("Display ok");

}

void loop() {
  // put your main code here, to run repeatedly:

}
