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
    pinMode(UP_BTN, INPUT);   // Sets the power btn as an INPUT
    pinMode(SEL_BTN, INPUT);
    pinMode(DW_BTN, INPUT);
    pinMode(R_BTN, INPUT);
    pinMode(L_BTN, INPUT);

}


/***************************************************************************************
** Function name: getBattery()
** location: display.cpp
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() { 
    return 0;
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
    
    if(digitalRead(SEL_BTN)==BTN_ACT || digitalRead(UP_BTN)==BTN_ACT || digitalRead(DW_BTN)==BTN_ACT || digitalRead(R_BTN)==BTN_ACT || digitalRead(L_BTN)==BTN_ACT) {
        if(!wakeUpScreen()) AnyKeyPress = true;
        else goto END;
    }    
    if(digitalRead(L_BTN)==BTN_ACT) {
        PrevPress = true;
    }
    if(digitalRead(R_BTN)==BTN_ACT) {
        NextPress = true;
    }
    if(digitalRead(UP_BTN)==BTN_ACT) {
        UpPress = true;
    }
    if(digitalRead(DW_BTN)==BTN_ACT) {
        DownPress = true;
    }
    if(digitalRead(SEL_BTN)==BTN_ACT) {
        SelPress = true;
    }
    END:
    if(AnyKeyPress) {
      long tmp=millis();
      while((millis()-tmp)<200 && (digitalRead(SEL_BTN)==BTN_ACT || digitalRead(UP_BTN)==BTN_ACT || digitalRead(DW_BTN)==BTN_ACT || digitalRead(R_BTN)==BTN_ACT || digitalRead(L_BTN)==BTN_ACT));
    }
}


/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) { 
  if(brightval>100) brightval=100;
   if(brightval == 0){
      analogWrite(BACKLIGHT, brightval);
    } else {
      int bl = MINBRIGHT + round(((255 - MINBRIGHT) * brightval/100 ));
      analogWrite(BACKLIGHT, bl);
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