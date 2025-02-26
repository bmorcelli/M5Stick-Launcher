#include <interface.h>
#include "powerSave.h"

#ifndef TFT_BRIGHT_CHANNEL
    #define TFT_BRIGHT_CHANNEL 0
    #define TFT_BRIGHT_FREQ 5000
    #define TFT_BRIGHT_Bits 8
    #define TFT_BL GPIO_BCKL
#endif

#if defined(HAS_CAPACITIVE_TOUCH)
    #include "CYD28_TouchscreenC.h"
    #define CYD28_DISPLAY_HOR_RES_MAX 240
    #define CYD28_DISPLAY_VER_RES_MAX 320
    CYD28_TouchC touch(CYD28_DISPLAY_HOR_RES_MAX, CYD28_DISPLAY_VER_RES_MAX);
#elif defined(TOUCH_GT911_I2C) || defined(TOUCH_AXS15231B_I2C) || defined(TOUCH_CST816S_I2C)
    #include <bb_captouch.h>
    #ifdef TOUCH_GT911_I2C
        #define TOUCH_SDA_PIN GT911_I2C_CONFIG_SDA_IO_NUM
        #define TOUCH_SCL_PIN GT911_I2C_CONFIG_SCL_IO_NUM
    #elif TOUCH_CST816S_I2C
        #define TOUCH_SDA_PIN CST816S_I2C_CONFIG_SDA_IO_NUM
        #define TOUCH_SCL_PIN CST816S_I2C_CONFIG_SCL_IO_NUM
    #elif TOUCH_AXS15231B_I2C
        #define TOUCH_SDA_PIN 41
        #define TOUCH_SCL_PIN 42
    #endif

    class CYD_Touch: public BBCapTouch {
        public:
        TouchPoint t;
        TOUCHINFO ti;
        CYD_Touch() { BBCapTouch(); }
        inline bool begin() { 
            bool result = init(TOUCH_SDA_PIN, TOUCH_SCL_PIN); // returns 0 if CT_SUCCESS;
            setOrientation(90, 320,240); // This orientation reflects the right position for the InputHandler logic.
            return result==0? true:false;
        }
        inline bool touched() { if(getSamples(&ti)) { t.x = ti.x[0]; t.y = ti.y[0]; t.pressed = true; } else { t.x=0; t.y=0; t.pressed=false; } }
        inline TouchPoint getPointScaled() { return t; }

    };
    CYD_Touch touch;
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
    pinMode(33, OUTPUT);
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