#include "battleship_client.h"

static lv_style_t style_kb;
const int SHIP_SIZES[NUM_SHIPS] = {2, 3, 3, 4, 5};
int ship_index = 0;

char battlefield[BOARD_SIZE][BOARD_SIZE];

const char *shoot_result_text;
const char *coordinate_label_text;
const char *btn_label_horizontal = "Horizontal ---";
const char *btn_label_vertical = "Vertikal |||";
const char *ship_status_text = "Schiff Status";
 lv_obj_t *kb_alpha;
  lv_obj_t *kb_num;

int rockets_ammo = 3; // neu
int use_rockets = 3; // neu - 0 = cannon, 1 = rockets horizontal, 2 = rockets vertical, 3 = not chosen yet

lv_obj_t *ta;
lv_obj_t *screen1;
lv_obj_t *screen2;
lv_obj_t *screen3;
lv_obj_t *screen4; // neu
String coordinates = ""; // here this is including the orientation when setting up the ships
lv_obj_t *label;
lv_obj_t *label_status;
//String orientation="";
char orientation='X';
const char *coordinates_player2;
char no_screen[]  = "no_screen";
char no_text[]  = "no_text";


// Screen variables
SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
static const uint16_t screenWidth = 240;
static const uint16_t screenHeight = 320;

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 /** (LV_COLOR_DEPTH / 8)*/)
uint32_t draw_buf[DRAW_BUF_SIZE / 4];


void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {

  uint16_t touchX, touchY;

  bool touched = (ts.tirqTouched() && ts.touched());
  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    TS_Point p = ts.getPoint();
    touchX = map(p.x, 200, 4000, 1, screenWidth);  /* Touchscreen X calibration */
    touchY = map(p.y, 500, 3750, 1, screenHeight); /* Touchscreen X calibration */
    data->state = LV_INDEV_STATE_PR;

    /*Set the touchpoints*/
    data->point.x = touchX;
    data->point.y = touchY;


  }
}

void initializeDisplay() {

  lv_init();
  lv_display_t *disp;
  #if LV_USE_TFT_ESPI
  /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
  lv_display_set_resolution(disp, 240, 320);
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);

  #else
  /*Else create a display yourself*/
  disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
  #endif
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS); /* Start second SPI bus for touchscreen */
  ts.begin(mySpi);                                                  /* Touchscreen init */
  ts.setRotation(2);                                                /* Landscape orientation */

  /*Initialize the (dummy) input device driver*/
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);
    lv_indev_set_display(indev, disp);

  //create 2 screens: 1 for the keyboards / 1 for shoot result display
  screen1 = lv_obj_create(NULL);
  screen2 = lv_obj_create(NULL);
  screen3 = lv_obj_create(NULL);
  screen4 = lv_obj_create(NULL);


}

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {

  lv_disp_flush_ready(disp);
}

void create_label(const char *coordinate_label_text , lv_obj_t *screen ) {
  //Text label
  label = lv_label_create(screen);
  lv_label_set_text(label, coordinate_label_text);
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);
}

void create_ship_label(const char *ship_status, int abstandX, int abstandY ) {
  //Text label
  label_status = lv_label_create(screen2);
  lv_label_set_text(label_status, ship_status_text);
   lv_obj_set_style_text_font(label_status, &lv_font_montserrat_14, 0);
  lv_obj_align(label_status, LV_ALIGN_BOTTOM_MID, 0, -85);

  label = lv_label_create(screen2);
  lv_label_set_text(label, ship_status);
   lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
  lv_obj_align(label, LV_ALIGN_BOTTOM_MID, abstandX, abstandY);
}


static void btn_event_horizontal(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        Serial.println("Horizontal Button Clicked");
        orientation = 'H';
    }

}
static void btn_event_vertical(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        Serial.println("Vertical Button Clicked");
        orientation = 'V';
    }

}

void create_textarea() {
  ta = lv_textarea_create(screen1);
  lv_obj_align(ta, LV_ALIGN_CENTER, 0, -60);
  lv_obj_set_size(ta, lv_pct(40), 40);
  lv_obj_add_state(ta, LV_STATE_FOCUSED);
  lv_textarea_set_one_line(ta, true);
  lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_VALUE_CHANGED, ta);
}


void lv_battleship_keyboard_alpha() {

  static const char *kb_map[] = { "A", "B", "C", "D", "\n",
                                  "E", "F", "G", "H", "\n",
                                  "I", "J", "\n",
                                  LV_SYMBOL_BACKSPACE, " ", " ", " ", LV_SYMBOL_OK, NULL };


static const lv_btnmatrix_ctrl_t kb_ctrl[] = { (lv_btnmatrix_ctrl_t)8, (lv_btnmatrix_ctrl_t)8, (lv_btnmatrix_ctrl_t)8, (lv_btnmatrix_ctrl_t)8,
                                                    (lv_btnmatrix_ctrl_t)8, (lv_btnmatrix_ctrl_t)8, (lv_btnmatrix_ctrl_t)8, (lv_btnmatrix_ctrl_t)8,
                                                    (lv_btnmatrix_ctrl_t)8, (lv_btnmatrix_ctrl_t)8,
                                                    (lv_btnmatrix_ctrl_t)10, (lv_btnmatrix_ctrl_t)(LV_BTNMATRIX_CTRL_HIDDEN | 2), (lv_btnmatrix_ctrl_t)(LV_BTNMATRIX_CTRL_HIDDEN | 6), (lv_btnmatrix_ctrl_t)(LV_BTNMATRIX_CTRL_HIDDEN | 2), (lv_btnmatrix_ctrl_t)10 };


  /*Create a keyboard and add the new map as USER_1 mode*/
  kb_alpha = lv_keyboard_create(screen1);

  // 4 keyboard modes are available, this uses mode 1, the numeric keyboard then must use mode 2
  lv_keyboard_set_map(kb_alpha, LV_KEYBOARD_MODE_USER_1, kb_map, kb_ctrl);
  lv_keyboard_set_mode(kb_alpha, LV_KEYBOARD_MODE_USER_1);
  lv_obj_set_height(kb_alpha, 150);

  lv_style_init(&style_kb);
  lv_style_set_bg_color(&style_kb, lv_palette_lighten(LV_PALETTE_BLUE, 1));
  lv_obj_add_style(kb_alpha, &style_kb, LV_PART_ITEMS);


  lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb_alpha);
  lv_keyboard_set_textarea(kb_alpha, ta);
}

void lv_battleship_keyboard_num() {

  static const char *kb_map[] = { "1", "2", "3", "4", "\n",
                                  "5", "6", "7", "8", "\n",
                                  "9", "0", "\n",
                                  LV_SYMBOL_BACKSPACE, " ", " ", " ", LV_SYMBOL_OK, NULL };


static const lv_btnmatrix_ctrl_t kb_ctrl[] = { (lv_btnmatrix_ctrl_t)8, (lv_btnmatrix_ctrl_t)8, (lv_btnmatrix_ctrl_t)8, (lv_btnmatrix_ctrl_t)8,
                                                    (lv_btnmatrix_ctrl_t)8, (lv_btnmatrix_ctrl_t)8, (lv_btnmatrix_ctrl_t)8, (lv_btnmatrix_ctrl_t)8,
                                                    (lv_btnmatrix_ctrl_t)8, (lv_btnmatrix_ctrl_t)8,
                                                    (lv_btnmatrix_ctrl_t)10, (lv_btnmatrix_ctrl_t)(LV_BTNMATRIX_CTRL_HIDDEN | 2), (lv_btnmatrix_ctrl_t)(LV_BTNMATRIX_CTRL_HIDDEN | 6), (lv_btnmatrix_ctrl_t)(LV_BTNMATRIX_CTRL_HIDDEN | 2), (lv_btnmatrix_ctrl_t)10 };


  /*Create a keyboard and add the new map as USER_1 mode*/
  kb_num = lv_keyboard_create(screen1);

  // 4 keyboard modes are available, this uses mode 1, the numeric keyboard then must use mode 2
  lv_keyboard_set_map(kb_num, LV_KEYBOARD_MODE_USER_2, kb_map, kb_ctrl);
  lv_keyboard_set_mode(kb_num, LV_KEYBOARD_MODE_USER_2);
  lv_obj_set_height(kb_num, 150);

  lv_style_init(&style_kb);
  lv_style_set_bg_color(&style_kb, lv_palette_lighten(LV_PALETTE_BLUE, 1));
  lv_obj_add_style(kb_num, &style_kb, LV_PART_ITEMS);


  /*Create a text area. The keyboard will write here*/

  lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb_num);
  lv_keyboard_set_textarea(kb_num, ta);
}

static void ta_event_cb(lv_event_t *e) {

  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_FOCUSED) {
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);
  }

  if (code == LV_EVENT_DEFOCUSED) {
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  }

  if (code == LV_EVENT_VALUE_CHANGED) {
    // coordinates = String(lv_textarea_get_text(ta));
    //wenn alle zeichen gelöscht wurden, dann zurück zur Alpha Tastatur
    if (String(lv_textarea_get_text(ta)).length() == 0){ 
     lv_obj_add_flag(kb_num, LV_OBJ_FLAG_HIDDEN);
     lv_obj_remove_flag(kb_alpha, LV_OBJ_FLAG_HIDDEN);
     //lv_battleship_keyboard_alpha();
    }
  }
  
  // ok button is pressed  - read the text area
  if (code == LV_EVENT_READY) {
    coordinates = String(lv_textarea_get_text(ta));
    Serial.println("Koordinaten eingegeben: ");
    Serial.println(coordinates);

    // wenn erste Kooridnate eingegeben wurde dann das 2. Keyboard anzeigen
    if (coordinates.length() == 1){ 
      lv_obj_add_flag(kb_alpha, LV_OBJ_FLAG_HIDDEN);
      lv_obj_remove_flag(kb_num, LV_OBJ_FLAG_HIDDEN);
    //  lv_battleship_keyboard_num();
    }
  }
}

void lv_orientation_btn(void)
{
    lv_obj_t * label;

    lv_obj_t * btn_horizontal = lv_btn_create(screen3);
    lv_obj_add_event_cb(btn_horizontal, btn_event_horizontal, LV_EVENT_ALL, NULL);
    lv_obj_align(btn_horizontal, LV_ALIGN_CENTER, 0, -40);
    lv_obj_set_height(btn_horizontal, 80);
    lv_obj_set_width(btn_horizontal, 150);
    lv_obj_set_style_text_font(btn_horizontal, &lv_font_montserrat_20, 0);

    label = lv_label_create(btn_horizontal);
    lv_label_set_text(label, btn_label_horizontal);
    lv_obj_center(label);

    lv_obj_t * btn_vertical = lv_btn_create(screen3);
    lv_obj_add_event_cb(btn_vertical, btn_event_vertical, LV_EVENT_ALL, NULL);
    lv_obj_align(btn_vertical, LV_ALIGN_CENTER, 0, 60);
    lv_obj_set_height(btn_vertical, 80);
    lv_obj_set_width(btn_vertical, 150);
    lv_obj_set_style_text_font(btn_vertical, &lv_font_montserrat_20, 0);

    label = lv_label_create(btn_vertical);
    lv_label_set_text(label, btn_label_vertical);
    lv_obj_center(label);
}

//neu
static void btn_event_rockets_h(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        Serial.println("Rockets  horizontal chosen");
       use_rockets = 1;
    }

}
//neu
static void btn_event_rockets_v(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        Serial.println("Rockets vertical chosen");
       use_rockets = 2;
    }

}
//neu
static void btn_event_cannon(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        Serial.println("Cannon chosen");
        use_rockets = 0;
    }

}

// //new
void lv_chooseweapon_btn(void)
{
    lv_obj_t * label;

    lv_obj_t * btn_rockets_h = lv_btn_create(screen4);
    lv_obj_add_event_cb(btn_rockets_h, btn_event_rockets_h, LV_EVENT_ALL, NULL);
    lv_obj_align(btn_rockets_h, LV_ALIGN_CENTER, 0, -60);
    lv_obj_set_height(btn_rockets_h, 60);
    lv_obj_set_width(btn_rockets_h, 250);
    lv_obj_set_style_text_font(btn_rockets_h, &lv_font_montserrat_20, 0);

    label = lv_label_create(btn_rockets_h);
    lv_label_set_text(label, "Raketen Horizontal");
    lv_obj_center(label);

    
    //
    //

    lv_obj_t * btn_rockets_v = lv_btn_create(screen4);
    lv_obj_add_event_cb(btn_rockets_v, btn_event_rockets_v, LV_EVENT_ALL, NULL);
    lv_obj_align(btn_rockets_v, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_height(btn_rockets_v, 60);
    lv_obj_set_width(btn_rockets_v, 250);
    lv_obj_set_style_text_font(btn_rockets_v, &lv_font_montserrat_20, 0);

    label = lv_label_create(btn_rockets_v);
    lv_label_set_text(label, "Raketen Vertikal");
    lv_obj_center(label);

    //
    //

    lv_obj_t * btn_cannon = lv_btn_create(screen4);
    lv_obj_add_event_cb(btn_cannon, btn_event_cannon, LV_EVENT_ALL, NULL);
    lv_obj_align(btn_cannon, LV_ALIGN_CENTER, 0, 80);
    lv_obj_set_height(btn_cannon, 60);
    lv_obj_set_width(btn_cannon, 250);
    lv_obj_set_style_text_font(btn_cannon, &lv_font_montserrat_20, 0);

    label = lv_label_create(btn_cannon);
    lv_label_set_text(label, "Kanone X");
    lv_obj_center(label);
}





void create_label_shoot_result(const char * shoot_result ="default"){
    label = lv_label_create( screen2 );
    lv_label_set_text( label, shoot_result );
    lv_obj_set_width(label, 250);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0); // use centered text alignment
    lv_obj_align( label, LV_ALIGN_CENTER, 0, -50 ); // shot centrally on the screen


}


void clearScreen() {

  lv_obj_clean(lv_scr_act());
}

// neu
void loadScreenChooseWeapon(int rockets_ammo) {
  String label_text = "Waffe waehlen - Raketen: ";  
  label_text +=  rockets_ammo;
  coordinate_label_text = label_text.c_str(); // convert String to Char*
  create_label(coordinate_label_text, screen4);
  lv_chooseweapon_btn();
  lv_screen_load(screen4);

}

//ships orientation
void loadScreen3(String shipName) {
   Serial.println("Loading Screen 3");
  String label_text = "Schiffsorientierung fuer "+  shipName; 
  coordinate_label_text = label_text.c_str(); // convert String to Char*
  create_label(coordinate_label_text, screen3);
  lv_orientation_btn();
  lv_screen_load(screen3);
  while (true) {
            if (orientation == 'H' || orientation == 'V') {
              //set ship orientation after input received
                 Serial.println("Orientation set");
              
              break;
            }

            lv_task_handler(); /* let the GUI do its work */
            lv_tick_inc(5);    /* tell LVGL how much time has passed */
            delay(5);          /* let this time pass */
             
   }
  //once orientation is set go to coordination screen
  clearScreen();
  loadScreen1(shipName);


}

//simple label
void loadScreen2(const char* result) {


  create_label_shoot_result(result);
  lv_screen_load(screen2);

      for (int i = 0; i < 500; i++) {
      lv_task_handler(); 
      lv_tick_inc(5); 
      delay(5); 
     }
  

}

//give coordinates
void loadScreen4() {

      //neu
    if (rockets_ammo > 0) {
         // choose weapon
      loadScreenChooseWeapon(rockets_ammo); // neu

    // neu
      // wait for user input for weapon choice
     while (true)
    {
      if ( use_rockets == 1 || use_rockets == 2) {
        Serial.println("Player has chosen weapon");
        // reduce ammmo by 1
        rockets_ammo--;
        break;
      } else if (use_rockets == 0) {
        Serial.println("Player has chosen normal shot");
        break;
      }
      
      lv_task_handler();
      lv_tick_inc(5);
      delay(5);
    }
    clearScreen();

    }



    // show the keyboards to enter coordinates for shots or ship positions

  String label_text = "Koordinaten eingeben "; 
  coordinate_label_text = label_text.c_str(); // convert String to Char*
  create_label(coordinate_label_text, screen1);
  create_textarea();
  lv_battleship_keyboard_alpha();
  lv_battleship_keyboard_num();
  lv_obj_add_flag(kb_num, LV_OBJ_FLAG_HIDDEN);
  lv_screen_load(screen1);
  while (true) {

      
            Ship ship;

            lv_task_handler(); /* let the GUI do its work */
            lv_tick_inc(5);    /* tell LVGL how much time has passed */
            delay(5);          /* let this time pass */

           

            // Length of coordinates not longer then 2; first character must be alpha; second character must be numeric; (if third character is 0 then 2nd char must be 1 (because only 0 is allowed e.g. A10)) or length =2, so e.g. B7
            if (coordinates.length() >= 2 && isalpha(coordinates[0]) && isdigit(coordinates[1]) && ((coordinates[2] == '0' && coordinates[1] =='1') || coordinates.length() == 2) ) {

              Serial.println("Coordinates:");
              Serial.println(orientation);
              Serial.println(coordinates);

                ship.startX = coordinates[0] - 'A';
                ship.startY = 0;
                for (size_t i = 1; i < coordinates.length(); ++i)
                {
                  if (isdigit(coordinates[i]))
                  {

                    ship.startY = ship.startY * 10 + (coordinates[i] - '0');
                  }
                  else
                  {

                    // Handle non-digit characters in the numeric part
                    Serial.println("Invalid input. Please enter in the format 'A1' to 'J10'.");
                    return;
                  }
                }
                ship.startY -= 1; // Adjust for 0-based indexing

                  Serial.println(ship.startX);
                  Serial.println(ship.startY);

                  
                  ship_index++; //next ship in the array for the next placement
                  
                  // sendMessage("no_screen", "no_text", "X" ,ship.startX, ship.startY );

                  // neu
                  // convert the use_rockets int to char* for sending the message
                  char use_rockets_converted[2];
                  itoa(use_rockets, use_rockets_converted, 10);
                  

                  sendMessage(no_screen, no_text,  use_rockets_converted  ,ship.startX, ship.startY ); // neu
                  
                  use_rockets = 3; // reset weapon choice for next round 

                  break;
          
            }

          }
          // empty all coordinates
          orientation='X';
            coordinates ="";
           clearScreen();
}

//ship setup coordinates
void loadScreen1(String shipName) {
    // show the keyboards to enter coordinates for shots or ship positions

  String label_text = "Koordinaten eingeben "+  shipName; 
  coordinate_label_text = label_text.c_str(); // convert String to Char*
  create_label(coordinate_label_text, screen1);
  create_textarea();
  lv_battleship_keyboard_alpha();
  lv_battleship_keyboard_num();
  lv_obj_add_flag(kb_num, LV_OBJ_FLAG_HIDDEN);
  lv_screen_load(screen1);

  while (true) {

      
            Ship ship;

            lv_task_handler(); /* let the GUI do its work */
            lv_tick_inc(5);    /* tell LVGL how much time has passed */
            delay(5);          /* let this time pass */

           

            // Length of coordinates not longer then 2; first character must be alpha; second character must be numeric; (if third character is 0 then 2nd char must be 1 (because only 0 is allowed e.g. A10)) or length =2, so e.g. B7
            if (coordinates.length() >= 2 && isalpha(coordinates[0]) && isdigit(coordinates[1]) && ((coordinates[2] == '0' && coordinates[1] =='1') || coordinates.length() == 2) ) {

              Serial.println("Coordinates:");
              Serial.println(orientation);
              Serial.println(coordinates);


              // Validate that the ship does not exceed the board depending on orientation
              bool validPlacement = false;
              bool overlap = false;
              // help variables
              int startX = coordinates[0] - 'A';
              int ship_startY = 0;
              ship.size = SHIP_SIZES[ship_index];
              // calculate ship_startY for several validations
              for (size_t i = 1; i < coordinates.length(); ++i)
              // this handles in case of Y coordinate is 10
              {
                if (isdigit(coordinates[i]))
                {
                  ship_startY = ship_startY * 10 + (coordinates[i] - '0');
                }
              }
              ship_startY -= 1; // Adjust for 0-based indexing

              // space in the code for visual separation
              // space
              // space
              // space
              // space
              // space

              

              // new logic
              // char orientation_converted[2];
              // orientation.toCharArray(orientation_converted, sizeof(orientation_converted));
              // Serial.println("Orientation:");
              // Serial.print(orientation_converted);

              ship.orientation = orientation; //sicher, dass hier index 0 nur gebraucht wird?


                    if (ship.orientation == 'H' ||orientation == 'X')
                {
                  Serial.println("If: ship.orientation == 'H'");
                  // check ship out of bounds for horizontally for the borad
                  if ((coordinates[0] - 'A' + ship.size) <= BOARD_SIZE)
                  {
                    Serial.println("Horizontal within bounds");
                    // check if ships are placed on top of each other
                    for (int k = 0; k < ship.size; ++k)
                    {
                      if (battlefield[startX + k][ship_startY] == 'S')
                      {
                        overlap = true;
                        Serial.println("Overlap detected (H)");
                        break;
                      }
                    }
                    if (overlap == false)
                    {
                      Serial.println("No overlap (H) -> validPlacement=true");
                      validPlacement = true;
                    }
                  }
                  else
                  {
                    Serial.println("Horizontal out of bounds");
                  }
                }
                else if (ship.orientation == 'V')
                {
                  Serial.println("If: ship.orientation == 'V'");
                  // check ship out of bounds for horizontally for the borad
                  if ((ship_startY + ship.size) <= BOARD_SIZE)
                  {
                    Serial.println("Vertical within bounds");
                    // check if ships are placed on top of each other
                    for (int k = 0; k < ship.size; ++k)
                    {
                      if (battlefield[startX][ship_startY + k] == 'S')
                      {
                        overlap = true;
                        Serial.println("Overlap detected (V)");
                        break;
                      }
                    }
                    if (overlap == false)
                    {
                      Serial.println("No overlap (V) -> validPlacement=true");
                      validPlacement = true;
                    }


                  }
                  else
                  {
                    Serial.println("Vertical out of bounds");
                  }
                }


                
              if (validPlacement)
              {
                Serial.println("validPlacement branch entered");

                ship.startX = coordinates[0] - 'A';
                ship.startY = 0;
                for (size_t i = 1; i < coordinates.length(); ++i)
                {
                  if (isdigit(coordinates[i]))
                  {

                    ship.startY = ship.startY * 10 + (coordinates[i] - '0');
                  }
                  else
                  {

                    // Handle non-digit characters in the numeric part
                    Serial.println("Invalid input. Please enter in the format 'A1' to 'J10'.");
                    return;
                  }
                }
                ship.startY -= 1; // Adjust for 0-based indexing

                char* orientation_converted = &orientation;

             
              

                    // Place ship on the player's battlefield
                for (int i = 0; i < SHIP_SIZES[ship_index]; i++)
                {
                  if (ship.orientation == 'H')
                  {
                    battlefield[ship.startX + i][ship.startY] = 'S';
                    Serial.println("Ship placed Horizontal");
                   
                  }
                  else
                  {
                    battlefield[ship.startX][ship.startY + i] = 'S';
                    Serial.println("Ship placed Vertical");
                    
                  }
                  
                }
                  Serial.println(orientation_converted);
                  Serial.println(ship.startX);
                  Serial.println(ship.startY);

                  
                  ship_index++; //next ship in the array for the next placement
                  
                  sendMessage(no_screen, no_text, orientation_converted ,ship.startX, ship.startY );
                  


                  break;

              }


              // int coordX = coordinates[0]-'A';
              // int coordY;
              // if (coordinates.length() == 3){ // so Y coorinate is bigger then 9
              //       coordY = 10 + coordinates[2] -'0'; // can also handle grids/battlefields bigger then 10
              // }else {
              //   coordY = coordinates[1] -'0';
              // }
              
              // coordY = coordY -1;  // Adjust for 0-based indexing

            
            
            }

             
          }
          // empty all coordinates
          orientation='X';
            coordinates ="";
           clearScreen();
}

void shipStatus(const char* shipStatus, int abstandX, int abstandY){
  create_ship_label(shipStatus, abstandX, abstandY);



}

//build temporary board to check if ships are not overlapping or out of bounds
void emptyTemporaryBoard(){

      // Clear the player's battlefield
    for (int row = 0; row < BOARD_SIZE; ++row)
    {
      for (int col = 0; col < BOARD_SIZE; ++col)
      {
       battlefield[row][col] = ' ';
      }
    }

}




