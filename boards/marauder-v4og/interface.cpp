#include <interface.h>
#include "powerSave.h"

#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <soc/soc_caps.h>
#include <soc/adc_channel.h>

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() { 
  pinMode(TFT_BL,OUTPUT);
  ledcSetup(TFT_BRIGHT_CHANNEL,TFT_BRIGHT_FREQ, TFT_BRIGHT_Bits); //Channel 0, 10khz, 8bits
  ledcAttachPin(TFT_BL, TFT_BRIGHT_CHANNEL);
  ledcWrite(TFT_BRIGHT_CHANNEL,255);

  //uint16_t calData[5] = { 275, 3494, 361, 3528, 4 }; //org portrait
  uint16_t calData[5] = { 391, 3491, 266, 3505, 7 }; // Landscape TFT Shield from maruader
  //uint16_t calData[5] = { 213, 3469, 320, 3446, 1 }; // Landscape TFT DIY  from maruader

  tft->setTouch(calData);

}

/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) { 
  int dutyCycle;
  if (bright==100) dutyCycle=255;
  else if (bright==75) dutyCycle=130;
  else if (bright==50) dutyCycle=70;
  else if (bright==25) dutyCycle=20;
  else if (bright==0) dutyCycle=0;
  else dutyCycle = ((bright*255)/100);
  Serial.printf("dutyCycle for bright 0-255: %d",dutyCycle);
  ledcWrite(TFT_BRIGHT_CHANNEL,dutyCycle); // Channel 0
}

bool menuPress(int bot) {
  //0 - prev
  //1 - Sel
  //2 - next
  //3 - all
  TouchPoint t;
  bool touched = tft->getTouch(&t.x, &t.y, 600);

  int terco=tftWidth/3;
  if(touched) { 
    log_i("Touchscreen Pressed at x=%d, y=%d", t.x,t.y);
    if(rotation==3) { 
      t.y = (tftHeight+20)-t.y;
      t.x = tftWidth-t.x;
    }

    if(t.y>(tftHeight) && ((t.x>terco*bot && t.x<terco*(1+bot)) || bot==3)) { 
      t.x=tftWidth+1;
      t.y=tftHeight+11;
      return true;
    } else return false;
  } else return false;
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
    TouchPoint t;
    bool touched = tft->getTouch(&t.x, &t.y, 600);
    if (touched) {
        if(rotation==3) {
            t.y = (tftHeight+20)-t.y;
            t.x = tftWidth-t.x;
        }
        if(rotation==0) {
            int tmp=t.x;
            t.x = tftWidth-t.y;
            t.y = tmp;
        }
        if(rotation==2) {
            int tmp=t.x;
            t.x = t.y;
            t.y = (tftHeight+20)-tmp;
        }

        if(!wakeUpScreen()) AnyKeyPress = true;
        else goto END;

        // Touch point global variable
        touchPoint.x = t.x;
        touchPoint.y = t.y;
        touchPoint.pressed=true;
        touchHeatMap(touchPoint);
    }
    END:
    if(AnyKeyPress) {
      long tmp=millis();
      while((millis()-tmp)<200 && tft->getTouch(&t.x, &t.y, 600));
    }
}

/*********************************************************************
** Function: powerOff
** location: mykeyboard.cpp
** Turns off the device (or try to)
**********************************************************************/
void powerOff() { }


/*********************************************************************
** Function: checkReboot
** location: mykeyboard.cpp
** Btn logic to tornoff the device (name is odd btw)
**********************************************************************/
void checkReboot() { }