#ifdef __cplusplus
extern "C" {
#endif

// This function just makes the wifi module scan for APs
void wifi_scan_for_aps(void);

// If we need to connect as a client, we can do so here.
void wifi_client_task(void);

// Here, we set up an access point for our devices to connect to.
void wifi_ap_set(void);

void ssid_input_screen(void);

void input_field_event_callback_function(lv_event_t * e);

#ifdef __cplusplus
}
#endif