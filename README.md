# Guition ESP32-4848S040 PlatformIO project

This is not like the included examples, these use the latest Arduino GFX (1.5.6) and slightly more recent LVGL (8.4.0) libraries. I am doing this because I got frustrated that everyone seem to use the older libraries and the code was a mess. Also, there are breaking changes in the newer versions, which should at least be tested. I did this so I can use this as a skeleton for my projects.

I wrote this as I found and solved problems. There is no structure to speak of, it is merely a sequence of what broke when.

## Key differences

* Uses Arduino framework on PlatformIO
* Custom header file for driving hardware
* Touch panel and screen now can be rotated together

## Environment set-up

The IO pins are defined in `src/Guition_ESP32_4848S040.h`. The device on board is an ESP32-S3-N16R8, so it has 16 MB (Quiad SPI) Flash, and 8 MB (Octal SPI) PSRAM.


The `platformio.ini` is so far:

```ini
[env:Guition_ESP32_4848S040]
platform = espressif32
framework = arduino
board = esp32-s3-devkitm-1
; A couple of overrides.
board_build.arduino.memory_type = qio_opi
board_build.flash_mode = qio
board_build.psram_type = opi
board_upload.flash_size = 16MB
board_upload.maximum_size = 16777216
monitor_speed = 115200
build_flags =
    -UARDUINO_USB_CDC_ON_BOOT=0  ; We have a ch340 chip on its uart
    -DBOARD_HAS_PSRAM
    -DLV_CONF_INCLUDE_SIMPLE
lib_deps =
    https://github.com/moononournation/Arduino_GFXl
    lvgl/lvgl@^8.4.0
    https://github.com/tamctec/gt911-arduino
```

In `Setup()`, the available SPI RAM (PSRAM) the very first thing it prints:

```C
 // Serial port
Serial.begin(115200);
Serial.printf("Available PSRAM: %d KB\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM)>>10);
```

## Display

Even if the datasheet and the schematic says it's loaded in parallel, the control parts are over SPI. For some reason, I only could get it to work with Software SPI, which may cause problems with the SD card later-on.

The code so far:

```C++
// Software SPI
Arduino_DataBus *bus = new Arduino_SWSPI( GFX_NOT_DEFINED /* DC pin */, TFT_CS /* CS pin of display*/, TFT_SCK /* Clock */, TFT_SDA /* MOSI */, GFX_NOT_DEFINED /* Data in */);

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
  bus /* Arduino Data bus */, GFX_NOT_DEFINED /* Reset pin, internal*/,
  st7701_type9_init_operations, sizeof(st7701_type9_init_operations)
);
```

....and then

```C++
tft->begin();
tft->do_whatever(what_other_ever_input);
tft->flush(); // So that I can control the flushing from LVGL.
```

There is some test code that creates a bunch of colourful pixels. This is a low-level thing that proves that the display library is accessing the screen properly.

## LVGL

I kinda gave up on lvgl 9, because I had to allocate the entire frame buffer in the SPI RAM, and all I got is garbage on the screen. [According to this issue on GitHub](https://github.com/lvgl/lvgl/issues/7402), someone solved this by setting `SPIRAM_MALLOC_ALWAYSINTERNAL` to 0 or at least a very low number. So anything more than X bytes would automatically be shoved to the SPI RAM.

Instead, I switched back to lvgl 8.4.0, which worked pretty much instantly.

In order to do this, one needs to change this line in `platformio.ini`:

```ini
lib_deps=
    lvgl/lvgl@^8.4.0
```

Generally speaking, there are a number of prerequisites to use LVGL.

### The *tick callback function* that is executed periodically.

To me, this seems to work, and I don't have to put anything in `Loop()`:

```C
#include <Ticker.h>
#includ <lvlg.h>

#define TICKER_MS 5

// Ticker
Ticker ticker;

//This function is executed every LVGL_TICKER_MS milliseconds.
void ticker_call_function(void)
{
  // Since the serial port here doesn't work, I blink the backlight.
  //digitalWrite(TFT_BL, !digitalRead(TFT_BL)); // Oldest trick in the book.
  lv_tick_inc(TICKER_MS);
  lv_task_handler();
}

void setup()
{
  // Ticker
  ticker.attach_ms(TICKER_MS, ticker_call_function);
}
```

### Display driver and buffer allocation

In LVGL 8, there is the `draw_buffer` where the local graphic mambo-jambo happens, the `frame_buffer` or `display_buffer`, there the actual picture data is stored. In the Guition ESP32-4848S040 board, the display is directly (HSync - VSync - Pixel Clock, 16-bit RGB data) driven, so it seems that the display may need to be fully refreshed every time.

Storing a 480x480 display in 16-bit colour depth needs about 450 kB of memory, which is more than the ESP32's internal SRAM. Luckily, the board came with the N16R8 variant, so there is a whopping extra 8 MB RAM available.

To access this RAM, one needs to allocate this with `heap_caps_malloc( <x_number_of_bytes>), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);`

lvgl 8 also needs a `display_driver` pointer, which is used to configure where is where. It also turned out that the entire screen does NOT need to be re-drawn all the time. So probably in lvgl 9, one could use `LV_DISPLAY_RENDER_MODE_PARTIAL` instead of `LV_DISPLAY_RENDER_MODE_DIRECT`

```C
#include <lvgl.h>
// Main pointers
static lv_disp_draw_buf_t draw_buffer;
static lv_color_t *frame_buffer;
static lv_disp_drv_t display_driver;


void Setup()
{
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
  display_driver.full_refresh = 0; // Always redraw the entire screen.
  display_driver.draw_buf = &draw_buffer; // The memory address where the draw buffer begins

  // Finally, register this display
  lv_disp_drv_register(&display_driver);
}
```

In lvgl 9, the above code changes to:

```C
#include <lvgl.h>

lv_display_t *disp;
// LVGL's internal frane buffer
#define DISPLAY_BUFFER_SIZE TFT_WIDTH * TFT_HEIGHT * 2

void Setup()
{
  lv_init(); // Start

  disp = lv_display_create(TFT_HEIGHT, TFT_WIDTH); // Create display

  void *frame_buffer = heap_caps_malloc(DISPLAY_BUFFER_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if(buffer == NULL)
  {
    Serial.println("Couldn't allocate memory for the display buffer.");
    while(1);
  }

  lv_display_set_buffers(disp, frame_buffer, NULL, DISPLAY_BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_DIRECT);

  lv_display_set_flush_cb(disp, my_disp_flush); // Callback function for updating the display.
}
```

### The  **flush** function, where the display buffer is being 'uploaded' to the display.

In this case, this is `my_disp_flush()`, which can be seen in oh so many examples.

* LVGL 8:

```C
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  // tft->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h) // Swap colours;
  tft->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
  tft->flush(); // Do the actual flushing

  lv_disp_flush_ready(disp);
}
```

* LVGL 9:

```C
void my_disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* frame_data)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  // Load the data tot he display
  tft->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&frame_data, w, h);

  // Because auto flush is disabled.
  tft->flush();
  lv_disp_flush_ready(disp);
}
 ```

### Some input device

In this case, the Guition ESP32-4848S040 board has a GT911 touch panel on the screen. The rotations do not correspond, and as of LVGL 8.4.0, there is no multitouch support. LVGL 9 does support gestures though.

In `platformio.ini`, add the library:

```ini
lib_deps=
    https://github.com/tamctec/gt911-arduino
```

Low-level hardware set-up

```C
#include <TAMC_GT911.h>

// Touch object
TAMC_GT911 touch_panel(TP_SDA, TP_SCL, TP_INT, TP_RST, TFT_WIDTH, TFT_HEIGHT);

void Setup()
{
  // Touch panel
  touch_panel.begin();
  // The display rotation is not the same as the touch panel rotation, see Guition_ESP32_4848S040.h
  touch_panel.setRotation(TAMC_GT911_ROTATION);
  touch_panel.setResolution(TFT_WIDTH, TFT_HEIGHT); // During initialisation, it doesn't get the right coordinates
}
```

* LVGL 8:

```C
// Touch panel callback function LVGL 8.4.0 does not support multitouch.
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

void Setup()
{
  // Initialise the touch panel driver
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER; // No multitouch :()
  indev_drv.read_cb = my_input_read; // This is where we read the touch controller
  lv_indev_drv_register(&indev_drv);

}
```

* LVGL 9:

```C
// Touch panel callback function.
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

// LVGL 9 supports multi-touch gestures, but I haven't been able to play with this yet.


void Setup()
{
  // Create input device
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_input_read);
}

```

## Adding custom things

There are [some subtle differences between C and C++]( https://community.platformio.org/t/files-in-lib-compile-but-linker-cant-find-them-resolved/10489). I got an example to run by following this structure:

```
├── src
    ├── custom_thing.h
    └── custom_thing.c
```

In `custom_thing.h`, add the following:

```C
#ifdef __cplusplus
extern "C" {
#endif

void your_fancy_function(whatever_input_arguments);

#ifdef __cplusplus
}
#endif
```
