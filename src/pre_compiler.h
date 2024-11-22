#ifndef REDRAW_DELAY
#define REDRAW_DELAY 250
#endif

#if !defined(PART_04MB) && !defined(PART_08MB) && !defined(PART_16MB) && !defined(HEADLESS)
#define PART_04MB 1
#endif

#ifndef ROTATION
#define ROTATION 3
#endif

#ifndef HAS_BTN
    #define HAS_BTN 0
    #define SEL_BTN -1
    #define UP_BTN -1
    #define DW_BTN -1
    #define BTN_ACT LOW
    #define BTN_ALIAS '"Boot"'
#endif
#ifndef LED
  #define LED -1
#endif
#ifndef LED_ON
  #define LED_ON 1
#endif  

#ifndef	WIDTH
  #define WIDTH 240
#endif  
#ifndef	HEIGHT
  #define HEIGHT 135
#endif  

#ifndef FONT_P
  #define FONT_P 1
#endif  
#ifndef FONT_M
  #define FONT_M 2
#endif  
#ifndef FONT_G
  #define FONT_G 3
#endif 

#ifndef SDCARD_MOSI
  #define SDCARD_MOSI -1
#endif
#ifndef SDCARD_MISO 
  #define SDCARD_MISO  -1
#endif
#ifndef SDCARD_CS
  #define SDCARD_CS -1
#endif
#ifndef SDCARD_SCK
  #define SDCARD_SCK -1
#endif