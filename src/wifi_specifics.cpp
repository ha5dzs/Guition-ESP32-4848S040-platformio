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
lv_theme_t *display_theme;
lv_obj_t *label;
lv_obj_t *keyboard;;
lv_obj_t *input_field;
lv_obj_t *button;




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
    lv_label_set_text(label, "Connecting...");
    timeout--;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.printf("\nWiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());
    lv_label_set_text(label, "Connected!");

    // Set the internal RTC to UTC time. Three common servers, whatever.
    configTime(0, 0, "pool.ntp.org", "time.windows.com", "time.google.com");
    time_sync_status = sntp_get_sync_status();

    // See esp_sntp.h
    while (time_sync_status != SNTP_SYNC_STATUS_COMPLETED)
    {
      time_sync_status = sntp_get_sync_status();
      vTaskDelay(100); // Wait here

      // TODO: Check if ween to include a break statement and checking SNTP_SYNC_STATUS_RESET
    }
    lv_label_set_text(label, "Time synchronised.");
    we_have_accurate_time = 1;

    // Adjust the local RTC time to what was obtained with NTP
    localtime_r(&now, &posixtime);

    // Get rid of this, if you want to use your wifi as WIFI_MODE_APSTA
    WiFi.disconnect();
  }
  else
  {
    lv_label_set_text(label, "Could not connect.");
    WiFi.disconnect();
  }

  wifi_task_handle = NULL;
  vTaskDelete(NULL);

}

void wifi_ap_set(void)
{
  // Set up the wifi access point here.
  WiFi.softAP(&wifi_ap_ssid, &wifi_ap_password, 7); // Third argument is channel.
  //...and leave pretty much everything else as default

}

static void input_field_event_callback_function(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}

void ssid_input_screen(void)
{
  // Format the label
  label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, LV_SYMBOL_WIFI "Enter SSID. Press 'Skip' to stay offline.");
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

  keyboard = lv_keyboard_create(lv_scr_act());

  input_field = lv_textarea_create(lv_scr_act());
  lv_obj_align(input_field, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_size(input_field, TFT_WIDTH, TFT_HEIGHT/10);
  lv_obj_add_event_cb(input_field, input_field_event_callback_function, LV_EVENT_ALL, keyboard);
  lv_textarea_set_placeholder_text(input_field, "SSID goes here");




}

