#include "interface.h"
#include "powerSave.h"
#include <SD_MMC.h>
#include <Wire.h>
#include <XPowersLib.h>
static PowersSY6970 PMU;
#define TOUCH_MODULES_CST_SELF
#include <TouchDrvCSTXXX.hpp>
#include <Wire.h>
#define LCD_MODULE_CMD_1

#include <esp_adc_cal.h>

#define BOARD_I2C_SDA       5
#define BOARD_I2C_SCL       6
#define BOARD_SENSOR_IRQ    21
#define BOARD_TOUCH_RST     13
TouchDrvCSTXXX  touch;

void touchHomeKeyCallback(void *user_data)
{
    Serial.println("Home key pressed!");
    static uint32_t checkMs = 0;
    if (millis() > checkMs) {
        if (digitalRead(TFT_BL)) {
            digitalWrite(TFT_BL, LOW);
        } else {
            digitalWrite(TFT_BL, HIGH);
        }
    }
    checkMs = millis() + 200;
}

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() { 
    gpio_hold_dis((gpio_num_t)BOARD_TOUCH_RST);//PIN_TOUCH_RES 
    pinMode(SEL_BTN, INPUT);
    pinMode(UP_BTN, INPUT);
    pinMode(DW_BTN, INPUT);

    // CS pins of SPI devices to HIGH
    pinMode(15, OUTPUT);
    digitalWrite(15,HIGH);
    pinMode(9, OUTPUT);
    digitalWrite(9,HIGH);
    pinMode(6, OUTPUT);
    digitalWrite(6,HIGH);


    pinMode(BOARD_TOUCH_RST, OUTPUT); //PIN_TOUCH_RES 
    digitalWrite(BOARD_TOUCH_RST, LOW);//PIN_TOUCH_RES 
    delay(500);
    digitalWrite(BOARD_TOUCH_RST, HIGH);//PIN_TOUCH_RES 
    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);//SDA, SCL

// Initialize capacitive touch
    touch.setPins(BOARD_TOUCH_RST, BOARD_SENSOR_IRQ);
    touch.begin(Wire, CST226SE_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL);
    touch.setMaxCoordinates(TFT_HEIGHT, TFT_WIDTH);
    touch.setSwapXY(true);
    touch.setMirrorXY(false, false);
    //Set the screen to turn on or off after pressing the screen Home touch button
    touch.setHomeButtonCallback(touchHomeKeyCallback);
    

    bool hasPMU =  PMU.init(Wire, BOARD_I2C_SDA, BOARD_I2C_SCL, SY6970_SLAVE_ADDRESS);
    if (!hasPMU) {
        Serial.println("PMU is not online...");
    } else {
        PMU.disableOTG();
        PMU.enableADCMeasure();
        PMU.enableCharge();
    }

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
  percent=(PMU.getSystemVoltage()-3300)*100/(float)(4150-3350);

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


struct box_t
{
  int x;
  int y;
  int w;
  int h;
  std::uint16_t color;
  int touch_id = -1;
  char key;
  char key_sh;

  void clear(void)
  {
    for (int i = 0; i < 8; ++i)
    {
      tft.fillRect(x, y, w, h,_BGCOLOR);
    }
  }
  void draw(void)
  {
    int ie = touch_id < 0 ? 4 : 8;
    for (int i = 0; i < ie; ++i)
    {
      tft.drawRect(x, y, w, h,color);
      tft.setTextColor(color);
      tft.drawChar(key,x+w/2-FONT_M*LW/2,y+h/2-FONT_M*LH/2);
    }
  }
  bool contain(int x, int y)
  {
    return this->x <= x && x < (this->x + this->w)
        && this->y <= y && y < (this->y + this->h);
  }
};

static constexpr std::size_t box_count = 52;
static box_t box_list[box_count];

struct TouchPoint {
  int16_t x[5];
  int16_t y[5];
};

bool menuPress(int bot) {
  //0 - prev
  //1 - Sel
  //2 - next
  //3 - all
  int terco=WIDTH/3;
  TouchPoint t;
  if (touch.getPoint(t.x, t.y, touch.getSupportTouchPoint()) && touch.isPressed()) {
    if(rotation==3) t.x[0] = WIDTH-t.x[0];
    else if (rotation==1) t.y[0] = (HEIGHT+20)-t.y[0];

    if(t.y[0]>(HEIGHT) && ((t.x[0]>terco*bot && t.x[0]<terco*(1+bot)) || bot==3)) { 
      t.x[0]=WIDTH+1;
      t.y[0]=HEIGHT+11;
      return true;
    } else return false;
  } else return false;
}

/*********************************************************************
** Function: checkNextPress
** location: mykeyboard.cpp
** Verifies Upper Btn to go to previous item
**********************************************************************/
bool checkNextPress(){ 
  if(digitalRead(UP_BTN)==BTN_ACT || menuPress(2)) 
  {
    if(wakeUpScreen()){
      delay(200);
      return false;
    }
    return true;
  }

  else return false;
}


/*********************************************************************
** Function: checkPrevPress
** location: mykeyboard.cpp
** Verifies Down Btn to go to next item
**********************************************************************/
bool checkPrevPress() {
  if(digitalRead(DW_BTN)==BTN_ACT || menuPress(0))
  {
    if(wakeUpScreen()){
      delay(200);
      return false;
    }
    return true;
  }

  else return false;
}


/*********************************************************************
** Function: checkSelPress
** location: mykeyboard.cpp
** Verifies if Select or OK was pressed
**********************************************************************/
bool checkSelPress(){
  checkPowerSaveTime();
  if(digitalRead(SEL_BTN)==BTN_ACT || menuPress(1)) 
  {
    if(wakeUpScreen()){
      delay(200);
      return false;
    }
    return true;
  }
  else return false;
}


/*********************************************************************
** Function: checkEscPress
** location: mykeyboard.cpp
** Verifies if Escape btn was pressed
**********************************************************************/
bool checkEscPress(){
    if(touch.getSupportTouchPoint() && touch.isPressed())
    {
        if(wakeUpScreen()){
          delay(200);
          return false;
        }
        return true;
    }
    else { return false; }
}


/*********************************************************************
** Function: checkAnyKeyPress
** location: mykeyboard.cpp
** Verifies id any of the keys was pressed
**********************************************************************/
bool checkAnyKeyPress() {
    if(menuPress(3) || digitalRead(UP_BTN)==BTN_ACT || digitalRead(DW_BTN)==BTN_ACT || digitalRead(SEL_BTN)==BTN_ACT) return true;

    return false;
}

/*********************************************************************
** Function: keyboard
** location: mykeyboard.cpp
** Starts keyboard to type data
**********************************************************************/
String keyboard(String mytext, int maxSize, String msg) {

  resetTftDisplay();
  bool caps=false;
  int x=0;
  int y=-1;
  int x2=0;
  int y2=0;
  char keys[4][12][2] = { //4 lines, with 12 characteres, low and high caps
    { 
      { '1', '!' },//1
      { '2', '@' },//2
      { '3', '#' },//3
      { '4', '$' },//4
      { '5', '%' },//5
      { '6', '^' },//6
      { '7', '&' },//7
      { '8', '*' },//8
      { '9', '(' },//9
      { '0', ')' },//10
      { '-', '_' },//11
      { '=', '+' } //12 
     }, 
    { 
      { 'q', 'Q' },//1
      { 'w', 'W' },//2
      { 'e', 'E' },//3
      { 'r', 'R' },//4
      { 't', 'T' },//5
      { 'y', 'Y' },//6
      { 'u', 'U' },//7
      { 'i', 'I' },//8
      { 'o', 'O' },//9
      { 'p', 'P' },//10
      { '[', '{' },//11
      { ']', '}' } //12
    },
    { 
      { 'a', 'A' },//1
      { 's', 'S' },//2
      { 'd', 'D' },//3
      { 'f', 'F' },//4
      { 'g', 'G' },//5
      { 'h', 'H' },//6
      { 'j', 'J' },//7
      { 'k', 'K' },//8
      { 'l', 'L' },//9
      { ';', ':' },//10
      { '"', '\'' },//11
      { '|', '\\' } //12
    },
    { 
      { '\\', '|' },//1
      { 'z', 'Z' },//2
      { 'x', 'X' },//3
      { 'c', 'C' },//4
      { 'v', 'V' },//5
      { 'b', 'B' },//6
      { 'n', 'N' },//7
      { 'm', 'M' },//8
      { ',', '<' },//9
      { '.', '>' },//10
      { '?', '/' },//11
      { '/', '/' } //12 
    }
  };
  int _x = WIDTH/12;
  int _y = (HEIGHT - 54)/4;
  int _xo = _x/2-3;

  int k=0;
  for(x2=0; x2<12;x2++) {
    for(y2=0; y2<4; y2++) {
      box_list[k].key=keys[y2][x2][0];
      box_list[k].key_sh=keys[y2][x2][1];
      box_list[k].color = ~_BGCOLOR;
      box_list[k].x=x2*_x;
      box_list[k].y=y2*_y+54;
      box_list[k].w=_x;
      box_list[k].h=_y;
      k++;
    }
  }
  // OK
  box_list[k].key=' ';
  box_list[k].key_sh=' ';
  box_list[k].color = ~_BGCOLOR;
  box_list[k].x=0;
  box_list[k].y=0;
  box_list[k].w=53;
  box_list[k].h=22;
  k++;
  // CAP
  box_list[k].key=' ';
  box_list[k].key_sh=' ';
  box_list[k].color = ~_BGCOLOR;
  box_list[k].x=55;
  box_list[k].y=0;
  box_list[k].w=50;
  box_list[k].h=22;
  k++;
  // DEL
  box_list[k].key=' ';
  box_list[k].key_sh=' ';
  box_list[k].color = ~_BGCOLOR;
  box_list[k].x=107;
  box_list[k].y=0;
  box_list[k].w=50;
  box_list[k].h=22;
  k++;
  // SPACE
  box_list[k].key=' ';
  box_list[k].key_sh=' ';
  box_list[k].color = ~_BGCOLOR;
  box_list[k].x=159;
  box_list[k].y=0;
  box_list[k].w=WIDTH-164;
  box_list[k].h=22;

  k=0;
  x2=0;
  y2=0;


  int i=0;
  int j=-1;
  bool redraw=true;
  delay(200);
  int cX =0;
  int cY =0;
  tft.fillScreen(_BGCOLOR);
  TouchPoint t;
  while(1) {
    if(redraw) {
      tft.setCursor(0,0);
      tft.setTextColor(~_BGCOLOR, _BGCOLOR);
      tft.setTextSize(FM);

      //Draw the rectangles
      if(y<0 || y2<0) {
        tft.fillRect(0,1,WIDTH,22,_BGCOLOR);
        tft.drawRect(7,2,46,20,~_BGCOLOR);       // Ok Rectangle
        tft.drawRect(55,2,50,20,~_BGCOLOR);      // CAP Rectangle
        tft.drawRect(107,2,50,20,~_BGCOLOR);     // DEL Rectangle
        tft.drawRect(159,2,74,20,~_BGCOLOR);     // SPACE Rectangle
        tft.drawRect(3,32,WIDTH-3,20,_FGCOLOR); // mystring Rectangle


        if(x==0 && y==-1) { tft.setTextColor(_BGCOLOR, ~_BGCOLOR); tft.fillRect(7,2,50,20,~_BGCOLOR); }
        else tft.setTextColor(~_BGCOLOR, _BGCOLOR);
        tft.drawString("OK", 18, 4);

        
        if(x==1 && y==-1) { tft.setTextColor(_BGCOLOR, ~_BGCOLOR); tft.fillRect(55,2,50,20,~_BGCOLOR); }
        else if(caps) { tft.fillRect(55,2,50,20,TFT_DARKGREY); tft.setTextColor(~_BGCOLOR, TFT_DARKGREY); }
        else tft.setTextColor(~_BGCOLOR, _BGCOLOR);
        tft.drawString("CAP", 64, 4);
      

        if(x==2 && y==-1) { tft.setTextColor(_BGCOLOR, ~_BGCOLOR); tft.fillRect(107,2,50,20,~_BGCOLOR); }
        else tft.setTextColor(~_BGCOLOR, _BGCOLOR);
        tft.drawString("DEL", 115, 4);

        if(x>2 && y==-1) { tft.setTextColor(_BGCOLOR, ~_BGCOLOR); tft.fillRect(159,2,74,20,~_BGCOLOR); }
        else tft.setTextColor(~_BGCOLOR, _BGCOLOR);
        tft.drawString("SPACE", 168, 4);
      }

      tft.setTextSize(FP);
      tft.setTextColor(~_BGCOLOR, 0x5AAB);
      tft.drawString(msg.substring(0,38), 3, 24);
      
      tft.setTextSize(FM);

      // reseta o quadrado do texto
      if (mytext.length() == 19 || mytext.length() == 20 || mytext.length() == 38 || mytext.length() == 39) tft.fillRect(3,32,WIDTH-3,20,_BGCOLOR); // mystring Rectangle
      // escreve o texto
      tft.setTextColor(~_BGCOLOR);    
      if(mytext.length()>19) { 
        tft.setTextSize(FP);
        if(mytext.length()>38) { 
          tft.drawString(mytext.substring(0,38), 5, 34);
          tft.drawString(mytext.substring(38,mytext.length()), 5, 42);
        } 
        else {
          tft.drawString(mytext, 5, 34);
        }
      } else {
        tft.drawString(mytext, 5, 34);
      }
      //desenha o retangulo colorido
      tft.drawRect(3,32,WIDTH-3,20,_FGCOLOR); // mystring Rectangle
      

      tft.setTextColor(~_BGCOLOR, _BGCOLOR);    
      tft.setTextSize(FM);
 
      for(i=0;i<4;i++) {
        for(j=0;j<12;j++) {
          //use last coordenate to paint only this letter
          if(x2==j && y2==i) { tft.setTextColor(~_BGCOLOR, _BGCOLOR); tft.fillRect(j*_x,i*_y+54,_x,_y,_BGCOLOR);}
          /* If selected, change font color and draw Rectangle*/
          if(x==j && y==i) { tft.setTextColor(_BGCOLOR, ~_BGCOLOR); tft.fillRect(j*_x,i*_y+54,_x,_y,~_BGCOLOR);}
          
                    
          /* Print the letters */
          if(!caps) tft.drawChar(keys[i][j][0], (j*_x+_xo), (i*_y+56));
          else tft.drawChar(keys[i][j][1], (j*_x+_xo), (i*_y+56));

          /* Return colors to normal to print the other letters */
          if(x==j && y==i) { tft.setTextColor(~_BGCOLOR, _BGCOLOR); }
        }
      }
      // save actual key coordenate
      x2=x;
      y2=y;
      redraw = false;
      previousMillis=millis(); // reset dimmer without delaying 200ms after each keypress

      touch.getPoint(t.x, t.y, touch.getSupportTouchPoint());
      
      TouchFooter();
    }



    //cursor handler
    if(mytext.length()>19) { 
      tft.setTextSize(FP);
      if(mytext.length()>38) { 
        cY=42;
        cX=5+(mytext.length()-38)*LW;
      } 
      else {
        cY=34;
        cX=5+mytext.length()*LW;
      }
    } else {
      cY=34;
      cX=5+mytext.length()*LW*2;
    }

    int z=0;

    if (touch.getPoint(t.x, t.y, touch.getSupportTouchPoint()) && touch.isPressed())
     {
        if(rotation==3) {
          //t.y = HEIGHT-t.y;
          t.x[0] = WIDTH-t.x[0];
        } else if (rotation==1) {
          t.y[0] = (HEIGHT+20)-t.y[0];
        }

      if (box_list[48].contain(t.x[0], t.y[0])) { break;      goto THIS_END; }      // Ok
      if (box_list[49].contain(t.x[0], t.y[0])) { caps=!caps; tft.fillRect(0,54,WIDTH,HEIGHT-54,_BGCOLOR); goto THIS_END; } // CAP
      if (box_list[50].contain(t.x[0], t.y[0])) goto DEL;               // DEL
      if (box_list[51].contain(t.x[0], t.y[0])) { mytext += box_list[51].key; goto ADD; } // SPACE
      for(k=0;k<48;k++){
        if (box_list[k].contain(t.x[0], t.y[0])) {
          if(caps) mytext += box_list[k].key_sh;
          else mytext += box_list[k].key;
        }
      }
      THIS_END:

      t.x[0]=WIDTH+1;
      t.y[0]=HEIGHT+11;
      delay(250);
      redraw=true;
    }
    if(checkSelPress())  { 
      tft.setCursor(cX,cY);
      if(caps) z=1;
      else z=0;
      if(x==0 && y==-1) break;
      else if(x==1 && y==-1) { caps=!caps; tft.fillRect(0,54,WIDTH,HEIGHT-54,_BGCOLOR); }
      else if(x==2 && y==-1 && mytext.length() > 0) {
        DEL:
        mytext.remove(mytext.length()-1);
        int fS=FM;
        if(mytext.length()>19) { tft.setTextSize(FP); fS=FP; }
        else tft.setTextSize(FM);
        tft.setCursor((cX-fS*LW),cY);
        tft.setTextColor(_FGCOLOR,_BGCOLOR);
        tft.print(" "); 
        tft.setTextColor(~_BGCOLOR, 0x5AAB);
        tft.setCursor(cX-fS*LW,cY);
        cX=tft.getCursorX();
        cY=tft.getCursorY();         
      }
      else if(x>2 && y==-1 && mytext.length()<maxSize) mytext += " ";
      else if(y>-1 && mytext.length()<maxSize) {
        mytext += keys[y][x][z];
        ADD:
        if(mytext.length()!=20 && mytext.length()!=20) tft.print(keys[y][x][z]);
        cX=tft.getCursorX();
        cY=tft.getCursorY();
      } 
      delay(250);
      redraw = true;
    }

    /* Down Btn to move in X axis (to the right) */  
    if(checkNextPress()) 
    { 
      delay(250);
      if(checkNextPress()) { x--; delay(250); } // Long Press
      else x++; // Short Press

      if(y<0 && x>3) x=0;
      if(x>11) x=0;
      else if (x<0) x=11;
      redraw = true;
    }
    /* UP Btn to move in Y axis (Downwards) */
    if(checkPrevPress()) { 
      delay(250);
      if(checkPrevPress()) { y--; delay(250);  }// Long press
      else y++; // short press
      if(y>3) { y=-1; }
      else if(y<-1) y=3;
      redraw = true;
    }
  }
  
  //Resets screen when finished writing
  tft.fillRect(0,0,tft.width(),tft.height(),_BGCOLOR);
  resetTftDisplay();

  return mytext;
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


/*********************************************************************
** Function: _checkKeyPress
** location: mykeyboard.cpp
** returns the key from the keyboard
**********************************************************************/
keyStroke _getKeyPress() { 
    keyStroke key;
    return key;
 } // must return something that the keyboards won´t recognize by default

/*********************************************************************
** Function: _checkNextPagePress
** location: mykeyboard.cpp
** returns the key from the keyboard
**********************************************************************/
bool _checkNextPagePress() { return false; }

/*********************************************************************
** Function: _checkPrevPagePress
** location: mykeyboard.cpp
** returns the key from the keyboard
**********************************************************************/
bool _checkPrevPagePress() { return false; }