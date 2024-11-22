#include "interface.h"
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

  tft.setTouch(calData);

}


/***************************************************************************************
** Function name: getBattery()
** location: display.cpp
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() { 
    uint8_t percent=0;

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
  if (bright==100) dutyCycle=255;
  else if (bright==75) dutyCycle=130;
  else if (bright==50) dutyCycle=70;
  else if (bright==25) dutyCycle=20;
  else if (bright==0) dutyCycle=0;
  else dutyCycle = ((bright*255)/100);
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
      tft.fillRect(x, y, w, h,BGCOLOR);
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
  uint16_t x;
  uint16_t y;
};

bool menuPress(int bot) {
  //0 - prev
  //1 - Sel
  //2 - next
  //3 - all
  TouchPoint t;
  bool touched = tft.getTouch(&t.x, &t.y, 600);

  int terco=WIDTH/3;
  if(touched) { 
    log_i("Touchscreen Pressed at x=%d, y=%d", t.x,t.y);
    if(rotation==3) { 
      t.y = (HEIGHT+20)-t.y;
      t.x = WIDTH-t.x;
    }

    if(t.y>(HEIGHT) && ((t.x>terco*bot && t.x<terco*(1+bot)) || bot==3)) { 
      t.x=WIDTH+1;
      t.y=HEIGHT+11;
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
    if(menuPress(2)) {
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
    if(menuPress(0)) {
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
    if(menuPress(1)) {
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
    if(menuPress(0)) {
        if(wakeUpScreen()){
            delay(200);
            return false;
        }
        returnToMenu=true;
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
    if(menuPress(3)) return true;
    else return false;
}


/*********************************************************************
** Function: keyboard
** location: mykeyboard.cpp
** Starts keyboard to type data
**********************************************************************/
/* Starts keyboard to type data */
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
      box_list[k].color = ~BGCOLOR;
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
  box_list[k].color = ~BGCOLOR;
  box_list[k].x=0;
  box_list[k].y=0;
  box_list[k].w=53;
  box_list[k].h=22;
  k++;
  // CAP
  box_list[k].key=' ';
  box_list[k].key_sh=' ';
  box_list[k].color = ~BGCOLOR;
  box_list[k].x=55;
  box_list[k].y=0;
  box_list[k].w=50;
  box_list[k].h=22;
  k++;
  // DEL
  box_list[k].key=' ';
  box_list[k].key_sh=' ';
  box_list[k].color = ~BGCOLOR;
  box_list[k].x=107;
  box_list[k].y=0;
  box_list[k].w=50;
  box_list[k].h=22;
  k++;
  // SPACE
  box_list[k].key=' ';
  box_list[k].key_sh=' ';
  box_list[k].color = ~BGCOLOR;
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
  tft.fillScreen(BGCOLOR);
  while(1) {
    if(redraw) {
      tft.setCursor(0,0);
      tft.setTextColor(~BGCOLOR, BGCOLOR);
      tft.setTextSize(FM);

      //Draw the rectangles
      if(y<0) {
        tft.fillRect(0,1,WIDTH,22,BGCOLOR);
        tft.drawRect(7,2,46,20,~BGCOLOR);       // Ok Rectangle
        tft.drawRect(55,2,50,20,~BGCOLOR);      // CAP Rectangle
        tft.drawRect(107,2,50,20,~BGCOLOR);     // DEL Rectangle
        tft.drawRect(159,2,74,20,~BGCOLOR);     // SPACE Rectangle
        tft.drawRect(3,32,WIDTH-3,20,FGCOLOR); // mystring Rectangle


        if(x==0 && y==-1) { tft.setTextColor(BGCOLOR, ~BGCOLOR); tft.fillRect(7,2,50,20,~BGCOLOR); }
        else tft.setTextColor(~BGCOLOR, BGCOLOR);
        tft.drawString("OK", 18, 4);

        
        if(x==1 && y==-1) { tft.setTextColor(BGCOLOR, ~BGCOLOR); tft.fillRect(55,2,50,20,~BGCOLOR); }
        else if(caps) { tft.fillRect(55,2,50,20,TFT_DARKGREY); tft.setTextColor(~BGCOLOR, TFT_DARKGREY); }
        else tft.setTextColor(~BGCOLOR, BGCOLOR);
        tft.drawString("CAP", 64, 4);
      

        if(x==2 && y==-1) { tft.setTextColor(BGCOLOR, ~BGCOLOR); tft.fillRect(107,2,50,20,~BGCOLOR); }
        else tft.setTextColor(~BGCOLOR, BGCOLOR);
        tft.drawString("DEL", 115, 4);

        if(x>2 && y==-1) { tft.setTextColor(BGCOLOR, ~BGCOLOR); tft.fillRect(159,2,74,20,~BGCOLOR); }
        else tft.setTextColor(~BGCOLOR, BGCOLOR);
        tft.drawString("SPACE", 168, 4);
      }

      tft.setTextSize(FP);
      tft.setTextColor(~BGCOLOR, 0x5AAB);
      tft.drawString(msg.substring(0,38), 3, 24);
      
      tft.setTextSize(FM);

      // reseta o quadrado do texto
      if (mytext.length() == 19 || mytext.length() == 20 || mytext.length() == 38 || mytext.length() == 39) tft.fillRect(3,32,WIDTH-3,20,BGCOLOR); // mystring Rectangle
      // escreve o texto
      tft.setTextColor(~BGCOLOR);    
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
      tft.drawRect(3,32,WIDTH-3,20,FGCOLOR); // mystring Rectangle
      

      tft.setTextColor(~BGCOLOR, BGCOLOR);    
      tft.setTextSize(FM);
 
      for(i=0;i<4;i++) {
        for(j=0;j<12;j++) {
          //use last coordenate to paint only this letter
          if(x2==j && y2==i) { tft.setTextColor(~BGCOLOR, BGCOLOR); tft.fillRect(j*_x,i*_y+54,_x,_y,BGCOLOR);}
          /* If selected, change font color and draw Rectangle*/
          if(x==j && y==i) { tft.setTextColor(BGCOLOR, ~BGCOLOR); tft.fillRect(j*_x,i*_y+54,_x,_y,~BGCOLOR);}
          
                    
          /* Print the letters */
          if(!caps) tft.drawChar(keys[i][j][0], (j*_x+_xo), (i*_y+56));
          else tft.drawChar(keys[i][j][1], (j*_x+_xo), (i*_y+56));

          /* Return colors to normal to print the other letters */
          if(x==j && y==i) { tft.setTextColor(~BGCOLOR, BGCOLOR); }
        }
      }
      // save actual key coordenate
      x2=x;
      y2=y;
      redraw = false;
      previousMillis=millis(); // reset dimmer without delaying 200ms after each keypress
    }

    TouchFooter();

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
    TouchPoint t;
    bool touched = tft.getTouch(&t.x, &t.y, 600);

    if(rotation==3) { 
      t.y = (HEIGHT+20)-t.y;
      t.x = WIDTH-t.x;
    }
    if(touched)

     {
      if (box_list[48].contain(t.x, t.y)) { break;      goto THIS_END; }      // Ok
      if (box_list[49].contain(t.x, t.y)) { caps=!caps; tft.fillRect(0,54,WIDTH,HEIGHT-54,BGCOLOR); goto THIS_END; } // CAP
      if (box_list[50].contain(t.x, t.y)) goto DEL;               // DEL
      if (box_list[51].contain(t.x, t.y)) { mytext += box_list[51].key; goto ADD; } // SPACE
      for(k=0;k<48;k++){
        if (box_list[k].contain(t.x, t.y)) {
          if(caps) mytext += box_list[k].key_sh;
          else mytext += box_list[k].key;
        }
      }
      THIS_END: 
      redraw=true;
    }

  }
  
  //Resets screen when finished writing
  tft.fillRect(0,0,tft.width(),tft.height(),BGCOLOR);
  resetTftDisplay();

  return mytext;
}


/*********************************************************************
** Function: powerOff
** location: mykeyboard.cpp
** Turns off the device (or try to)
**********************************************************************/
void powerOff() {
    digitalWrite(4,LOW);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)UP_BTN,LOW); 
    esp_deep_sleep_start();
}


/*********************************************************************
** Function: checkReboot
** location: mykeyboard.cpp
** Btn logic to tornoff the device (name is odd btw)
**********************************************************************/
void checkReboot() {
    int countDown;
    /* Long press power off */
    if (digitalRead(UP_BTN)==LOW)
    {
        uint32_t time_count = millis();
        while (digitalRead(UP_BTN)==LOW)
        {
            // Display poweroff bar only if holding button
            if (millis() - time_count > 500) {
                tft.setCursor(60, 12);
                tft.setTextSize(1);
                tft.setTextColor(TFT_RED, TFT_BLACK);
                countDown = (millis() - time_count) / 1000 + 1;
                tft.printf(" PWR OFF IN %d/3\n", countDown);
                delay(10);
            }
        }

        // Clear text after releasing the button
        delay(30);
        tft.fillRect(60, 12, WIDTH - 60, tft.fontHeight(1), TFT_BLACK);
    }
}