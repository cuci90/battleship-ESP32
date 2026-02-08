#include "battleship_client.h"

const char* comm_text = "default";
boolean v_updateText = false;
boolean v_ship_setup = false;
boolean v_provide_coordinates= false;
boolean v_update_shipStatus = false;
const char* v_shipStatus;
// int v_abstandX;
// int v_abstandY;


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

  lv_tick_inc(5);


}

void loop() {

  static uint32_t last_tick = 0;

  // uint32_t now = millis();
  // if (now - last_tick >= 5) {
  //   lv_tick_inc(5);
  //   last_tick = now;
  // }

  //lv_task_handler();
  lv_timer_handler();   
  delay(5);

  // handle all the flags that are set by the callback function when data is received from the other player and call the corresponding functions to update the display
  if (v_updateText == true){
    clearScreen();
    Serial.println("Update Text");
    v_updateText = false;
    loadScreen2(comm_text );
  }

    if (v_ship_setup == true){
    Serial.println("Ship Setup jetzt");
    v_ship_setup = false;
    loadScreen3(comm_text);
  }

      if (v_provide_coordinates == true){
        clearScreen();
    Serial.println("Coordinates");
    v_provide_coordinates = false;
    loadScreen4();
  }

    if (v_update_shipStatus == true){
      v_update_shipStatus = false;


    int i = 0;
    char *status;

    // convert the v_shipStatus to a char array to use strtok
    char buffer[32];                
    strcpy(buffer, v_shipStatus);

    // Using strtok to split the string
    status = strtok(buffer, ",");

    int abstandX = -40;
    int abstandY = -60;
    

     while (status != NULL)
    {
      Serial.println("Ship status hier: ");
      Serial.println(status);


      Serial.println(status);
      if (i == 2 || i == 4)
      {
        // start new line
        abstandY += 20;
        abstandX = -50; // little bit left from the start, so it is centrally aligned
      }

      shipStatus(status, abstandX, abstandY);
      abstandX += 60;

      status = strtok(NULL, ",");
      i++;
    }

      for (int i = 0; i < 500; i++) {
      lv_task_handler(); 
      lv_tick_inc(5); 
      delay(5); 
     }


  }




}



void updateText(const char* text){
  comm_text =text;
  v_updateText = true;
}

void  ship_setup(const char* text){
  comm_text =text;
  v_ship_setup = true;
}


void provide_coordinates() {
  v_provide_coordinates= true;

}


void update_shipStatus(const char* shipStatus) {
  v_update_shipStatus = true;
  v_shipStatus = shipStatus;
  // v_abstandX = abstandX;
  // v_abstandY = abstandY;
}