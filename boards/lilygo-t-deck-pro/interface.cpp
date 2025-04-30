#include <interface.h>
#include <Wire.h>
#include "powerSave.h"
#include <TouchDrvCSTXXX.hpp>
TouchDrvCSTXXX touch;

#include <bq27220.h>
BQ27220 bq;

#include <XPowersLib.h>
XPowersPPM PPM;

#define BOARD_SDA       13
#define BOARD_SCL       14
#define TOUCH_INT    12
#define TOUCH_RST     45
#define BOARD_I2C_ADDR_TOUCH      0x1A 

#define BOARD_EPD_CS   34
#define BOARD_LORA_CS   3
#define BOARD_SD_CS   48
#define BOARD_GPS_EN  39  // enable GPS module
#define BOARD_1V8_EN  38  // enable gyroscope module
#define BOARD_6609_EN 41  // enable 7682 module
#define BOARD_LORA_EN 46  // enable LORA module
#define BOARD_MOTOR_PIN 2
#define BOARD_KEYBOARD_LED 42
#define BOARD_A7682E_PWRKEY 40

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() { 
    // LORA、SD、EPD use the same SPI, in order to avoid mutual influence;
    // before powering on, all CS signals should be pulled high and in an unselected state;
    pinMode(BOARD_EPD_CS, OUTPUT); 
    digitalWrite(BOARD_EPD_CS, HIGH);
    pinMode(BOARD_SD_CS, OUTPUT); 
    digitalWrite(BOARD_SD_CS, HIGH);
    pinMode(BOARD_LORA_CS, OUTPUT); 
    digitalWrite(BOARD_LORA_CS, HIGH);
    // Assuming that the previous touch was in sleep state, wake it up
    pinMode(TOUCH_INT, OUTPUT);
    digitalWrite(TOUCH_INT, HIGH);
    delay(1);
    digitalWrite(TOUCH_INT, LOW);

    pinMode(TOUCH_RST, OUTPUT);
    digitalWrite(TOUCH_RST, HIGH);
    delay(1);
    digitalWrite(TOUCH_RST, LOW);

    Serial.begin(115200);

    // IO
    pinMode(0, INPUT);
    pinMode(BOARD_KEYBOARD_LED, OUTPUT);
    pinMode(BOARD_MOTOR_PIN, OUTPUT);
    pinMode(BOARD_6609_EN, OUTPUT);         // enable 7682 module
    pinMode(BOARD_LORA_EN, OUTPUT);         // enable LORA module
    pinMode(BOARD_GPS_EN, OUTPUT);          // enable GPS module
    pinMode(BOARD_1V8_EN, OUTPUT);          // enable gyroscope module
    pinMode(BOARD_A7682E_PWRKEY, OUTPUT); 
    digitalWrite(BOARD_KEYBOARD_LED, LOW);
    digitalWrite(BOARD_MOTOR_PIN, LOW);
    digitalWrite(BOARD_6609_EN, HIGH);
    digitalWrite(BOARD_LORA_EN, HIGH);
    digitalWrite(BOARD_GPS_EN, HIGH);
    digitalWrite(BOARD_1V8_EN, HIGH);
    digitalWrite(BOARD_A7682E_PWRKEY, HIGH);

    SPI.begin(BOARD_SPI_SCK, SDCARD_MISO, BOARD_SPI_MOSI, BOARD_SPI_CS);

    Wire.begin(BOARD_SDA, BOARD_SCL);
    delay(500);

   // BQ25896 --- 0x6B
    Wire.beginTransmission(BQ25896_SLAVE_ADDRESS);
    if (Wire.endTransmission() == 0)
    {
        // battery_25896.begin();
        PPM.init(Wire, BOARD_SDA, BOARD_SCL, BQ25896_SLAVE_ADDRESS);
        // Set the minimum operating voltage. Below this voltage, the PPM will protect
        PPM.setSysPowerDownVoltage(3300);
        // Set input current limit, default is 500mA
        PPM.setInputCurrentLimit(3250);
        Serial.printf("getInputCurrentLimit: %d mA\n",PPM.getInputCurrentLimit());
        // Disable current limit pin
        PPM.disableCurrentLimitPin();
        // Set the charging target voltage, Range:3840 ~ 4608mV ,step:16 mV
        PPM.setChargeTargetVoltage(4208);
        // Set the precharge current , Range: 64mA ~ 1024mA ,step:64mA
        PPM.setPrechargeCurr(64);
        // The premise is that Limit Pin is disabled, or it will only follow the maximum charging current set by Limi tPin.
        // Set the charging current , Range:0~5056mA ,step:64mA
        PPM.setChargerConstantCurr(832);
        // Get the set charging current
        PPM.getChargerConstantCurr();
        Serial.printf("getChargerConstantCurr: %d mA\n",PPM.getChargerConstantCurr());
        PPM.enableADCMeasure();
        PPM.enableCharge();
        PPM.disableOTG();
    }
}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   second stage gpio setup to make a few functions work
***************************************************************************************/
	#define TFT_BRIGHT_CHANNEL 0
	#define TFT_BRIGHT_Bits 8
	#define TFT_BRIGHT_FREQ 5000	
    #define TFT_BL 40

void scanDevices(void){
    byte error, address;
    int nDevices = 0;
    Serial.println("Scanning for I2C devices ...");
    for (address = 0x01; address < 0x7f; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0) {
            Serial.printf("I2C device found at address 0x%02X\n", address);
            nDevices++;
        } else if (error != 2) {
            Serial.printf("Error %d at address 0x%02X\n", error, address);
        }
    }
    if (nDevices == 0) {
        Serial.println("No I2C devices found");
    }
}
void _post_setup_gpio() { 
        /*
    * The touch reset pin uses hardware pull-up,
    * and the function of setting the I2C device address cannot be used.
    * Use scanning to obtain the touch device address.*/


    // Scan I2C devices
    Serial.println("Scanning for I2C devices ...");
    scanDevices();
    uint8_t address = 0xFF;
    Wire.beginTransmission(CST328_SLAVE_ADDRESS);
    if (Wire.endTransmission() == 0) {
        address = CST328_SLAVE_ADDRESS;
    }

   uint8_t touchAddress = 0;
   touch.setPins(TOUCH_RST, TOUCH_INT);
   bool hasTouch=true;
   hasTouch = touch.begin(Wire, address, BOARD_SDA, BOARD_SCL);
   if (!hasTouch) {
       Serial.println("Failed to find Capacitive Touch !");
   } else {
       Serial.println("Find Capacitive Touch");
   }
   Serial.print("Model :"); Serial.println(touch.getModelName());
   //touch.setMaxCoordinates(TFT_WIDTH, TFT_HEIGHT);
   //touch.setSwapXY(true);
   //touch.setMirrorXY(false, true);
    // Brightness control must be initialized after tft in this case @Pirata
    //pinMode(TFT_BL,OUTPUT);
    //ledcSetup(TFT_BRIGHT_CHANNEL,TFT_BRIGHT_FREQ, TFT_BRIGHT_Bits); //Channel 0, 10khz, 8bits
    //ledcAttachPin(TFT_BL, TFT_BRIGHT_CHANNEL);
    //ledcWrite(TFT_BRIGHT_CHANNEL,125);
}


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
void _setBrightness(uint8_t brightval) {
    int dutyCycle;
    if (brightval==100) dutyCycle=255;
    else if (brightval==75) dutyCycle=130;
    else if (brightval==50) dutyCycle=70;
    else if (brightval==25) dutyCycle=20;
    else if (brightval==0) dutyCycle=0;
    else dutyCycle = ((brightval*255)/100);

    log_i("dutyCycle for bright 0-255: %d",dutyCycle);
    //ledcWrite(TFT_BRIGHT_CHANNEL,dutyCycle); // Channel 0
}

struct TouchPointPro {
  int16_t x=0;
  int16_t y=0;
};

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
    static long _tmptmp;
    TouchPointPro t;
    uint8_t touched = 0;
    touched = touch.getPoint(&t.x, &t.y);
    if((millis()-_tmptmp)>150 || LongPress) { // one reading each 500ms
        if(digitalRead(0)==LOW) NextPress=true;
        
        //Serial.printf("\nPressed x=%d , y=%d, rot: %d",t.x, t.y, rotation);
        if (touched) {

            Serial.printf("\nPressed x=%d , y=%d, rot: %d, millis=%d, tmp=%d",t.x, t.y, rotation, millis(), _tmptmp);
            _tmptmp=millis();

            // if(!wakeUpScreen()) AnyKeyPress = true;
            // else goto END;

            // Touch point global variable
            touchPoint.x = t.x;
            touchPoint.y = t.y;
            touchPoint.pressed=true;
            touchHeatMap(touchPoint);
            touched=0;
        }
        END:
        yield();
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
