#include <Arduino.h>
#if defined(CARDPUTER)
  #include <util/Cardputer/Keyboard.h>
  extern Keyboard_Class Keyboard;
#elif defined (STICK_C_PLUS) || defined (STICK_C) 
  #include <util/StickCP/AXP192.h>
  extern AXP192 axp192;
#elif defined(T_DISPLAY_S3)
  #define TOUCH_MODULES_CST_SELF
  #include <TouchLib.h>
  #include <Wire.h>
  #define LCD_MODULE_CMD_1
  extern TouchLib touch;
#endif

#if defined(CYD)
  //#include <XPT2046_Touchscreen.h>
  //extern XPT2046_Touchscreen touch;
  #include "CYD28_TouchscreenR.h"
  #define CYD28_DISPLAY_HOR_RES_MAX 320
  #define CYD28_DISPLAY_VER_RES_MAX 240  
  extern CYD28_TouchR touch;
#endif


String keyboard(String mytext, int maxSize = 76, String msg = "Type your message:");

bool checkNextPress();

bool checkPrevPress();

bool checkSelPress(bool dimmOff=false);

//It is here to not create a new set of files just to use AXP192 library again
int getBattery();

void resetDimmer();

bool menuPress(int bot);