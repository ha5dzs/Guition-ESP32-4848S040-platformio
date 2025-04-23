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

extern char wifi_ssid_to_connect[32];


/*
 * Callback functions
*/

static void wifi_manual_ssid_input_scan_button_callback_function(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if(event_code == LV_EVENT_PRESSED)
    {
    // If we got here, we clicked the button.
    Serial.println("Scan button was pressed.");



    wl_status_t wifi_status = WiFi.status();

    if(wifi_status == WL_IDLE_STATUS)
    {
        // Force scanning, if the adapter is idle.
        WiFi.scanDelete();
        WiFi.scanNetworks(true);
    }

    wifi_ap_list_screen();
    }

}

static void wifi_manual_ssid_input_skip_button_callback_function(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if(event_code == LV_EVENT_CLICKED)
    {
        // If we got here, we clicked the button.
        Serial.println("Skip button was pressed.");
        return;
    }
}


static void wifi_ssid_manual_ssid_input_input_field_callback_function(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *input_field_pointer = lv_event_get_target(e);
    if(event_code == LV_EVENT_VALUE_CHANGED)
    {
        // If we got here, the input field changed with the SSID.
        //Serial.println(lv_textarea_get_text(input_field_pointer));
        String input_text = lv_textarea_get_text(input_field_pointer);

        // Copy the string
        strcpy(wifi_ssid_to_connect, input_text.c_str());

        //Serial.println(input_text);

    }
}

static void wifi_ssid_manual_ssid_input_keyboard_callback_function(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *keyboard_object = lv_event_get_target(e);
    uint16_t currently_pressed_key = lv_btnmatrix_get_selected_btn(keyboard_object);
    // Start the connection if the tick button, or the enter key was pressed.
    if(event_code == LV_EVENT_READY)
    {
        Serial.println("OK was pressed.");
        Serial.println(wifi_ssid_to_connect);
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

void wifi_manual_ssid_input_screen(void)
{

    // We 'clear' the screen, by adding a large object.
    //lv_obj_clean(lv_scr_act());
    lv_obj_t *background = lv_obj_create(lv_scr_act());
    lv_obj_set_size(background, TFT_WIDTH, TFT_HEIGHT);
    lv_obj_align(background, LV_ALIGN_CENTER, 0, 0);

    // These are here because it is local to this function.
    lv_obj_t *screen_text_line_first;
    lv_obj_t *screen_text_line_second;

    // Label, on the top
    screen_text_line_first = lv_label_create(lv_scr_act());
    lv_label_set_text(screen_text_line_first, "wifi_manual_ssid_input_screen()");
    lv_obj_align(screen_text_line_first, LV_ALIGN_TOP_MID, 0, TFT_HEIGHT/100);


    // Second screen line
    screen_text_line_second = lv_label_create(lv_scr_act());
    lv_label_set_text(screen_text_line_second, "Second line text.");
    lv_obj_align(screen_text_line_second, LV_ALIGN_TOP_MID, 0, TFT_HEIGHT/100+16);

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
    lv_obj_add_event_cb(scan_button, wifi_manual_ssid_input_scan_button_callback_function, LV_EVENT_ALL, NULL);

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
    lv_obj_add_event_cb(skip_button, wifi_manual_ssid_input_skip_button_callback_function, LV_EVENT_CLICKED, NULL);

    // Some text in the middle.
    lv_obj_t *nice_guidance_label = lv_label_create(lv_scr_act());
    lv_obj_align(nice_guidance_label, LV_ALIGN_TOP_MID, 0, TFT_HEIGHT/3);
    //lv_label_set_long_mode(nice_guidance_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_size(nice_guidance_label, TFT_WIDTH-TFT_WIDTH/50, TFT_HEIGHT/20);
    lv_label_set_text(nice_guidance_label, "Enter SSID. Press " LV_SYMBOL_OK " when done.");

    // SSID field
    lv_obj_t *ssid_input_field = lv_textarea_create(lv_scr_act());
    lv_textarea_set_one_line(ssid_input_field, true);
    lv_obj_align(ssid_input_field, LV_ALIGN_CENTER, 0, -TFT_HEIGHT/20);
    lv_obj_set_size(ssid_input_field, TFT_WIDTH/2+TFT_WIDTH/4, TFT_HEIGHT/10);
    lv_textarea_set_placeholder_text(ssid_input_field, "Type the SSID here if you dare...");
    lv_obj_set_style_text_align(ssid_input_field, LV_TEXT_ALIGN_CENTER, 0);
    // Callback function
    lv_obj_add_event_cb(ssid_input_field, wifi_ssid_manual_ssid_input_input_field_callback_function, LV_EVENT_ALL, NULL);

    // Keyboard
    lv_obj_t *keyboard = lv_keyboard_create(lv_scr_act());
    lv_keyboard_set_popovers(keyboard, true);
    lv_keyboard_set_textarea(keyboard, ssid_input_field); // This allows typing to the input field.
    // Callback function: By default, this is lv_keyboard_def_event_cb(), but we override that here.
    lv_obj_add_event_cb(keyboard, wifi_ssid_manual_ssid_input_keyboard_callback_function, LV_EVENT_ALL, NULL);

}