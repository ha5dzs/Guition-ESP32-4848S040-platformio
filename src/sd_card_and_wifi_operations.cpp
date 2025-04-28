#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <time.h>
#include <FS.h>
#include <sd_diskio.h>
#include <SD.h> // SD card is wired up for SPI mode. It will be slow, but that's OK
#include "sd_card_and_wifi_operations.h"
#include "Guition_ESP32_4848S040.h"

/*
 * Global variables
*/

extern char wifi_password_to_connect[32];
extern char wifi_ssid_to_connect[32];
extern char wifi_ap_ssid[32];
extern char wifi_ap_password[32];
// low-level TFT access, before we use LVGL.
extern Arduino_RGB_Display *tft;


/*
 * Local variables shared across these functions
*/

int16_t cursor_position = 0;
bool wifi_ap_password_set = false;

/*
 * Attempt to mount the SD card.
*/
void mount_sd_card(void)
{
    // So one can read the debug messages before LVGL starts.
    tft->setTextColor(0x7e0); // Green, RGB565.
    tft->setTextSize(2);

    // SD card.
    if(!SD.begin(SDCARD_CS))
    {
        Serial.println("Mounting SD card failed.");
        // Put the message on the screen.
        tft->setCursor(0, cursor_position);
        tft->printf("Could not mount the SD card.");
        tft->flush();

    }
    else
    {
        tft->setCursor(0, 0);
        tft->printf("Mounting SD card OK, size is %d GB.", SD.cardSize() >> 30);
        tft->flush();
    }
}

/*
 * Load in the plain-text config files.
 * This is all hard-coded, but optional. Feel free to comment it out.
*/
void load_files(void)
{



    cursor_position = 20;
    /*
     * connect_to_ssid.txt
    */
    File ssid_config_file = SD.open("/connect_to_ssid.txt");
    if(!ssid_config_file)
    {
        // If we got here, the file is missing
        tft->setCursor(0, cursor_position);
        tft->printf("/connect_to_ssid.txt is missing, won't connect anywhere.");
        tft->flush();
        delay(1000);
    }
    else
    {
        // The file should not be longer than 32 bytes.
        uint32_t ssid_config_file_length = ssid_config_file.size();
        if(ssid_config_file_length > 32)
        {
            tft->setCursor(0, cursor_position);
            tft->printf("/connect_to_ssid.txt longer than 32 bytes, won't connect anywhere.");
            tft->flush();
        }
        else
        {
            // If we got here, then we can copy the file contents to the SSID string.
            uint8_t i = 0; // in this case, we use this to go through the file contents
            while(ssid_config_file.available())
            {
                wifi_ssid_to_connect[i] = ssid_config_file.read();
                i++;
            }
            // and in the end, we put a \0.
            wifi_ssid_to_connect[i] = 0x00;
            tft->setCursor(0, cursor_position);
            tft->printf("SSID: %s", wifi_ssid_to_connect);
            tft->flush();
        }
    }

    /*
     * connect_to_password.txt
    */
    cursor_position = 40;
    File password_config_file = SD.open("/connect_to_password.txt");
    if(!password_config_file)
    {
        // If we got here, the file is missing
        tft->setCursor(0, cursor_position);
        tft->printf("/connect_to_password.txt is missing");
        tft->flush();
        delay(1000);
    }
    else
    {
        // The password file should not be longer than 64 bytes.
        uint32_t ssid_password_file_length = password_config_file.size();
        if(ssid_password_file_length > 64 || ssid_password_file_length < 8 )
        {
            tft->setCursor(0, cursor_position);
            tft->printf("/connect_to_password.txt must be between 8 and 64 bytes.");
            tft->flush();
        }
        else
        {
            // If we got here, then we can copy the file contents to the SSID string.
            uint8_t i = 0; // in this case, we use this to go through the file contents
            while(password_config_file.available())
            {
                wifi_password_to_connect[i] = password_config_file.read();
                i++;
            }
            // and in the end, we put a \0.
            wifi_password_to_connect[i] = 0x00;
            tft->setCursor(0, cursor_position);
            tft->printf("Password: saved to memory");
            tft->flush();
        }
    }

    /*
     * create_ssid.txt
    */
   cursor_position = 60;
   File create_ssid_config_file = SD.open("/create_ssid.txt");
   if(!create_ssid_config_file)
   {
       // If we got here, the file is missing
       tft->setCursor(0, cursor_position);
       tft->printf("/create_ssid.txt is missing.");
       tft->flush();
       delay(1000);
   }
   else
   {
       // The file should not be longer than 32 bytes.
       uint32_t create_ssid_file_length = create_ssid_config_file.size();
       if(create_ssid_file_length > 32)
       {
           tft->setCursor(0, cursor_position);
           tft->printf("/create_ssid.txt must be less than 32 bytes.");
           tft->flush();
       }
       else
       {
           // If we got here, then we can copy the file contents to the SSID string.
           uint8_t i = 0; // in this case, we use this to go through the file contents
           while(create_ssid_config_file.available())
           {
                wifi_ap_ssid[i] = create_ssid_config_file.read();
                i++;
           }
           // and in the end, we put a \0.
           wifi_ap_ssid[i] = 0x00;
           tft->setCursor(0, cursor_position);
           tft->printf("This AP: %s", wifi_ap_ssid);
           tft->flush();
       }
   }


    /*
     * wifi_ap_password.txt
    */
    cursor_position = 80;
    File wifi_ap_password_config_file = SD.open("/create_password.txt");
    if(!wifi_ap_password_config_file)
    {
        // If we got here, the file is missing
        tft->setCursor(0, cursor_position);
        tft->printf("/create_password.txt is missing");
        tft->flush();
        delay(1000);
    }
    else
    {
        // The password file should not be longer than 64 bytes.
        uint32_t wifi_ap_password_file_length = wifi_ap_password_config_file.size();
        if(wifi_ap_password_file_length > 64 || wifi_ap_password_file_length < 8)
        {
            tft->setCursor(0, cursor_position);
            tft->printf("/create_password.txt must be between 8 and 64 bytes.");
            tft->flush();
        }
        else
        {
            // If we got here, then we can copy the file contents to the SSID string.
            uint8_t i = 0; // in this case, we use this to go through the file contents
            while(wifi_ap_password_config_file.available())
            {
                wifi_ap_password[i] = wifi_ap_password_config_file.read();
                i++;
            }
            // and in the end, we put a \0.
            wifi_ap_password[i] = 0x00;
            tft->setCursor(0, cursor_position);
            tft->printf("passwd: %s", wifi_ap_password);
            tft->flush();
            wifi_ap_password_set = true; // If loaded properly, set this as AP password.
        }
    }

}

void connect_to_wifi_and_sync_time(void)
{
    /*
     * Connect to wifi.
    */

    cursor_position = 100;

    uint8_t retry_counter  = 0;
    uint8_t max_retries = 30;

    tft->setCursor(0, cursor_position);

    tft->flush();
    WiFi.begin(wifi_ssid_to_connect, wifi_password_to_connect);

    while(retry_counter <= max_retries)
    {
        if(WiFi.status() != WL_CONNECTED)
        {
            tft->setCursor(0, cursor_position);
            tft->printf("Connecting: %s", wifi_ssid_to_connect);
            tft->flush();
            Serial.printf("Connecting to %s... (%d/%d)\n", wifi_ssid_to_connect, retry_counter, max_retries);
            delay(1000);
        }
        else
        {
            tft->setCursor(0, cursor_position + 20);
            tft->printf("Connected, IP:  %s", WiFi.localIP().toString());
            tft->flush();
            break;
        }
        retry_counter++;
    }

    if(retry_counter >= max_retries)
    {
        tft->setCursor(0, cursor_position + 20);
        tft->printf("Giving up on %s", wifi_ssid_to_connect);
        tft->flush();
        // If we got here, there is no point in trying to sync time, because there is no network connection.
        return;
    }


    /*
     * Sync time
     * All timestamps are going to be in UTC
     * Time server is hard-coded to some popular NTP servers.
    */
    cursor_position = 140;

    // If we got here, we have a network connection.
    struct tm utc_time;
    // Get NTP time. I think this is blocking execution
    configTime(0, 0, "pool.ntp.org", "time.windows.com", "time.google.com");

    if(!getLocalTime(&utc_time))
    {
        tft->setCursor(0, cursor_position);
        tft->printf("Unable to get time. Network problem?");
        tft->flush();
    }
    else
    {
        tft->setCursor(0, cursor_position);
        // See tm structure definition: https://cplusplus.com/reference/ctime/tm/
        tft->printf("Time synced on: %02.0f/%02.0f/%d %2.0f:%2.0f:%2.0f", (float)utc_time.tm_mday, (float)utc_time.tm_mon+1, utc_time.tm_year-100, (float)utc_time.tm_hour, (float)utc_time.tm_min, (float)utc_time.tm_sec);
        tft->flush();
    }

    // Disconnect.
    cursor_position = 160;
    WiFi.disconnect();

    tft->setCursor(0, cursor_position);
    tft->print("Wifi client disconnected.");
    tft->flush();

    return;
}

/*
 * This function sets the wifi AP as per the config file.
*/
void set_up_wifi_ap(void)
{
    cursor_position = 200;
    tft->setCursor(0, cursor_position);
    tft->printf("Setting up: %s", wifi_ap_ssid);
    if(wifi_ap_password_set)
    {
        tft->setCursor(0, cursor_position + 20);
        tft->printf("Password: %s", wifi_ap_password);
    }
    else
    {
        tft->setCursor(0, cursor_position + 20);
        tft->printf("<NO PASSWORD, OPEN AP>");
    }
    tft->flush();

    WiFi.mode(WIFI_AP);

    WiFi.softAPsetHostname("Guition_ESP32_4848S040");

    // Change the local IP addresses
    IPAddress access_point_ip(192,168,54,1);
    IPAddress default_gateway(192,168,54,1);
    IPAddress subnet_mask(255,255,255,0); // 255 hosts. Perhaps a bit too ambitious.

    WiFi.softAPConfig(access_point_ip, default_gateway, subnet_mask);

    if(wifi_ap_password_set)
    {
        WiFi.softAP(wifi_ap_ssid, wifi_ap_password);
    }
    else
    {
        // If no password set, then just set up the network as it is.
        WiFi.softAP(wifi_ap_ssid);
    }

}