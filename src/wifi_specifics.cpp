#include "wifi_specifics.h"
#include <lvgl.h>
#include <font/lv_font.h>
#include <Wifi.h>
#include <WifiAP.h>
#include <esp_sntp.h>
#include "Guition_ESP32_4848S040.h"

/*
 * Global variables
*/

extern char wifi_ssid_to_connect;
extern char wifi_password_to_connect;
extern uint8_t wifi_need_to_connect;
extern char wifi_ap_ssid;
extern char wifi_ap_password;
extern TaskHandle_t wifi_task_handle;
extern uint8_t we_have_accurate_time;
extern time_t now;
extern tm posixtime;

/*
 * Local variables
*/

int no_of_wifi_networks = 0;
sntp_sync_status_t time_sync_status; // This is for checking the ntp status.
// GUI elements. As simple as it can be.
lv_obj_t *screen_title; // This is accessible from other functions





void wifi_scan_for_aps(void)
{
  // WiFi.scanNetworks will return the number of networks found.
  no_of_wifi_networks = WiFi.scanNetworks();
}

/*
 * This is heavily inspired by: https://github.com/pangcrd/LVGL_Bassic-tutorial/tree/main/CYD_WiFiLoginForm/src
 * We get the info here from the GUI
*/

void wifi_client_task(void)
{
  // If we were connected, disconnect
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(&wifi_ssid_to_connect, &wifi_password_to_connect);

  // Give it a timeout
  uint8_t timeout = 10; // Seconds.
  while ( WiFi.status() != WL_CONNECTED && timeout > 0 )
  {
    vTaskDelay(1000); // NOT Arduino delay.
    lv_label_set_text(screen_title, "Connecting...");
    timeout--;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.printf("\nWiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());
    lv_label_set_text(screen_title, "Connected!");

    wifi_get_time_ntp(); // Set the time to accurate UTC.

    // Get rid of this, if you want to use your wifi as WIFI_MODE_APSTA
    WiFi.disconnect();
  }
  else
  {
    lv_label_set_text(screen_title, "Could not connect.");
    WiFi.disconnect();
  }

  wifi_task_handle = NULL;
  vTaskDelete(NULL);

}

void wifi_get_time_ntp(void)
{
  // Set the internal RTC to UTC time. Three common servers, whatever.
  configTime(0, 0, "pool.ntp.org", "time.windows.com", "time.google.com");
  time_sync_status = sntp_get_sync_status();

  // See esp_sntp.h
  while (time_sync_status != SNTP_SYNC_STATUS_COMPLETED)
  {
    time_sync_status = sntp_get_sync_status();
    vTaskDelay(100); // Wait here

    // TODO: Check if we need to include a break statement and checking SNTP_SYNC_STATUS_RESET
  }
  lv_label_set_text(screen_title, "Time synchronised.");
  we_have_accurate_time = 1;

  // Adjust the local RTC time to what was obtained with NTP
  localtime_r(&now, &posixtime);

}

void wifi_ap_set(void)
{
  // Set up the wifi access point here.
  WiFi.softAP(&wifi_ap_ssid, &wifi_ap_password, 7); // Third argument is channel.
  //...and leave pretty much everything else as default

}

static void wifi_skip_button_callback_function(lv_event_t *e)
{
  lv_event_code_t event_code = lv_event_get_code(e);
  if(event_code == LV_EVENT_CLICKED)
  {
    // If we got here, we clicked the button.
    Serial.println("Skip button was pressed.");
  }
}

static void wifi_scan_button_callback_function(lv_event_t *e)
{
  lv_event_code_t event_code = lv_event_get_code(e);
  if(event_code == LV_EVENT_CLICKED)
  {
    // If we got here, we clicked the button.
    Serial.println("Scan button was pressed.");
  }
}

static void wifi_ssid_input_field_callback_function(lv_event_t *e)
{
  lv_event_code_t event_code = lv_event_get_code(e);
  if(event_code == LV_EVENT_VALUE_CHANGED)
  {
    // If we got here, we clicked the button.
    Serial.println("Scan button was pressed.");
  }
}

void wifi_start_screen(void)
{

  // We 'clear' the screen, by adding a large object.
  lv_obj_t *background = lv_obj_create(lv_scr_act());
  lv_obj_set_size(background, TFT_WIDTH, TFT_HEIGHT);
  lv_obj_align(background, LV_ALIGN_CENTER, 0, 0);

  // Label, on the top
  screen_title = lv_label_create(lv_scr_act());
  lv_label_set_text(screen_title, "wifi_start_screen()");
  lv_obj_align(screen_title, LV_ALIGN_TOP_MID, 0, TFT_HEIGHT/100);

  // Scan button
  lv_obj_t *scan_button = lv_btn_create(lv_scr_act());
  lv_obj_set_size(scan_button, TFT_WIDTH/3, TFT_HEIGHT/8);
  lv_obj_align(scan_button, LV_ALIGN_TOP_LEFT, TFT_WIDTH/100, TFT_HEIGHT/10);
  // Scan button: Add icon
  lv_obj_t *scan_button_icon = lv_label_create(scan_button);
  lv_label_set_text(scan_button_icon, LV_SYMBOL_WIFI);
  lv_obj_align(scan_button_icon, LV_ALIGN_LEFT_MID, 0, 0);
  // Scan button: Add label
  lv_obj_t *scan_button_label = lv_label_create(scan_button); // This also assigns the label as the button's child
  lv_label_set_text(scan_button_label, "Scan for\nnetworks");
  lv_obj_center(scan_button_label);
  // Scan button: Callback function
  lv_obj_add_event_cb(scan_button, wifi_scan_button_callback_function, LV_EVENT_CLICKED, NULL);

  // Skip button
  lv_obj_t *skip_button = lv_btn_create(lv_scr_act());
  lv_obj_set_size(skip_button, TFT_WIDTH/3, TFT_HEIGHT/8);
  lv_obj_align(skip_button, LV_ALIGN_TOP_RIGHT, -TFT_WIDTH/100, TFT_HEIGHT/10);
  // Skip button: Add icon
  lv_obj_t *skip_button_icon = lv_label_create(skip_button);
  lv_label_set_text(skip_button_icon, LV_SYMBOL_CLOSE);
  lv_obj_align(skip_button_icon, LV_ALIGN_LEFT_MID, 0, 0);
  // Skip button: Add label
  lv_obj_t *skip_button_label = lv_label_create(skip_button); // This also assigns the label as the button's child
  lv_label_set_text(skip_button_label, "Stay offline");
  lv_obj_center(skip_button_label);
  // Skip button: callback function
  lv_obj_add_event_cb(skip_button, wifi_skip_button_callback_function, LV_EVENT_CLICKED, NULL);

  // Some text in the middle.
  lv_obj_t *nice_guidance_label = lv_label_create(lv_scr_act());
  lv_obj_align(nice_guidance_label, LV_ALIGN_TOP_MID, 0, TFT_HEIGHT/3);
  lv_label_set_long_mode(nice_guidance_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_size(nice_guidance_label, TFT_WIDTH-TFT_WIDTH/50, TFT_HEIGHT/20);
  lv_label_set_text(nice_guidance_label, "         Scan for available 2.4 GHz networks. If the SSID you want is hidden, type it in below. Press " LV_SYMBOL_OK " when done.        ");


  lv_obj_t *input_field = lv_textarea_create(lv_scr_act());
  lv_textarea_set_one_line(input_field, true);
  lv_obj_align(input_field, LV_ALIGN_CENTER, 0, -TFT_HEIGHT/20);
  lv_obj_set_size(input_field, TFT_WIDTH/2+TFT_WIDTH/4, TFT_HEIGHT/10);
  lv_textarea_set_placeholder_text(input_field, "Type the SSID here if you dare...");
  lv_obj_set_style_text_align(input_field, LV_TEXT_ALIGN_CENTER, 0);
  // TODO: Save wifi_ssid_to_connect when value changed.



  lv_obj_t *keyboard = lv_keyboard_create(lv_scr_act());
  lv_keyboard_set_popovers(keyboard, true);
  lv_keyboard_set_textarea(keyboard, input_field); // This allows typing to the input field.
  // TODO: Add callback function when the OK button is pressed.





}

