#include <interface.h>
#include "powerSave.h"


#if defined(HAS_CAPACITIVE_TOUCH)
    #include "CYD28_TouchscreenC.h"
    #define CYD28_DISPLAY_HOR_RES_MAX 240
    #define CYD28_DISPLAY_VER_RES_MAX 320
    CYD28_TouchC touch(CYD28_DISPLAY_HOR_RES_MAX, CYD28_DISPLAY_VER_RES_MAX);
#else
    #include "CYD28_TouchscreenR.h"
    #ifndef CYD28_DISPLAY_HOR_RES_MAX
    #define CYD28_DISPLAY_HOR_RES_MAX 320
    #endif

    #ifndef CYD28_DISPLAY_VER_RES_MAX
    #define CYD28_DISPLAY_VER_RES_MAX 240  
    #endif
    CYD28_TouchR touch(CYD28_DISPLAY_HOR_RES_MAX, CYD28_DISPLAY_VER_RES_MAX);
#endif

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() { 
#if !defined(HAS_CAPACITIVE_TOUCH)
    pinMode(XPT2046_CS, OUTPUT);
#endif

}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   second stage gpio setup to make a few functions work
***************************************************************************************/
void _post_setup_gpio() { 
    // Brightness control must be initialized after tft in this case @Pirata
    pinMode(TFT_BL,OUTPUT);
    ledcSetup(TFT_BRIGHT_CHANNEL,TFT_BRIGHT_FREQ, TFT_BRIGHT_Bits); //Channel 0, 10khz, 8bits
    ledcAttachPin(TFT_BL, TFT_BRIGHT_CHANNEL);
    ledcWrite(TFT_BRIGHT_CHANNEL,255);

    if(!touch.begin(
        #ifdef CYD28_TouchR_MOSI    
            #if TFT_MOSI==CYD28_TouchR_MOSI
            &SPI
            #endif
        #endif
    
        )) {
            Serial.println("Touch IC not Started");
            log_i("Touch IC not Started");
        } else Serial.println("Touch IC Started");
}

/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) { 
    int dutyCycle;
    if (brightval==100) dutyCycle=255;
    else if (brightval==75) dutyCycle=130;
    else if (brightval==50) dutyCycle=70;
    else if (brightval==25) dutyCycle=20;
    else if (brightval==0) dutyCycle=0;
    else dutyCycle = ((brightval*255)/100);

    log_i("dutyCycle for bright 0-255: %d",dutyCycle);
    ledcWrite(TFT_BRIGHT_CHANNEL,dutyCycle); // Channel 0
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
    static long d_tmp=0;
    if (millis()-d_tmp>200) { // I know R3CK.. I Should NOT nest if statements..
                            // but it is needed to not keep SPI bus used without need, it save resources
        TouchPoint t;
        #ifdef DONT_USE_INPUT_TASK
        checkPowerSaveTime();
        #endif
        if(touch.touched()) { 
            auto t = touch.getPointScaled();
            t = touch.getPointScaled();
          #ifdef DONT_USE_INPUT_TASK // need to reset the variables to avoid ghost click
            NextPress=false;
            PrevPress=false;
            UpPress=false;
            DownPress=false;
            SelPress=false;
            EscPress=false;
            AnyKeyPress=false;
            touchPoint.pressed=false;
          #endif

        #ifdef CYD28_TouchR_MOSI    
            #if TFT_MOSI==CYD28_TouchR_MOSI // S024R is inverted
                int tmp=t.x;
                t.x=t.y;
                t.y=tmp;
            #endif
        #endif
        
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
        //Serial.printf("\nTouch Pressed on x=%d, y=%d, rot=%d\n",t.x, t.y,rotation);

        if(!wakeUpScreen()) AnyKeyPress = true;
        else goto END;

        // Touch point global variable
        touchPoint.x = t.x;
        touchPoint.y = t.y;
        touchPoint.pressed=true;
        touchHeatMap(touchPoint);

        d_tmp=millis();
      }
    }
    END:
    delay(0);
}

/*********************************************************************
** Function: powerOff
** location: mykeyboard.cpp
** Turns off the device (or try to)
**********************************************************************/
void powerOff() { 
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0,LOW); 
    esp_deep_sleep_start();
}


/*********************************************************************
** Function: checkReboot
** location: mykeyboard.cpp
** Btn logic to tornoff the device (name is odd btw)
**********************************************************************/
void checkReboot() { }