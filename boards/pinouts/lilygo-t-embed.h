#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>
#include "soc/soc_caps.h"

// Lite Version
// #define LITE_VERSION 1

// Main I2C Bus
#define SPI_SS_PIN      17
#define SPI_MOSI_PIN    41
#define SPI_MISO_PIN    40
#define SPI_SCK_PIN     38
static const uint8_t SS    = SPI_SS_PIN;
static const uint8_t MOSI  = SPI_MOSI_PIN;
static const uint8_t SCK   = SPI_MISO_PIN;
static const uint8_t MISO  = SPI_SCK_PIN;

// Set Main I2C Bus
#define GROVE_SDA 44
#define GROVE_SCL 43
static const uint8_t SDA = GROVE_SDA;
static const uint8_t SCL = GROVE_SCL;

// Display Setup#
#define HAS_SCREEN
#define ROTATION        3
#define MINBRIGHT       (uint8_t) 1
#define PIN_POWER_ON    46

// Font Sizes#
#define FP  1
#define FM  2
#define FG  3

// SD Card#
#define SDCARD_CS   39
#define SDCARD_MISO SPI_MISO_PIN
#define SDCARD_MOSI SPI_MOSI_PIN
#define SDCARD_SCK  SPI_SCK_PIN

// NRF24 - Over QWIIC Port #
#define USE_NRF24_VIA_SPI
#define NRF24_CE_PIN    44
#define NRF24_SS_PIN    43
#define NRF24_MOSI_PIN  SDCARD_MOSI
#define NRF24_SCK_PIN   SDCARD_SCK
#define NRF24_MISO_PIN  SDCARD_MISO

// CC1101#
#define USE_CC1101_VIA_SPI
#define CC1101_GDO0_PIN 44
#define CC1101_SS_PIN   43
#define CC1101_MISO_PIN SPI_MISO_PIN
#define CC1101_MOSI_PIN SPI_MOSI_PIN
#define CC1101_SCK_PIN  SPI_SCK_PIN

// Mic#
#define MIC_SPM1423
#define PIN_CLK         21
#define PIN_DATA        14

// Speaker#
#define HAS_NS4168_SPKR
#define BCLK    7
#define WCLK    5
#define DOUT    6
#define MCLK    39

// Serial
#define SERIAL_TX 43
#define SERIAL_RX 44

#define BAT_PIN 4

// Encoder
#define HAS_ENCODER
#define ENCODER_INA	2
#define ENCODER_INB	1
#define ENCODER_KEY	0
#define HAS_BTN	    1
#define BTN_ALIAS	'"Mid"'
#define SEL_BTN	    ENCODER_KEY
#define UP_BTN	    1
#define DW_BTN	    1
#define BTN_ACT	    LOW

// IR
#define LED	        44
#define RXLED	    43
#define LED_ON	    HIGH
#define LED_OFF	    LOW

// RF Module

// FM
// #define FM_SI4713
// #define FM_RSTPIN 40

// RGB LED
#define HAS_RGB_LED
#define RGB_LED             42
#define RGB_LED_CLK         45
#define LED_TYPE            APA102
#define LED_ORDER           RGB
#define LED_TYPE_IS_RGBW    0
#define LED_COUNT           8

// BadUSB 
#define USB_as_HID 1

#endif /* Pins_Arduino_h */

