#include "battleship_client.h"

#include <esp_now.h>
#include <WiFi.h>

bool message_received = false;
bool send_status_received = false;
const char *coordinates_opponent;

// REPLACE WITH YOUR RECEIVER MAC Address
// own MAC: D4:8A:FC:A7:A4:E4
uint8_t broadcastAddress[] = {0xD4, 0x8A, 0xFC, 0xA7, 0xCA, 0x50};
esp_now_peer_info_t peerInfo;

struct_message outGoingMessage;
struct_message incomingReadings;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");

  if (status == ESP_NOW_SEND_SUCCESS)
  {
    Serial.println("Delivery Success");
    send_status_received = true;
  }
  else
  {
    Serial.println("Delivery Fail");
    send_status_received = false;
  }

}

void initalizeCommunication()
{
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
};

void sendMessage(char screen_name[], char text[], char orientation[], int coordX, int coordY)
{

  // Copy screen name to outGoingMessage.screen_name - conversion from char to char[16]

  outGoingMessage.orientation[0] = orientation[0];
  Serial.println("beim senden <<<<<<<<<<<<<<<<");
  Serial.println(outGoingMessage.orientation[0]);
  outGoingMessage.coordX = coordX;
  outGoingMessage.coordY = coordY;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&outGoingMessage, sizeof(outGoingMessage));

  if (result == ESP_OK)
  {
    Serial.println("Sent with success");
  }
  else
  {
    Serial.println("Error sending the data");
  }
};


// Callback when data is received
void OnDataRecv(const esp_now_recv_info_t * mac, const uint8_t *incomingData, int len)
{
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  // Serial.print("Bytes received: ");
  // Serial.println(len);
  // Serial.println("Koordinaten: ");
  // Serial.println(incomingReadings.coordinates_opponent);
  // Serial.println("Text: ");
  // Serial.println(incomingReadings.text);
  //   Serial.println("screenname: ");
  // Serial.println(incomingReadings.screen_name);
  // Determine which screen to display
  if (strcmp(incomingReadings.screen_name, "ship_setup") == 0)
  {
    // Serial.println("Show buttons + coordinates screen");
    //loadScreen3(incomingReadings.text);
    ship_setup(incomingReadings.text);
  }
  else if (strcmp(incomingReadings.screen_name, "hit") == 0)
  { // not needed
    // Serial.println("Show hit screen");
    // Get orientation & coordinates from player 2
  }
  else if (strcmp(incomingReadings.screen_name, "show_text") == 0)
  {
    // clearScreen();
    // just show the text on the screen
    updateText(incomingReadings.text);
    //loadScreen2(incomingReadings.text);
    //clearScreen();
  }
  else if (strcmp(incomingReadings.screen_name, "coordinates") == 0)
  {

    // clearScreen();
     provide_coordinates();
    //loadScreen1();
    // Serial.println("Show Coordinate screen");
    
    // Get orientation & coordinates from player 2
  }
  else if (strcmp(incomingReadings.screen_name, "PreviewLED") == 0)
  {
   
    // new logic

    // Serial.println("Shots: ");
    // Serial.println(incomingReadings.coordX);
    // Serial.println(incomingReadings.coordY);

    lightUpPreviewLED(incomingReadings.coordX, incomingReadings.coordY);
  }
  else if (strcmp(incomingReadings.screen_name, "HitLED") == 0)
  {
    // Serial.println("Show Hit LED on the LED matrix");
  
    // new logic
    lightUpHitLED(incomingReadings.coordX, incomingReadings.coordY);
  }
  else if (strcmp(incomingReadings.screen_name, "MissedLED") == 0)
  {
   
    //  Serial.println("Missed Hit LED on the LED matrix");
    // new logic
    lightUpMissedLED(incomingReadings.coordX, incomingReadings.coordY);
  }
  else if (strcmp(incomingReadings.screen_name, "ship_status") == 0)
  {

    // char *status;

    // Using strtok to split the string
    // status = strtok(incomingReadings.text, ",");

    // int abstandX = -40;
    // int abstandY = -60;
    // int i = 0;

    update_shipStatus(incomingReadings.text);

   

    // while (status != NULL)
    // {

    //   Serial.println(status);
    //   if (i == 2 || i == 4)
    //   {
    //     // start new line
    //     abstandY += 20;
    //     abstandX = -50; // little bit left from the start, so it is centrally aligned
    //   }

    //   shipStatus(status, abstandX, abstandY);
    //   abstandX += 60;

    //   status = strtok(NULL, ",");
    //   i++;
    // }
    
  }

  else
  {
    // this should not happen
    // Serial.println("Message could not be handled/resolved");
    // Serial.println(incomingReadings.screen_name);
    // Serial.println(incomingReadings.text);
  }


}

void registerCallback()
{
  esp_now_register_recv_cb(OnDataRecv);
}
