#include "battleship.h"

#include <esp_now.h>
#include <WiFi.h>

bool message_received = false;
bool send_status_received = false;
const char* coordinates_opponent;
const char* orientation_opponent;
int coordX;
int coordY;


// REPLACE WITH YOUR RECEIVER MAC Address
//own MAC: D4:8A:FC:A7:CA:50
// client MAC: D4:8A:FC:A7:A4:E4
uint8_t broadcastAddress[] = {0xD4, 0x8A, 0xFC, 0xA7, 0xA4, 0xE4};
esp_now_peer_info_t peerInfo;

struct_message outGoingMessage;
struct_message incomingReadings;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");

  if (status == ESP_NOW_SEND_SUCCESS) {
  Serial.println("Delivery Success");
  send_status_received = true;
  } else {
    Serial.println("Delivery Fail");
    send_status_received = false;
  }
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


void initalizeCommunication(){
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
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
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
};

void sendMessage(char screen_name[], char text[],  int coordX, int coordY){

    // Copy screen name to outGoingMessage.screen_name - conversion from char to char[16]
  strncpy(outGoingMessage.screen_name, screen_name, sizeof(outGoingMessage.screen_name) - 1);

  // Copy text to outGoingMessage.text
  strncpy(outGoingMessage.text, text, sizeof(outGoingMessage.text) - 1);


  outGoingMessage.coordX = coordX;
  outGoingMessage.coordY = coordY;
       // Send message via ESP-NOW
   esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outGoingMessage, sizeof(outGoingMessage));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

};

// Callback when data is received
//void OnDataRecv(const esp_now_recv_info_t * mac, const uint8_t *incomingData, int len) {
void OnDataRecv(const esp_now_recv_info_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);

  //new logic
  Serial.println("int data recived");
  Serial.println(coordX);
  Serial.println(coordY);
  orientation_opponent = incomingReadings.orientation;
  coordX = incomingReadings.coordX;
  coordY = incomingReadings.coordY;


  message_received = true;

  

};

void registerCallback() {
  esp_now_register_recv_cb(OnDataRecv);
};



