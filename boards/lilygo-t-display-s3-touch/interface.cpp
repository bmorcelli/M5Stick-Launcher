#include <interface.h>
#include "powerSave.h"
#include <SD_MMC.h>
#include <Wire.h>

#define TOUCH_MODULES_CST_SELF
#include <TouchLib.h>
#include <Wire.h>
#define LCD_MODULE_CMD_1

#include <esp_adc_cal.h>
TouchLib touch(Wire, 18, 17, CTS820_SLAVE_ADDRESS, 21);


/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() { 
    SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
    gpio_hold_dis((gpio_num_t)21);//PIN_TOUCH_RES 
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);//PIN_POWER_ON 
    pinMode(21, OUTPUT); //PIN_TOUCH_RES 
    digitalWrite(21, LOW);//PIN_TOUCH_RES 
    delay(500);
    digitalWrite(21, HIGH);//PIN_TOUCH_RES 
    Wire.begin(18, 17);//SDA, SCL
    if (!touch.init()) {
        Serial.println("Touch IC not found");
    }
    
    touch.setRotation(1);
    // PWM backlight setup
    ledcSetup(TFT_BRIGHT_CHANNEL,TFT_BRIGHT_FREQ, TFT_BRIGHT_Bits); //Channel 0, 10khz, 8bits
    ledcAttachPin(TFT_BL, TFT_BRIGHT_CHANNEL);
    ledcWrite(TFT_BRIGHT_CHANNEL,255);
}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   second stage gpio setup to make a few functions work
***************************************************************************************/
void _post_setup_gpio() { 
      // PWM backlight setup
    ledcSetup(TFT_BRIGHT_CHANNEL,TFT_BRIGHT_FREQ, TFT_BRIGHT_Bits); //Channel 0, 10khz, 8bits
    ledcAttachPin(TFT_BL, TFT_BRIGHT_CHANNEL);
    ledcWrite(TFT_BRIGHT_CHANNEL,255);
}

/***************************************************************************************
** Function name: getBattery()
** location: display.cpp
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() { 
  int percent=0;
  esp_adc_cal_characteristics_t adc_chars;

  // Get the internal calibration value of the chip
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  uint32_t raw = analogRead(BAT_PIN);
  uint32_t v1 = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2; //The partial pressure is one-half
  if(v1<4300) {
    float mv = v1 * 2;
    percent = (mv - 3300) * 100 / (float)(4150 - 3350);
  } else  { percent = 0; } 

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
  int dutyCycle;
  if (brightval==100) dutyCycle=255;
  else if (brightval==75) dutyCycle=130;
  else if (brightval==50) dutyCycle=70;
  else if (brightval==25) dutyCycle=20;
  else if (brightval==0) dutyCycle=5;
  else dutyCycle = ((brightval*255)/100);

  Serial.printf("dutyCycle for bright 0-255: %d",dutyCycle);
  ledcWrite(TFT_BRIGHT_CHANNEL,dutyCycle); // Channel 0
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
    static long tm=millis();
    if(millis()-tm>200) {
    if (touch.read()) { //touch.tirqTouched() &&
        auto t = touch.getPoint(0);
        if(rotation==1) {
            t.y = (tftHeight+20)-t.y;
            //t.x = tftWidth-t.x;
        }        
        if(rotation==3) {
            //t.y = (tftHeight+20)-t.y;
            t.x = tftWidth-t.x;
        }
        // Need to test the other orientations

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

        //Serial.printf("\nPressed x=%d , y=%d, rot: %d",t.x, t.y, rotation);

        if(!wakeUpScreen()) AnyKeyPress = true;
        else goto END;

        // Touch point global variable
        touchPoint.x = t.x;
        touchPoint.y = t.y;
        touchPoint.pressed=true;
        touchHeatMap(touchPoint);
        tm=millis();
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
void powerOff() { }


/*********************************************************************
** Function: checkReboot
** location: mykeyboard.cpp
** Btn logic to tornoff the device (name is odd btw)
**********************************************************************/
void checkReboot() { }
