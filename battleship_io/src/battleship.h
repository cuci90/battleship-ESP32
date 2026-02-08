#ifndef BATTLESHIP_H
#define BATTLESHIP_H

// Define the size of the battleground
#define BOARD_SIZE 10
// Number of ships according to the mumber of ships in SHIP_SIZES[NUM_SHIPS] - Max 5 are currently supported
#define NUM_SHIPS 5

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

/*Set to your screen resolution*/
#define TFT_HOR_RES   320
#define TFT_VER_RES   240

#include <lvgl.h>
#include <SPI.h>
#if LV_USE_TFT_ESPI
#include <TFT_eSPI.h>
#endif
#include <XPT2046_Touchscreen.h>



extern String coordinates;
extern char orientation;
extern bool message_received;
extern bool send_status_received;
extern const char* coordinates_opponent;
extern const char* orientation_opponent;
extern int coordX;
extern int coordY;
extern int use_rockets; // neu

typedef struct struct_message {
    char screen_name[16];
    char text[32];
    char orientation[8];
    int coordX;
    int coordY;
} struct_message;



// Data structure to represent a ship
struct Ship {
  char orientation;  // 'H' for horizontal, 'V' for vertical
  int size;
  int hits;
  int startX;
  int startY;
};

// Data structure to represent a player
struct Player {
  Ship ships[NUM_SHIPS];
  char battlefield[BOARD_SIZE][BOARD_SIZE];
  int rockets_ammo = 3; // neu
};

// Display functions
void my_touchpad_read( lv_indev_t * indev, lv_indev_data_t * data );
void initializeDisplay();
void my_disp_flush( lv_display_t *disp, const lv_area_t *area, uint8_t * px_map);
void create_label(const char *coordinate_label_text , lv_obj_t *screen);
void create_textarea();
void lv_battleship_keyboard_alpha();
void lv_battleship_keyboard_num();
static void ta_event_cb(lv_event_t * e);
void loadScreen1(String shipName = "");
void loadScreen2(const char* result);
void loadScreen3(String shipName);
void shipStatus(const char* shipStatus, int abstandX =0 , int abstandY =0);
void clearScreen();
void loadScreenChooseWeapon(int rockets_ammo); //neu

void initalizeCommunication();
void sendMessage(char screen_name[], char text[] =" ", int coordX = 0, int coordY = 0);
void registerCallback();

void ledInitalize();
void lightUpHitLED(int shotX, int shotY);
void lightUpPreviewLED(int shotX, int shotY);
void lightUpMissedLED(int shotX, int shotY);


#endif