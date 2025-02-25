#ifndef _CYD_PREDEFS
#define  _CYD_PREDEFS

#ifdef CYD // To automatize CYD defs from 
  #define DISABLE_OTA
  #define PART_04MB=1
  // #define PART_08MB =0
  // #define PART_16MB=0

  #define ROTATION   3
  #define HAS_BTN    0
  #define SEL_BTN    -1
  #define UP_BTN     -1
  #define DW_BTN     -1
  #define BTN_ACT    LOW
  #define BAT_PIN    4
  #define BTN_ALIAS  '"Sel"'
  #define MINBRIGHT  190
  #define BACKLIGHT  21
  #define LED        -1
  #define LED_ON     LOW
  #define HAS_TOUCH   1


  #define TFT_BRIGHT_CHANNEL  0
  #define TFT_BRIGHT_Bits     8
  #define TFT_BRIGHT_FREQ     5000	

  #ifdef BOARD_HAS_TF
    #define SDCARD_CS      TF_CS
    #define SDCARD_SCK     TF_SPI_SCLK
    #define SDCARD_MISO    TF_SPI_MISO
    #define SDCARD_MOSI    TF_SPI_MOSI
  #else
    #define SDCARD_CS      5
    #define SDCARD_SCK     18
    #define SDCARD_MISO    19
    #define SDCARD_MOSI    23
  #endif

  #ifdef defined(DISPLAY_ILI9341_SPI)
    #define ILI9341_DRIVER  1
    #define TFT_MISO    ILI9341_SPI_BUS_MISO_IO_NUM
    #define TFT_MOSI    ILI9341_SPI_BUS_MOSI_IO_NUM
    #define TFT_SCLK    ILI9341_SPI_BUS_SCLK_IO_NUM
    #define TFT_CS      ILI9341_SPI_CONFIG_CS_GPIO_NUM
    #define TFT_DC      ILI9341_SPI_CONFIG_DC_GPIO_NUM
    #define TFT_RST     ILI9341_DEV_CONFIG_RESET_GPIO_NUM
    #define TFT_BL      GPIO_BCKL
    #define TFT_WIDTH   DISPLAY_WIDTH
    #define TFT_HEIGH   DISPLAY_HEIGHT

    #define  GFX_BL     GPIO_BCKL
    #define  TFT_IPS        0
    #define  TFT_COL_OFS1   0
    #define  TFT_ROW_OFS1   0
    #define  TFT_COL_OFS2   0
    #define  TFT_ROW_OFS2   0

  #elif defined(DISPLAY_ST7789_I80)
    #define  LOVYAN_BUS     Bus_Parallel8 
    #define  USE_LOVYANGFX  1
    #define  LOVYAN_PANEL   Panel_ST7789
    #define  TFT_ROTATION   0 //# rotation offset
    #define  TFT_INVERSION_OFF
    #define  TFT_PARALLEL_8_BIT
    #define  TFT_WIDTH      DISPLAY_WIDTH
    #define  TFT_HEIGHT     DISPLAY_HEIGHT
    #define  TFT_CS         ST7789_IO_I80_CONFIG_CS_GPIO_NUM // Chip select control pin=library pulls permanently low
    #define  TFT_DC         ST7789_I80_BUS_CONFIG_DC //      ; Data Command control pin
    #define  TFT_RST        ST7789_DEV_CONFIG_RESET_GPIO_NUM //    ; Reset pin, toggles on startup
    #define  TFT_WR         ST7789_I80_BUS_CONFIG_WR  //   ; Write strobe control pin
    #define  TFT_RD         ST7789_RD_GPIO  //    ; Read strobe control pin
    #define  TFT_D0         ST7789_I80_BUS_CONFIG_DATA_GPIO_D8
    #define  TFT_D1         ST7789_I80_BUS_CONFIG_DATA_GPIO_D9
    #define  TFT_D2         ST7789_I80_BUS_CONFIG_DATA_GPIO_D10
    #define  TFT_D3         ST7789_I80_BUS_CONFIG_DATA_GPIO_D11
    #define  TFT_D4         ST7789_I80_BUS_CONFIG_DATA_GPIO_D12
    #define  TFT_D5         ST7789_I80_BUS_CONFIG_DATA_GPIO_D13
    #define  TFT_D6         ST7789_I80_BUS_CONFIG_DATA_GPIO_D14
    #define  TFT_D7         ST7789_I80_BUS_CONFIG_DATA_GPIO_D15
    #define  TFT_BCKL       GPIO_BCKL    // LED back-light
    #define  TFT_BL         GPIO_BCKL
    #define  TFT_BUS_SHARED 0
    #define  TFT_INVERTED   0

    #define  GFX_BL         GPIO_BCKL
    #define  TFT_IPS        0
    #define  TFT_COL_OFS1   0
    #define  TFT_ROW_OFS1   0

  #else 
    #define ILI9341_DRIVER  1
    #define TFT_MISO        12
    #define TFT_MOSI        13
    #define TFT_SCLK        14
    #define TFT_CS          15
    #define TFT_DC          2
    #define TFT_RST         -1
    #define TFT_BL          21
    #define TFT_WIDTH       240
    #define TFT_HEIGHT      320

    #define  GFX_BL         21
    #define  TFT_IPS        0
    #define  TFT_COL_OFS1   0
    #define  TFT_ROW_OFS1   0
    #define  TFT_COL_OFS2   0
    #define  TFT_ROW_OFS2   0
  #endif

  #ifdef TOUCH_XPT2046_SPI
    #define  CYD28_TouchR_IRQ   XPT2046_TOUCH_CONFIG_INT_GPIO_NUM
    #define  CYD28_TouchR_MISO  XPT2046_SPI_BUS_MISO_IO_NUM
    #define  CYD28_TouchR_MOSI  XPT2046_SPI_BUS_MOSI_IO_NUM
    #define  CYD28_TouchR_CSK   XPT2046_SPI_BUS_SCLK_IO_NUM
    #define  CYD28_TouchR_CS    XPT2046_SPI_CONFIG_CS_GPIO_NUM
  #endif

  #if defined(CYD28_TouchR_MOSI) && defined(CYD28_TouchR_MOSI)
    #if CYD28_TouchR_MOSI == CYD28_TouchR_MOSI
        #define  DONT_USE_INPUT_TASK    1
    #endif
  #endif

#endif

#endif