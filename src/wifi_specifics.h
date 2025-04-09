#ifdef __cplusplus
extern "C" {
#endif

// This function just makes the wifi module scan for APs
void wifi_scan_for_aps(void);

// If we need to connect as a client, we can do so here.
void wifi_client_task(void);

// Get time using NTP, and set the local clock.
void wifi_get_time_ntp(void);

// Here, we set up an access point for our devices to connect to.
void wifi_ap_set(void);

// Initial wifi greeter screen.
void wifi_start_screen(void);

// If the user pressed 'Scan for networks', this gets started.
void wifi_scan_screen(void);

// This shows the scanned wifi APs.
void wifi_ap_list_screen(void);

#ifdef __cplusplus
}
#endif