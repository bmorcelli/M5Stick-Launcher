#include <interface.h>
#include "powerSave.h"
#include <LilyGo_AMOLED.h>

LilyGo_Class amoled;

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() { 
}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   second stage gpio setup to make a few functions work
***************************************************************************************/
void _post_setup_gpio() { 
    amoled.begin();
    // Only 1.91 Inch AMOLED board support
    amoled.setHomeButtonCallback([](void *ptr) {
        Serial.println("Home key pressed!");
        static uint32_t checkMs = 0;
        if (millis() > checkMs) {
            EscPress=true;
            AnyKeyPress=true;
        }
        checkMs = millis() + 200;
    }, NULL);
}


/***************************************************************************************
** Function name: getBattery()
** location: display.cpp
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() { 
  int percent=0;
  percent=amoled.BQ.getNTCPercentage();

  return  (percent < 0) ? 0
        : (percent >= 100) ? 100
        :  percent;
}


/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) { 
    //amoled.setBrightness(brightval*254/100);
}

struct TouchPointPro {
  int16_t x[5];
  int16_t y[5];
};
/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
    static long tm=0;
    TouchPointPro t;
    if(millis() - tm>200 || LongPress) {
    if (amoled.getPoint(t.x, t.y)) {
        tm=millis();
        if(rotation==1) {
            t.y[0] = TFT_WIDTH-t.y[0];
        }
        if(rotation==3) {
            t.x[0] = TFT_HEIGHT-t.x[0];
        }
        // Need to test these 2
        if(rotation==0) {
            int tmp=t.x[0];
            t.x[0]=t.y[0];
            t.y[0]=tmp;
        }
        if(rotation==2) {
            int tmp=t.x[0];
            t.x[0]=TFT_WIDTH-t.y[0];
            t.y[0]=TFT_HEIGHT-tmp;
        }

        Serial.printf("\nPressed x=%d , y=%d, rot: %d",t.x[0], t.y[0], rotation);

        if(!wakeUpScreen()) AnyKeyPress = true;
        else return;

        // Touch point global variable
        touchPoint.x = t.x[0];
        touchPoint.y = t.y[0];
        touchPoint.pressed=true;
        touchHeatMap(touchPoint);
    }
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
