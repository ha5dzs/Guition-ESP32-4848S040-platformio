#include "wifi_selector_screens.h" // This has all the screens, which load all the functions.
#include <Arduino.h>
#include <lvgl.h>
#include <WiFi.h>
#include <esp_task_wdt.h> // ESP task watchdog.


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



/*
 * The main course
*/

 // This only gets called when the SSID is set and optionally when the password is selected.

 void wifi_connect_screen(void)
 {

    esp_task_wdt_init(30, true); // Reset if hangs for more than 30 seconds

    // Clear screen.
    lv_obj_clean(lv_scr_act());

    lv_obj_t *background = lv_obj_create(lv_scr_act());
    lv_obj_set_size(background, TFT_WIDTH, TFT_HEIGHT);
    lv_obj_align(background, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *status_label = lv_label_create(lv_scr_act());
    lv_obj_align(status_label, LV_ALIGN_CENTER, 0, -TFT_WIDTH/10);


    lv_obj_t *info_label = lv_label_create(lv_scr_act());
    lv_obj_align(info_label, LV_ALIGN_CENTER, 0, TFT_WIDTH/10);



    // Connect to Wifi
    Serial.printf("Connecting to %s", wifi_ssid_to_connect);
    WiFi.begin(wifi_ssid_to_connect, wifi_password_to_connect);

    // Block execution?
    while( WiFi.status() != WL_CONNECTED )
    {
        esp_task_wdt_reset(); // Reset watchdog while we are waiting.
        lv_label_set_text(status_label, "Connecting to:");

        lv_obj_invalidate(status_label); // Force a redraw

        lv_label_set_text(info_label, wifi_ssid_to_connect);

    }


    Serial.println("Connected.");
    // If we got here, we have a connection.
    lv_label_set_text(status_label, "Connected, IP address is:");
    lv_obj_invalidate(status_label);
    lv_label_set_text(info_label, WiFi.localIP().toString().c_str()); // Wow this is complicated.
    lv_obj_invalidate(info_label);



 }