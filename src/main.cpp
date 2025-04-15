#include <Arduino.h>
#include "Guition_ESP32_4848S040.h" // Pin definitions and other goodies.
#include <Ticker.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>
#include <demos/lv_demos.h>
#include <TAMC_GT911.h>
#include <esp_heap_caps.h> // to be able to use the SPIRAM (aka PSRAM)
#include "keyboard_example_scene.h"
#include <WiFi.h>
#include <WifiAP.h>
#include <time.h>
/*
 * Each screen is moved to a separate file wit their header files,
 * But, there is also a summarised header file too.
 * If you add to this, then have the following structure:
 * |
 * |-wifi_selector_screens
 * >-[my_fancy_code.c] which is where you do the work
 * >-[my_fancy_code.h] which shows the globally accessible functions
 * ...and add #iinclude "wifi_selector_screens/my_fancy_code.h" to:
 * wifi_selector_screens/wifi_selector_screens.h
 */
#include "wifi_selector_screens/wifi_selector_screens.h"


/*
 * Global variables go here
*/
// Wifi specific stuff.
char wifi_ssid_to_connect[32];
char wifi_password_to_connect[32];
int16_t no_of_wifi_networks = 0; // We use this as a scan status indicator.
uint8_t wifi_selected_network_index = 255;
uint8_t wifi_need_to_connect = 0; // 0 don't connect, everything else, connect.
char wifi_ap_ssid[32] = "Suspiciously open WiFi network";
char wifi_ap_password[32] = "passw"; // A valid password must have at least 7 characters.
TaskHandle_t wifi_task_handle = NULL;
uint8_t we_have_accurate_time = 0;
time_t now;
tm posixtime;


// Make sure you add -DLV_CONF_INCLUDE_SIMPLE to build_flags section in platformio.ini too if you are gettng errors about missing lv_conf.
//#define LV_CONF_INCLUDE_SIMPLE
#define TICKER_MS 5

// Software SPI
Arduino_DataBus *sw_spi_bus = new Arduino_SWSPI( GFX_NOT_DEFINED /* DC pin */, TFT_CS /* CS pin of display*/, TFT_SCK /* Clock */, TFT_SDA /* MOSI */, GFX_NOT_DEFINED /* Data in */);

// Display hardware definition. The display data is loaded in parallel ST7701.
Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
  TFT_DE /* DE */, TFT_VS /* VSYNC */, TFT_HS /* HSYNC */, TFT_PCLK /* PCLK */,
  TFT_R0_DB13 /* R0 */, TFT_R1_DB14 /* R1 */, TFT_R2_DB15 /* R2 */, TFT_R3_DB16 /* R3 */, TFT_R4_DB17 /* R4 */,
  TFT_G0_DB6 /* G0 */, TFT_G1_DB7 /* G1 */, TFT_G2_DB8 /* G2 */, TFT_G3_DB9 /* G3 */, TFT_G4_DB10 /* G4 */, TFT_G5_DB11 /* G5 */,
  TFT_B0_DB1 /* B0 */, TFT_B1_DB2 /* B1 */, TFT_B2_DB3 /* B2 */, TFT_B3_DB4 /* B3 */, TFT_B4_DB5 /* B4 */,
  TFT_HSYNC_POLARITY /* 0 or 1? */, TFT_HSYNC_FRONT_PORCH, TFT_HSYNC_PULSE_WIDTH, TFT_HSYNC_BACK_PORCH,
  TFT_VSYNC_POLARITY /* 0 or 1? */, TFT_VSYNC_FRONT_PORCH, TFT_VSYNC_PULSE_WIDTH, TFT_VSYNC_BACK_PORCH,
  TFT_PCLK_ACTIVE_NEG /* Active low? Falling edge? */, TFT_DATA_SPEED, TFT_USE_BIG_ENDIAN
);


// As of Adrduino_GFX 1.5.5:
Arduino_RGB_Display *tft = new Arduino_RGB_Display(
  TFT_WIDTH, TFT_HEIGHT, rgbpanel, ROTATION, TFT_AUTO_FLUSH,
  sw_spi_bus /* Arduino Data bus */, GFX_NOT_DEFINED /* Reset pin, internal*/,
  st7701_type9_init_operations, sizeof(st7701_type9_init_operations)
);

// Touch object
TAMC_GT911 touch_panel(TP_SDA, TP_SCL, TP_INT, TP_RST, TFT_WIDTH, TFT_HEIGHT);

// Ticker
Ticker ticker;

/*
 * LVGL.
*/
// Main pointers
static lv_disp_draw_buf_t draw_buffer;
static lv_color_t *frame_buffer;
static lv_disp_drv_t display_driver;
static lv_disp_t *display_object; // An all-in-one object.

// Display updater function
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    // tft->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h) // Swap colours;
    tft->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
    tft->flush(); // Do the actual flushing

    lv_disp_flush_ready(disp);
}

//This function is executed every LVGL_TICKER_MS milliseconds.
void ticker_call_function(void)
{
  // Since the serial port here doesn't work, I blink the backlight.
  //digitalWrite(TFT_BL, !digitalRead(TFT_BL)); // Oldest trick in the book.
  lv_tick_inc(TICKER_MS);
  lv_task_handler();
}

// Clear screen function.
void clear_screen(void)
{
  // We 'clear' the screen, by adding a large object.
  lv_obj_t *background = lv_obj_create(lv_scr_act());
  lv_obj_set_size(background, TFT_WIDTH, TFT_HEIGHT);
  lv_obj_align(background, LV_ALIGN_CENTER, 0, 0);
}

// Touch panel callback function. LVGL 8.4.0 does not support multitouch.
void my_input_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  touch_panel.read();
  {
    if (touch_panel.isTouched)
    {
      data->state = LV_INDEV_STATE_PRESSED;
      // Since no multitouch, get the first point.
      data->point.x = touch_panel.points[0].x;
      data->point.y = touch_panel.points[0].y;
    }
    else
    {
      data->state = LV_INDEV_STATE_RELEASED;
    }
  }
}

void setup() {
  // Serial port
  Serial.begin(115200);
  Serial.printf("Available PSRAM: %d KB\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM)>>10);

  // Ticker
  ticker.attach_ms(TICKER_MS, ticker_call_function);

  // Backlight
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // Relays or I2S
  #ifdef USE_RELAYS
  // We use relays
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  // All off, by default.
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, LOW);
  #else
  // I2S. Not implemented yet, let the next victim know.
  #pragma message("Looks like you want to use I2S in this board. See Guition_ESP32_4848S040.h, or the hardware documentation.")
  #endif
  // Wifi.
  WiFi.mode(WIFI_STA); // Start as client
  vTaskDelay(100); // Delay a bit for the other code to handle the adapter
  WiFi.scanNetworks(true); // True is Async


  // Display hardware
  tft->begin();
  // Test: Throw some pixels out to check that the low-level stuff works.
  tft->flush();
  for (uint16_t x_coord = 0; x_coord < TFT_WIDTH; x_coord++)
  {
    for (uint16_t y_coord = 0; y_coord < TFT_HEIGHT; y_coord++)
    {
      // X, Y, colour. In this case, 16 bits.
      tft -> writePixel(x_coord, y_coord, tft->color565( x_coord<<1, (x_coord + y_coord)<<2, y_coord<<1));
    }
  }
  tft->flush();

  // Touch panel
  touch_panel.begin();
  // The display rotation is not the same as the touch panel rotation, see Guition_ESP32_4848S040.h
  touch_panel.setRotation(TAMC_GT911_ROTATION);
  touch_panel.setResolution(TFT_WIDTH, TFT_HEIGHT); // During initialisation, it doesn't get the right coordinates

  //Start LVGL
  lv_init(); // Start the dance

  // Initialise an entire frame's buffer in the SPI RAM
  frame_buffer = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * TFT_WIDTH * TFT_HEIGHT, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

  // If the PSRAM is not initialised, this should fail. 480x480x2=460800 -> 450 kB
  if(frame_buffer == NULL)
  {
  Serial.println("Unable to allocate memory for the frame buffer. Do you have enough PSRAM?\n");
  while(1);
  }

  // Initialise draw buffer, and assign it to the frame buffer.
  lv_disp_draw_buf_init(&draw_buffer, frame_buffer, NULL, TFT_WIDTH * TFT_HEIGHT);

  // Initialise the display driver, and set some basic details.
  lv_disp_drv_init(&display_driver);
  display_driver.hor_res = TFT_WIDTH;
  display_driver.ver_res = TFT_HEIGHT;
  display_driver.flush_cb = my_disp_flush; // Assign callback for display update
  display_driver.full_refresh = 0; // Always redraw the entire screen. This makes it slower
  display_driver.draw_buf = &draw_buffer; // The memory address where the draw buffer begins

  // Finally, register this display
  display_object = lv_disp_drv_register(&display_driver);


  // Initialise the touch panel driver
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER; // No multitouch :()
  indev_drv.read_cb = my_input_read; // This is where we read the touch controller
  lv_indev_drv_register(&indev_drv);


  // By this time, the initial wifi scan must have produced something
  no_of_wifi_networks = WiFi.scanComplete();

  // Print something
  //lv_obj_t *label = lv_label_create( lv_scr_act() );
  //lv_label_set_text( label, "LVGL V" GFX_STR(LVGL_VERSION_MAJOR) "." GFX_STR(LVGL_VERSION_MINOR) "." GFX_STR(LVGL_VERSION_PATCH));
  //lv_obj_align( label, LV_ALIGN_CENTER, 0, -20 );


  wifi_ap_list_screen();
  //wifi_manual_ssid_input_screen();
  //wifi_password_input_screen();
  //wifi_start_screen();
  //wifi_scan_for_aps();

  //clear_screen();
  // Straight from the examples: https://docs.lvgl.io/8.4/examples.html?highlight=keyboard
  //lv_example_keyboard_1();

}

void loop() {


}

