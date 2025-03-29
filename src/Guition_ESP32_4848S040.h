/*
 * Pin assignments for the Guition_ESP32_4848S040 board.
 * This is pretty much written from scratch.
*/

#include <stdint.h>


// I2C
#define SCL 45
#define SDA 19

// Touch panel
#define TP_SCL SCL
#define TP_SDA SDA
#define TP_INT -1
#define TP_RST -1


// I2S and Relays
/*
 * Notice:
 * If you want to use I2S, you won't be able to use the relays.
 * The 0 ohm resistors R25, R26, and R27 should be moved to R21, R22, and R23.
 * Then, comment out the following define.
*/
#define USE_RELAYS

#ifdef USE_RELAYS

    #define RELAY1 40
    #define RELAY2 2
    #define RELAY3 1

#else

    #define DIN 40
    #define LRCLK 2
    #define BCLK 1

#endif

// Micro SD card, SPI.
#define SDCARD_CS   42
// The Expressif Arduino library wants SS defined. Apparently.
#define SS SDCARD_CS
#define SDCARD_MISO 47
#define SDCARD_MOSI 41
#define SDCARD_SCK  48


/*
 * TFT-panel-spefific stuff.
 * According to the schematic, both the serial and parallel interfaces are wired up.
 * Controller is an ST7701, 480x480 pixels, 4 inches diameter
*/
// I get some glitching at 16 MHz, so let's slow it down a little
#define TFT_DATA_SPEED 13000000

// TFT panel pin definitions
#define TFT_SDA 47
#define TFT_SCK 48
#define TFT_CS 39

#define TFT_PCLK 21
#define TFT_DE 18
#define TFT_VS 17
#define TFT_HS 16

#define TFT_B0_DB1 4
#define TFT_B1_DB2 5
#define TFT_B2_DB3 6
#define TFT_B3_DB4 7
#define TFT_B4_DB5 15

#define TFT_G0_DB6 8
#define TFT_G1_DB7 20
#define TFT_G2_DB8 3
#define TFT_G3_DB9 46
#define TFT_G4_DB10 9
#define TFT_G5_DB11 10

// there is no DB12 on the schematic. Not sure why.
#define TFT_R0_DB13 11
#define TFT_R1_DB14 12
#define TFT_R2_DB15 13
#define TFT_R3_DB16 14
#define TFT_R4_DB17 0

// Backlight. This works with a boost converter, so you can forget about PWM dimming.
#define TFT_BL 38

// Defines for the TFT driver.

#define TFT_WIDTH 480
#define TFT_HEIGHT 480
#define ROTATION 2

#define TFT_16BIT_BGR_FORMAT true
// I am assuming that timings are in nanoseconds
// These are from: https://github.com/moononournation/Arduino_GFX/blob/fcbe67fbcd2081a259a7d99c8197b9ad6442764a/examples/PDQgraphicstest/Arduino_GFX_dev_device.h#L229
#define TFT_HSYNC_POLARITY 1
#define TFT_HSYNC_FRONT_PORCH 10
#define TFT_HSYNC_PULSE_WIDTH 8
#define TFT_HSYNC_BACK_PORCH 50


#define TFT_VSYNC_POLARITY 1
#define TFT_VSYNC_FRONT_PORCH 10
#define TFT_VSYNC_PULSE_WIDTH 8
#define TFT_VSYNC_BACK_PORCH 20

#define TFT_PCLK_ACTIVE_NEG 0
#define TFT_USE_BIG_ENDIAN false

// Arduino GFX-specific stuff.
#define TFT_AUTO_FLUSH false

// This is for use with the TAMC_GT911 library: make sure that the screen rotates together with the touch coordinates.
#if defined(ROTATION) && ROTATION == 0
#define TAMC_GT911_ROTATION 1
#elif defined(ROTATION) && ROTATION == 1
#define TAMC_GT911_ROTATION 2
#elif defined(ROTATION) && ROTATION == 2
#define TAMC_GT911_ROTATION 3
#elif defined(ROTATION) && ROTATION == 3
#define TAMC_GT911_ROTATION 0
#endif