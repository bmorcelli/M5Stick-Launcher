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

#ifndef	TFT_WIDTH
  #define TFT_WIDTH 135
#endif  
#ifndef	TFT_HEIGHT
  #define TFT_HEIGHT 240
#endif  

#ifndef FP
  #define FP 1
#endif  
#ifndef FM
  #define FM 2
#endif  
#ifndef FG
  #define FG 3
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
#ifndef LH
  #define LH 8
#endif
#ifndef LW
  #define LW 6
#endif
#ifndef TFT_MISO 
  #define TFT_MISO  -1
#endif