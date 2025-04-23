#include "wifi_selector_screens.h" // This has all the screens, which load all the functions.
#include <Arduino.h>
#include <lvgl.h>
#include <WiFi.h>

// This is defined in Guition_ESP32_4848S040.h, but this makes it more portable for future applications.
#define TFT_WIDTH 480
#define TFT_HEIGHT 480

/*
 * Global variables
*/

extern char wifi_password_to_connect[32];
extern char wifi_ssid_to_connect[32];


/*
 * Callback functions
 */


 // This only gets called when the SSID is set and optionally when the password is selected.

 void wifi_connect_screen(void)
 {

    // Clear screen.
    lv_obj_t *background = lv_obj_create(lv_scr_act());
    lv_obj_set_size(background, TFT_WIDTH, TFT_HEIGHT);
    lv_obj_align(background, LV_ALIGN_CENTER, 0, 0);

 }