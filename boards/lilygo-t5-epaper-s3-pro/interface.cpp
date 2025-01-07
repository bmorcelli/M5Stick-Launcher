#include <interface.h>
#include <Wire.h>
#include "powerSave.h"
// #include "touch.h"
// TouchClass touch;
#include <TouchDrvGT911.hpp>

#include "epd_driver.h"
#include "utilities.h"
TouchDrvGT911 touch(Wire,6,5,0x5d);

#include <bq27220.h>
BQ27220 bq;

#define BOARD_I2C_SDA       6
#define BOARD_I2C_SCL       5
#define BOARD_SENSOR_IRQ    15
#define BOARD_TOUCH_RST     41

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() { 
    gpio_hold_dis((gpio_num_t)BOARD_TOUCH_RST);//PIN_TOUCH_RES 
    pinMode(SEL_BTN, INPUT);
    //pinMode(UP_BTN, INPUT);
    pinMode(DW_BTN, INPUT);

    // CS pins of SPI devices to HIGH
    pinMode(46, OUTPUT); // LORA module
    digitalWrite(46,HIGH);


    pinMode(BOARD_TOUCH_RST, OUTPUT); //PIN_TOUCH_RES 
    digitalWrite(BOARD_TOUCH_RST, LOW);//PIN_TOUCH_RES 
    delay(500);
    digitalWrite(BOARD_TOUCH_RST, HIGH);//PIN_TOUCH_RES 
    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);//SDA, SCL

// Assuming that the previous touch was in sleep state, wake it up
    pinMode(TOUCH_INT, OUTPUT);
    digitalWrite(TOUCH_INT, HIGH);

/*
    * The touch reset pin uses hardware pull-up,
    * and the function of setting the I2C device address cannot be used.
    * Use scanning to obtain the touch device address.*/

   /*
   I2C device found at address 0x51
I2C device found at address 0x55
I2C device found at address 0x5d
I2C device found at address 0x6b
*/
    delay(10000);
    byte error, address;
    int nDevices = 0;
    Wire.begin(BOARD_SDA, BOARD_SCL);
    delay(2);
    for(address = 0x01; address < 0x7F; address++){
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if(error == 0){ // 0: success.
            nDevices++;
            Serial.printf("I2C device found at address 0x%x\n", address);
        }
        delay(2);
    }
    touch.begin(Wire,6,5,0x5d);
    //Set to trigger on falling edge
    touch.setInterruptMode(FALLING);

}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   second stage gpio setup to make a few functions work
***************************************************************************************/
void _post_setup_gpio() { }


/***************************************************************************************
** Function name: getBattery()
** location: display.cpp
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() { 
  int percent=0;
  percent=bq.getChargePcnt();

  return  (percent < 0) ? 0
        : (percent >= 100) ? 100
        :  percent;
}


/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) { }

struct TouchPointPro {
  int16_t x[5];
  int16_t y[5];
};
/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
    TouchPointPro t;
    uint8_t index=0;
     uint8_t touched = touch.getPoint(t.x, t.y, touch.getSupportTouchPoint());
    Serial.printf("\nPressed x=%d , y=%d, rot: %d",t.x, t.y, rotation);
    delay(200);
    if (touched) {
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

        Serial.printf("\nPressed x=%d , y=%d, rot: %d",t.x, t.y, rotation);
        

        if(!wakeUpScreen()) AnyKeyPress = true;
        else goto END;

        // Touch point global variable
        touchPoint.x = t.x[0];
        touchPoint.y = t.y[0];
        touchPoint.pressed=true;
        touchHeatMap(touchPoint);
    }
    END:
    yield();
//     if(AnyKeyPress) {
//       long tmp=millis();
//       touch.getPoint(t.x, t.y, index);
//       while((millis()-tmp)<200 && touch.data[0].state==0x07) { touch.getPoint(t.x, t.y, index); delay(50);}
//     }
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
