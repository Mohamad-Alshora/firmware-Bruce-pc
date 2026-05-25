#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include "soc/soc_caps.h"
#include <stdint.h>

#define ADC_EN 14 // ADC_EN is the ADC detection enable port
#define ANALOG_BAT_PIN 34

// Lite Version
// #define LITE_VERSION 1

#define SPI_SS_PIN 33
#define SPI_MOSI_PIN 26
#define SPI_MISO_PIN 27
#define SPI_SCK_PIN 25

#define SDCARD_CS SPI_SS_PIN
#define SDCARD_SCK SPI_SCK_PIN
#define SDCARD_MISO SPI_MISO_PIN
#define SDCARD_MOSI SPI_MOSI_PIN

#define USE_CC1101_VIA_SPI
#define CC1101_GDO0_PIN 39
#define CC1101_SS_PIN 32
#define CC1101_MOSI_PIN SPI_MOSI_PIN
#define CC1101_SCK_PIN SPI_SCK_PIN
#define CC1101_MISO_PIN SPI_MISO_PIN

#define USE_NRF24_VIA_SPI
#define NRF24_CE_PIN 37
#define NRF24_SS_PIN 38
#define NRF24_MOSI_PIN SPI_MOSI_PIN
#define NRF24_SCK_PIN SPI_SCK_PIN
#define NRF24_MISO_PIN SPI_MISO_PIN

#define USE_W5500_VIA_SPI
#define W5500_SS_PIN 38
#define W5500_MOSI_PIN SPI_MOSI_PIN
#define W5500_SCK_PIN SPI_SCK_PIN
#define W5500_MISO_PIN SPI_MISO_PIN
#define W5500_INT_PIN 37

static const uint8_t SS = SPI_SS_PIN;
static const uint8_t MOSI = SPI_MOSI_PIN;
static const uint8_t SCK = SPI_MISO_PIN;
static const uint8_t MISO = SPI_SCK_PIN;

// Set Main I2C Bus
#define GROVE_SDA 21
#define GROVE_SCL 22
static const uint8_t SDA = GROVE_SDA;
static const uint8_t SCL = GROVE_SCL;

// LovyanGFX display configuration for GMT020 240x320 ST7789
#define USE_LOVYANGFX 1
#define LOVYAN_PANEL Panel_ST7789
#define LOVYAN_BUS Bus_SPI
#define LOVYAN_SPI_BUS 1

#define TFT_SPI_HOST SPI3_HOST
#define TFT_SPI_MODE 0
#define TFT_WRITE_FREQ 40000000
#define TFT_READ_FREQ 15000000
#define TFT_SPI_3WIRE true
#define TFT_USE_LOCK true
#define TFT_SCLK 18
#define TFT_MOSI 19
#define TFT_MISO -1
#define TFT_DC 16

// Panel configuration macros for full 240x320 display
#define TFT_CS 5
#define TFT_RST 23
#define TFT_BUSY_PIN -1
#define TFT_BL 4 // Backlight control pin
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define TFT_OFFSET_X 0
#define TFT_OFFSET_Y 0
#define TFT_INVERTION 0
#define TFT_RGB_ORDER 0
#define TFT_MEM_WIDTH 240
#define TFT_MEM_HEIGHT 320

// Display Setup
#define HAS_SCREEN
#define ROTATION 0
#define MINBRIGHT (uint8_t)1

// Font Sizes#
#define FP 1
#define FM 2
#define FG 3

// Serial
#define SERIAL_TX 12
#define SERIAL_RX 13

#define GPS_SERIAL_TX SERIAL_TX
#define GPS_SERIAL_RX SERIAL_RX

#define BAD_TX 12
#define BAD_RX 13

// Buttons & Navigation
#define BTN_ALIAS "\"OK\""
#define HAS_3_BUTTONS
#define UP_BTN 17
#define DW_BTN 35
#define BTN_ACT LOW

// IR pins
#define TXLED 2
#define RXLED 15

#define LED_ON HIGH
#define LED_OFF LOW

#endif /* Pins_Arduino_h */
