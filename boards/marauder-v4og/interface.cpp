#include <interface.h>
#include "powerSave.h"

#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <soc/soc_caps.h>
#include <soc/adc_channel.h>
#include <CYD28_TouchscreenR.h>

CYD28_TouchR touch(320,240);

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

  touch.begin(&SPI);

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

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
  static long tm=millis();
  if(millis()-tm>200) { // don´t allow multiple readings in less than 200ms
    if(touch.touched()) { 
      auto t = touch.getPointScaled();
      t = touch.getPointScaled();
      tm=millis();
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
        else return;

        // Touch point global variable
        touchPoint.x = t.x;
        touchPoint.y = t.y;
        touchPoint.pressed=true;
        touchHeatMap(touchPoint);
    } else touchPoint.pressed=false;
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