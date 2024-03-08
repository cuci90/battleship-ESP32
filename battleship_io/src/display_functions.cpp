#include "battleship.h"

static lv_style_t style_kb;

const char *shoot_result_text;
const char *coordinate_label_text;
const char *btn_label_horizontal = "Horizontal ---";
const char *btn_label_vertical = "Vertikal |||";
const char *ship_status_text = "Schiff Status";
const char *ship_orientation_for = "Schiffsorientierung fuer ";
const char *enter_coordinates = "Koordinaten eingeben ";
 lv_obj_t *kb_alpha;
  lv_obj_t *kb_num;

lv_obj_t *ta;
lv_obj_t *screen1;
lv_obj_t *screen2;
lv_obj_t *screen3;
String coordinates = "";
char orientation='X';


// Screen variables
SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;

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
    touchX = map(p.x, 150, 4000, 1, screenWidth);  /* Touchscreen X calibration */
    touchY = map(p.y, 209, 3750, 1, screenHeight); /* Touchscreen X calibration */
    data->state = LV_INDEV_STATE_PR;

    /*Set the touchpoints*/
    data->point.x = touchX;
    data->point.y = touchY;

    // Serial.print("Data x ");
    // Serial.println(touchX);

    // Serial.print("Data y ");
    // Serial.println(touchY);
  }
}

void initializeDisplay() {

  lv_init();
  lv_display_t *disp;
  #if LV_USE_TFT_ESPI
  /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
  #else
  /*Else create a display yourself*/
  disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
  #endif
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS); /* Start second SPI bus for touchscreen */
  ts.begin(mySpi);                                                  /* Touchscreen init */
  ts.setRotation(3);                                                /* Landscape orientation */

  /*Initialize the (dummy) input device driver*/
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);

  //create 2 screens: 1 for the keyboards / 1 for shoot result display
  screen1 = lv_obj_create(NULL);
  screen2 = lv_obj_create(NULL);
  screen3 = lv_obj_create(NULL);


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

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {

  lv_disp_flush_ready(disp);
}

void create_label(const char *coordinate_label_text , lv_obj_t *screen ) {
  //Text label
  lv_obj_t *label = lv_label_create(screen);
  lv_label_set_text(label, coordinate_label_text);
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);
}
void create_ship_label(const char *ship_status, int abstandX, int abstandY ) {
  //Text label
  lv_obj_t *label_status = lv_label_create(screen2);
  lv_label_set_text(label_status, ship_status_text);
   lv_obj_set_style_text_font(label_status, &lv_font_montserrat_14, 0);
  lv_obj_align(label_status, LV_ALIGN_BOTTOM_MID, 0, -85);

  lv_obj_t *label = lv_label_create(screen2);
  lv_label_set_text(label, ship_status);
   lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
  lv_obj_align(label, LV_ALIGN_BOTTOM_MID, abstandX, abstandY);
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


  static const lv_buttonmatrix_ctrl_t kb_ctrl[] = { 8, 8, 8, 8,
                                                    8, 8, 8, 8,
                                                    8, 8,
                                                    10, LV_BUTTONMATRIX_CTRL_HIDDEN | 2, LV_BUTTONMATRIX_CTRL_HIDDEN | 6, LV_BUTTONMATRIX_CTRL_HIDDEN | 2, 10 };

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


  static const lv_buttonmatrix_ctrl_t kb_ctrl[] = { 8, 8, 8, 8,
                                                    8, 8, 8, 8,
                                                    8, 8,
                                                    10, LV_BUTTONMATRIX_CTRL_HIDDEN | 2, LV_BUTTONMATRIX_CTRL_HIDDEN | 6, LV_BUTTONMATRIX_CTRL_HIDDEN | 2, 10 };

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

void create_label_shoot_result(const char * shoot_result ="default"){
    lv_obj_t *label = lv_label_create( screen2 );
    lv_label_set_text( label, shoot_result );
    lv_obj_set_width(label, 250);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0); // use centered text alignment
    lv_obj_align( label, LV_ALIGN_CENTER, 0, -50 ); // shot centrally on the screen

    //lv_screen_load(screen2);
}




void clearScreen() {

  lv_obj_clean(lv_scr_act());

}

void loadScreen3(String shipName) {
  String label_text = ship_orientation_for +  shipName; 
  coordinate_label_text = label_text.c_str(); // convert String to Char*
  create_label(coordinate_label_text, screen3);
  lv_orientation_btn();
  lv_screen_load(screen3);

}

void loadScreen2(const char* result ) {
  create_label_shoot_result(result);
  lv_screen_load(screen2);
     for (int i = 0; i < 500; i++) {
     lv_task_handler(); 
    lv_tick_inc(5); 
    delay(5); 
  }
  
    

}


void loadScreen1(String shipName) {

    // show the keyboards to enter coordinates for shots or ship positions
    String label_text = enter_coordinates +  shipName; 
  coordinate_label_text = label_text.c_str(); // convert String to Char*
  create_label(coordinate_label_text, screen1);
  create_textarea();
  lv_battleship_keyboard_alpha();
  lv_battleship_keyboard_num();
  lv_obj_add_flag(kb_num, LV_OBJ_FLAG_HIDDEN);

  lv_screen_load(screen1);
}

void shipStatus(const char* shipStatus, int abstandX, int abstandY){
  create_ship_label(shipStatus, abstandX, abstandY);



}
