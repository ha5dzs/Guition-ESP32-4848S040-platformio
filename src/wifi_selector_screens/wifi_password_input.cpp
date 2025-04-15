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

lv_obj_t *password_input_field; // We will nudge this using a callback function

/*
 * Callback functions
 */

static void wifi_password_input_screen_connect_button_callback_function(lv_event_t *e)
{
 lv_event_code_t event_code = lv_event_get_code(e);
}

static void wifi_password_input_screen_skip_button_callback_function(lv_event_t *e)
{

}

static void wifi_password_input_screen_change_network_button_callback_function(lv_event_t *e)
{
  // We don't even need the event code really, as the button has only one function
  wifi_ap_list_screen(); // Go back to the main screen
}

static void wifi_password_input_field_callback_function(lv_event_t *e)
{
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *the_input_field = lv_event_get_target(e);
  // Show/hide password when tapping on the input field
  if(event_code == LV_EVENT_CLICKED)
  {
    // Toggle password mask
    if(lv_textarea_get_password_mode(the_input_field))
    {
      // if it's on, then turn it off.
      lv_textarea_set_password_mode(the_input_field, false);
    }
    else
    {
      // if it's off, then turn it on.
      lv_textarea_set_password_mode(the_input_field, true);
    }
  }
  // Update the password string
  if(event_code == LV_EVENT_VALUE_CHANGED)
  {
    String the_password = lv_textarea_get_text(the_input_field);
    strcpy(wifi_password_to_connect, the_password.c_str());
  }

}

static void wifi_password_input_screen_keyboard_callback_function(lv_event_t *e)
{
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *keyboard_object = lv_event_get_target(e);
  uint16_t currently_pressed_key = lv_btnmatrix_get_selected_btn(keyboard_object);
  // Start the connection if the tick button, or the enter key was pressed.
  if(event_code == LV_EVENT_READY)
  {
    Serial.println("OK was pressed.");
    Serial.println(wifi_password_to_connect);
  } /*else if( (event_code == LV_EVENT_VALUE_CHANGED) && (currently_pressed_key == 22) ) // 22 is the enter key in the button matrix
  {

    Serial.println("Enter key was pressed.");
    Serial.println(wifi_ssid_to_connect);
  }*/
  else if (event_code == LV_EVENT_CANCEL)
  {
    Serial.println("The 'keyboard close' button was pressed.");
  }
}






/*
 * The main course
 */

 void wifi_password_input_screen(void)
 {

  // Clear screen.
  lv_obj_t *background = lv_obj_create(lv_scr_act());
  lv_obj_set_size(background, TFT_WIDTH, TFT_HEIGHT);
  lv_obj_align(background, LV_ALIGN_CENTER, 0, 0);

  // These are here because it is local to this function.
  lv_obj_t *screen_text_line_first;
  lv_obj_t *screen_text_line_second;

  // First line
  screen_text_line_first = lv_label_create(lv_scr_act());
  lv_label_set_text(screen_text_line_first, "wifi_manual_ssid_input_screen()");
  lv_obj_align(screen_text_line_first, LV_ALIGN_TOP_MID, 0, TFT_HEIGHT/100);

  // Second line
  screen_text_line_second = lv_label_create(lv_scr_act());
  lv_label_set_text(screen_text_line_second, "Enter password. Tap on text area to reveal.");
  lv_obj_align(screen_text_line_second, LV_ALIGN_TOP_MID, 0, TFT_HEIGHT/100+16);

  // Connect button
  lv_obj_t *connect_button = lv_btn_create(lv_scr_act());
  lv_obj_set_size(connect_button, TFT_WIDTH/3, TFT_HEIGHT/8);
  lv_obj_align(connect_button, LV_ALIGN_TOP_LEFT, TFT_WIDTH/100, TFT_HEIGHT/10);
  // Scan button: Add icon
  lv_obj_t *connect_button_icon = lv_label_create(connect_button);
  lv_label_set_text(connect_button_icon, LV_SYMBOL_WIFI);
  lv_obj_align(connect_button_icon, LV_ALIGN_LEFT_MID, 0, 0);
  // Scan button: Add label
  lv_obj_t *connect_button_label = lv_label_create(connect_button); // This also assigns the label as the button's child
  lv_label_set_text(connect_button_label, "Connect");
  lv_obj_center(connect_button_label);
  // Scan button: Callback function
  lv_obj_add_event_cb(connect_button, wifi_password_input_screen_connect_button_callback_function, LV_EVENT_CLICKED, NULL);

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
  lv_obj_add_event_cb(skip_button, wifi_password_input_screen_skip_button_callback_function, LV_EVENT_CLICKED, NULL);

  // Change network button.
  lv_obj_t *change_network_button = lv_btn_create(lv_scr_act());
  lv_obj_set_size(change_network_button, TFT_WIDTH/3-TFT_WIDTH/25, TFT_HEIGHT/8);
  lv_obj_align(change_network_button, LV_ALIGN_TOP_MID, 0, TFT_HEIGHT/10);
  lv_obj_set_style_bg_color(change_network_button, (lv_color_t)lv_color_make(128, 64, 64), 0);
  // Manual entry button: Add label
  lv_obj_t *change_network_button_label = lv_label_create(change_network_button);
  lv_label_set_text(change_network_button_label, "Change network");
  lv_obj_align(change_network_button_label, LV_ALIGN_CENTER, 0, 0);
  // Manual entry button: callback function
  lv_obj_add_event_cb(change_network_button, wifi_password_input_screen_change_network_button_callback_function, LV_EVENT_CLICKED, NULL);


  // The password input field
  password_input_field = lv_textarea_create(lv_scr_act());
  lv_textarea_set_one_line(password_input_field, true);
  lv_obj_align(password_input_field, LV_ALIGN_CENTER, 0, -TFT_HEIGHT/20);
  lv_obj_set_size(password_input_field, TFT_WIDTH/2+TFT_WIDTH/4, TFT_HEIGHT/10);
  lv_textarea_set_password_mode(password_input_field, true); // This prints password things.
  lv_obj_set_style_text_align(password_input_field, LV_TEXT_ALIGN_CENTER, 0);
  // Callback function
  lv_obj_add_event_cb(password_input_field, wifi_password_input_field_callback_function, LV_EVENT_ALL, NULL);


  // Keyboard
  lv_obj_t *keyboard = lv_keyboard_create(lv_scr_act());
  lv_keyboard_set_popovers(keyboard, true);
  lv_keyboard_set_textarea(keyboard, password_input_field);
  // Keyboard: Callback function
  lv_obj_add_event_cb(keyboard, wifi_password_input_screen_keyboard_callback_function, LV_EVENT_ALL, NULL);

 }