#ifdef __cplusplus
extern "C" {
#endif

/*
 * This header file is for the SD card-specifics, namely:
 - Mount the card
 - Load the text files if possible and set the global strings
*/

void mount_sd_card(void);

void load_files(void);

void connect_to_wifi_and_sync_time(void);

void set_up_wifi_ap(void);

#ifdef __cplusplus
}
#endif