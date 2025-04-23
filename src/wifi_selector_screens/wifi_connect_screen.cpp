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



/*
 * The main course
*/

 // This only gets called when the SSID is set and optionally when the password is selected.

 void wifi_connect_screen(void)
 {
    uint32_t then = millis();
    uint32_t now = millis();
    uint32_t max_retries = 10;
    uint32_t retry_counter = 0;

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

    // Since this is essentially a render loop, we can measure time.
    if( (now - then) < 2000 && (WiFi.status() != WL_CONNECTED))
    {
        now = millis();

        if(now-then > 1000)
        {
            lv_label_set_text(status_label, "Connecting to:");
        }
        else
        {
            lv_label_set_text(status_label, LV_SYMBOL_WIFI "Connecting to:");
        }
        lv_obj_invalidate(status_label); // Force a redraw

        lv_label_set_text(info_label, wifi_ssid_to_connect);

    }
    else
    {
        if(retry_counter <= max_retries)
        {
            then = now;
            retry_counter++;
        }
        else
        {
            WiFi.disconnect();
            Serial.println("Could not connect to wifi.");
            lv_label_set_text(status_label, "Failed to connect to:");
            lv_obj_invalidate(status_label); // Force a redraw
        }

    }

    if(WiFi.status() == WL_CONNECTED)
    {
        // If we got here, we have a connection.
        lv_label_set_text(status_label, "Connected, IP address is:");
        lv_obj_invalidate(status_label);
        lv_label_set_text(info_label, WiFi.localIP().toString().c_str()); // Wow this is complicated.
        lv_obj_invalidate(info_label);

    }


 }