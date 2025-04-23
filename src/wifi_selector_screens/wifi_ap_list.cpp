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
extern int16_t no_of_wifi_networks;
extern uint8_t wifi_selected_network_index;

// These are here so I can update these from within the callback functions too.
lv_obj_t *screen_text_line_first;
lv_obj_t *screen_text_line_second;
lv_obj_t *connect_button;

/*
 * Callback functions
*/

static void wifi_ap_list_list_callback_function(lv_event_t *e)
{
  // Fetch the event that triggered it
  lv_event_code_t event_code = lv_event_get_code(e);
  // Get user data, the pointer's address IS the value.
  void* user_data = lv_event_get_user_data(e); // see explanation where the list is being populated with the for loop.

  // See: https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Pointer_002dInteger-Conversion.html
  wifi_selected_network_index = (uintptr_t)user_data;



  if(event_code == LV_EVENT_CLICKED)
  {
    Serial.printf( "Selected network's encryption type is: %d\n", WiFi.encryptionType(wifi_selected_network_index) );
    if(WiFi.encryptionType(wifi_selected_network_index) == WIFI_AUTH_WPA2_ENTERPRISE)
    {
        /*
         * For this authentication method, we may need additional stuff such as
         * - Username
         * - Anonymous username (talk about an oximoron, huh?)
         * - Some certificate file
         * ...so let's not faff on with this.
        */
       String new_second_line_text = "This network (" + WiFi.SSID(wifi_selected_network_index) + ") has unsupported encryption.";
        lv_label_set_text(screen_text_line_second, new_second_line_text.c_str());
        lv_obj_invalidate(screen_text_line_second); //Force a redraw

        // Gray out the button
        lv_obj_set_style_bg_color(connect_button, (lv_color_t)lv_color_make(64, 64, 64), 0);
        // Disable the button.
        lv_obj_clear_flag(connect_button, LV_OBJ_FLAG_CLICKABLE);
        // Force a redraw
        lv_obj_invalidate(connect_button);


    }
    else
    {
        // If we got here, update wifi_ssid_to_connect with lb_list_get_btn_text
        Serial.printf("User data value is: %d\n", user_data);
        Serial.printf("Converted pointer value is: %d\n", wifi_selected_network_index);
        Serial.printf("Selected network is: %s\n", WiFi.SSID(wifi_selected_network_index));

        // Update the second line
        String new_second_line_text = "Selected network is: " + WiFi.SSID(wifi_selected_network_index);
        lv_label_set_text(screen_text_line_second, new_second_line_text.c_str());
        lv_obj_invalidate(screen_text_line_second); //Force a redraw

        // Make the connect button go green.
        lv_obj_set_style_bg_color(connect_button, (lv_color_t)lv_color_make(0, 128, 0), 0);
        // Enable the button
        lv_obj_add_flag(connect_button, LV_OBJ_FLAG_CLICKABLE);
        // Force a redraw
        lv_obj_invalidate(connect_button);
    }


  }
  else
  {
    return;
  }

}

void wifi_ap_list_connect_button_callback_function(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if(event_code == LV_EVENT_CLICKED)
    {
        // This bit gets executed when the button is clicked
        Serial.println("Connect button pressed.");

        // Check if we have anything selected
        if(wifi_selected_network_index != 255)
        {
            // Put the SSID to a global variable
            strcpy(wifi_ssid_to_connect, WiFi.SSID(wifi_selected_network_index).c_str());

            // Check if it needs security
            if(WiFi.encryptionType(wifi_selected_network_index) != WIFI_AUTH_OPEN)
            {
                // We need to get the password from a different screen.
                Serial.println("The selected network needs a password.");
                wifi_password_input_screen(); // Request the password
            }
            else
            {
                // We can go ahead and connect, no encryption used.
                wifi_connect_screen();
            }
        }
    }
}

static void wifi_ap_list_manual_entry_button_callback_function(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if(event_code == LV_EVENT_CLICKED)
    {
        // This bit gets executed when the button is clicked
        Serial.println("Manual entry button pressed.");
        wifi_manual_ssid_input_screen();
    }
}

static void wifi_ap_list_stay_offline_button_callback_function(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if(event_code == LV_EVENT_CLICKED)
    {
        // This bit gets executed when the button is clicked
        Serial.println("Stay offline button pressed.");

        // Set up an open access point, this will be done in a different function.
        return;
    }
}

static void wifi_ap_list_refresh_button_callback_function(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if(event_code == LV_EVENT_CLICKED)
    {
        // This bit gets executed when the button is clicked
        Serial.println("Refresh button pressed.");


        wl_status_t wifi_status = WiFi.status();

        if(wifi_status == WL_IDLE_STATUS)
        {
            // Force scanning, if the adapter is idle.
            WiFi.scanDelete();
            WiFi.scanNetworks(true);
        }

        int16_t wifi_scan_return_code = WiFi.scanComplete();
        Serial.printf("WiFi.scanComplete()'s return value is: %d\n", wifi_scan_return_code);


        if (wifi_scan_return_code >= 0)
        {
            // If we got here, we are either scanning, or something failed.
            if(wifi_status == WL_IDLE_STATUS)
            {
                // Force scanning, if the adapter is idle.
                WiFi.scanDelete();
                WiFi.scanNetworks(true);
            }

        }
        else
        {
            // If we got here, we can restart the scan, and with it, the entire screen.

            WiFi.scanNetworks(true);
            no_of_wifi_networks = -1; // Scan in progress, will be updated from the GUI function.


            //lv_obj_clean(lv_scr_act());
            //wifi_ap_list_screen(); // Call the entire GUI again
        }

        // Redraw the entire screen.
        lv_obj_invalidate(lv_scr_act());

    }
}


/*
 * The main course
*/

void wifi_ap_list_screen(void)
{

  // We 'clear' the screen, by adding a large object.
  //lv_obj_clean(lv_scr_act());
  lv_obj_t *background = lv_obj_create(lv_scr_act());
  lv_obj_set_size(background, TFT_WIDTH, TFT_HEIGHT);
  lv_obj_align(background, LV_ALIGN_CENTER, 0, 0);

  // First screen line
  screen_text_line_first = lv_label_create(lv_scr_act());
  lv_label_set_text(screen_text_line_first, "wifi_ap_list_screen()");
  //lv_obj_set_size(screen_text_line_first, TFT_WIDTH, 16);
  lv_obj_align(screen_text_line_first, LV_ALIGN_TOP_MID, 0, TFT_HEIGHT/100);

  no_of_wifi_networks = WiFi.scanComplete();
  Serial.printf("%d wifi networks found.\n");

  // Second screen line
  screen_text_line_second = lv_label_create(lv_scr_act());
  lv_label_set_text(screen_text_line_second, "Looking for access points...");
  //lv_obj_set_size(screen_text_line_second, TFT_WIDTH, 16);
  lv_obj_align(screen_text_line_second, LV_ALIGN_TOP_MID, 0, TFT_HEIGHT/100+16);


  if(no_of_wifi_networks < 0)
  {
    // Wait for the scan to finish
    while(no_of_wifi_networks == -1)
    {
        no_of_wifi_networks = WiFi.scanComplete();
        vTaskDelay(100);
    }
    lv_obj_invalidate(lv_scr_act()); // This forces the screen to be redrawn
  }

  lv_obj_clean(screen_text_line_second);
  String result_summary_text = "Found " + (String)no_of_wifi_networks + " access points.";
  lv_label_set_text(screen_text_line_second, result_summary_text.c_str()); // Make it pretty.
  lv_obj_invalidate(screen_text_line_second);

  // List of wifi APs.
  lv_obj_t *wifi_network_list = lv_list_create(lv_scr_act());
  lv_obj_set_size(wifi_network_list, TFT_WIDTH, TFT_HEIGHT-(TFT_HEIGHT/5)-(TFT_HEIGHT/10));
  lv_obj_align(wifi_network_list, LV_ALIGN_CENTER, 0, 0);
  lv_list_add_text(wifi_network_list, "Access points detected:" );

  // Populate the list
  uint8_t i = 0;
  //lv_obj_clean(wifi_network_list); // clear the list
  for( i = 0; i < no_of_wifi_networks; i++)
  {
    String list_button_text = (String)WiFi.SSID(i) + " " + LV_SYMBOL_LEFT + (String)WiFi.RSSI(i) + " dBm, heard on Channel " + (String)WiFi.channel(i);
    // Populate the list with what we found.
    lv_obj_t *list_button = lv_list_add_btn(wifi_network_list, LV_SYMBOL_WIFI, list_button_text.c_str());
    // Disable networks with WIFI_AUTH_ENTERPRISE in the list

    lv_obj_align(list_button, LV_ALIGN_CENTER, 0, 0);
    /*
    * A bit of explanation here
    * In the list, the user data to be transferred is the index as per WiFi.SSID(index)
    * The function expects a pointer. In this case, it's the value, cast as a pointer.
    * This is apparently fixed in newer LVGL versions.
    * See: https://forum.lvgl.io/t/passing-user-data-to-lv-obj-add-event-cb/6739/5
    */
    lv_obj_add_event_cb(list_button, wifi_ap_list_list_callback_function, LV_EVENT_ALL, (void *)i);

  }




  // Refresh button
  lv_obj_t *refresh_button = lv_btn_create(lv_scr_act());
  lv_obj_set_size(refresh_button, TFT_WIDTH/8, TFT_HEIGHT/8);
  lv_obj_align(refresh_button, LV_ALIGN_TOP_RIGHT, -TFT_WIDTH/100, TFT_WIDTH/100);
  // Refresh button: add icon
  lv_obj_t *refresh_button_icon = lv_label_create(refresh_button);
  lv_label_set_text(refresh_button_icon, LV_SYMBOL_REFRESH);
  lv_obj_align(refresh_button_icon, LV_ALIGN_CENTER, 0, 0);
  // Refresh button: callback function
  lv_obj_add_event_cb(refresh_button, wifi_ap_list_refresh_button_callback_function, LV_EVENT_ALL, NULL);




  // Connect button
  connect_button = lv_btn_create(lv_scr_act());
  lv_obj_set_size(connect_button, TFT_WIDTH/3, TFT_HEIGHT/8);
  lv_obj_align(connect_button, LV_ALIGN_BOTTOM_RIGHT, -TFT_WIDTH/100, -TFT_HEIGHT/100);
  // Connect button: Add icon
  lv_obj_t *connect_button_icon = lv_label_create(connect_button);
  lv_label_set_text(connect_button_icon, LV_SYMBOL_WIFI);
  lv_obj_align(connect_button_icon, LV_ALIGN_RIGHT_MID, 0, 0);
  // Connect button: Add label
  lv_obj_t *connect_button_label = lv_label_create(connect_button); // This also assigns the label as the button's child
  lv_label_set_text(connect_button_label, "Connect");
  lv_obj_center(connect_button_label);
  // Connect button: callback function
  lv_obj_add_event_cb(connect_button, wifi_ap_list_connect_button_callback_function, LV_EVENT_ALL, NULL);

  // Manual entry button
  lv_obj_t *manual_entry_button = lv_btn_create(lv_scr_act());
  lv_obj_set_size(manual_entry_button, TFT_WIDTH/3-TFT_WIDTH/25, TFT_HEIGHT/8);
  lv_obj_align(manual_entry_button, LV_ALIGN_BOTTOM_MID, 0, -TFT_HEIGHT/100);
  lv_obj_set_style_bg_color(manual_entry_button, (lv_color_t)lv_color_make(128, 64, 64), 0);
  // Manual entry button: Add label
  lv_obj_t *manual_entry_button_label = lv_label_create(manual_entry_button); // This also assigns the label as the button's child
  lv_label_set_text(manual_entry_button_label, "Manual entry");
  lv_obj_center(manual_entry_button_label);
  // Skip button: callback function
  lv_obj_add_event_cb(manual_entry_button, wifi_ap_list_manual_entry_button_callback_function, LV_EVENT_ALL, NULL);

  // Stay offline button
  lv_obj_t *button_stay_offline = lv_btn_create(lv_scr_act());
  lv_obj_set_size(button_stay_offline, TFT_WIDTH/3, TFT_HEIGHT/8);
  lv_obj_align(button_stay_offline, LV_ALIGN_BOTTOM_LEFT, TFT_WIDTH/100, -TFT_HEIGHT/100);
  // Stay offline button: Add icon
  lv_obj_t *button_stay_offline_icon = lv_label_create(button_stay_offline);
  lv_label_set_text(button_stay_offline_icon, LV_SYMBOL_CLOSE);
  lv_obj_align(button_stay_offline_icon, LV_ALIGN_LEFT_MID, 0, 0);
  // Stay offline button: Add label
  lv_obj_t *button_stay_offline_label = lv_label_create(button_stay_offline); // This also assigns the label as the button's child
  lv_label_set_text(button_stay_offline_label, "Stay offline");
  lv_obj_center(button_stay_offline_label);
  // Stay offline button: callback function
  lv_obj_add_event_cb(button_stay_offline, wifi_ap_list_stay_offline_button_callback_function, LV_EVENT_ALL, NULL);

}