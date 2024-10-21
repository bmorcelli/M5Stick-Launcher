#include "mykeyboard.h"
#include "display.h"
#include "globals.h"
#include "settings.h"


#define FP FONT_P
#define FM FONT_M
#define FG FONT_G

#if defined(CARDPUTER) || defined(STICK_C_PLUS2) || (defined(T_EMBED) && !defined(T_EMBED_1101)) || defined(T_DECK)
  #include <driver/adc.h>
  #include <esp_adc_cal.h>
  #include <soc/soc_caps.h>
  #include <soc/adc_channel.h>
#endif

#if defined(T_DISPLAY_S3)
  #include <esp_adc_cal.h>
  TouchLib touch(Wire, 18, 17, CTS820_SLAVE_ADDRESS, 21);
#endif

#if defined(CYD)
  CYD28_TouchR touch(CYD28_DISPLAY_HOR_RES_MAX, CYD28_DISPLAY_VER_RES_MAX);
#endif

#if defined(T_DECK)

bool trackball_interrupted = false;
int8_t trackball_up_count = 0;
int8_t trackball_down_count = 0;
int8_t trackball_left_count = 0;
int8_t trackball_right_count = 0;
void IRAM_ATTR ISR_up()   { trackball_interrupted = true; trackball_up_count = 1;   }
void IRAM_ATTR ISR_down() { trackball_interrupted = true; trackball_down_count = 1; }
void IRAM_ATTR ISR_left() { trackball_interrupted = true; trackball_left_count = 1; }
void IRAM_ATTR ISR_right(){ trackball_interrupted = true; trackball_right_count = 1;}

void ISR_rst(){
  trackball_up_count = 0;
  trackball_down_count = 0;
  trackball_left_count = 0;
  trackball_right_count = 0;
  trackball_interrupted = false;
}
bool menuPress(int bot){
  // 0 - UP
  // 1 - Down
  // 2 - Left
  // 3 - Right
  if (trackball_interrupted)
  {
    uint8_t xx=1;
    uint8_t yy=1;
    xx += trackball_left_count;
    xx -= trackball_right_count;
    yy -= trackball_up_count;
    yy += trackball_down_count;
    if(xx==1 && yy==1) {
      ISR_rst();
      return false;
    }
    delay(50);
    // Print "bot - xx - yy",  1 is normal value for xx and yy 0 and 2 means movement on the axis
    //Serial.print(bot); Serial.print("-"); Serial.print(xx); Serial.print("-"); Serial.println(yy);
    if (xx < 1 && bot==2)       { ISR_rst();   return true;  } // left
    else if (xx > 1 && bot==3)  { ISR_rst();   return true;  } // right
    else if (yy < 1 && bot==0)  { ISR_rst();   return true;  } // up
    else if (yy > 1 && bot==1)  { ISR_rst();   return true;  } // down
    else return false;
  }
  else return false;

}
#endif

#if defined(T_EMBED)
  #if defined(T_EMBED_1101)
    // Power handler for battery detection
    XPowersPPM PPM;
  #endif
  //RotaryEncoder encoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);
  RotaryEncoder *encoder = nullptr;
  int _new_pos = 0;
  int _last_pos = 0;
  int _last_dir = 0;
  IRAM_ATTR void checkPosition()
    {
      encoder->tick(); // just call tick() to check the state.
      _last_dir = (int)encoder->getDirection();
      _last_pos = _new_pos;
      _new_pos = encoder->getPosition();
    }

  bool menuPress(int bot){
    //0 - prev
    //1 - Sel
    //2 - next
    if(bot==0 && _last_dir>0) {
      _last_dir=0;
      return true;
    }
    if(bot==2 && _last_dir<0) {
      _last_dir=0;
      return true;
    }
    if(bot==1 && digitalRead(SEL_BTN)==BTN_ACT) {
      _last_dir=0;
      return true;
    }
    return false;
  }
#endif


#if defined(CARDPUTER)
  Keyboard_Class Keyboard = Keyboard_Class();

#elif defined (STICK_C_PLUS)
  AXP192 axp192;
#endif

#if defined(M5STACK) || defined(T_DISPLAY_S3) || defined(CYD) || defined(MARAUDERV4)
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

#endif
void resetDimmer() {
  dimmerTemp=millis();
  setBrightness(bright);
  setCpuFrequencyMhz(240);
  dimmer=false;
  delay(200);
}

#if defined(T_DISPLAY_S3)
bool menuPress(int bot) {
  //0 - prev
  //1 - Sel
  //2 - next
  int terco=WIDTH/3;
  if (touch.read()) {
    auto t = touch.getPoint(0);
    if(rotation==3) t.x = WIDTH-t.x;
    else if (rotation==1) t.y = (HEIGHT+20)-t.y;

    if(t.y>(HEIGHT) && t.x>terco*bot && t.x<terco*(1+bot)) { 
      t.x=WIDTH+1;
      t.y=HEIGHT+11;
      return true;
    } else return false;
  } else return false;
}
#endif

#if defined(CYD)

bool menuPress(int bot) {
  //0 - prev
  //1 - Sel
  //2 - next
  int terco=WIDTH/3;
  if (touch.touched()) { //touch.tirqTouched() && 
    auto t = touch.getPointScaled();
    t = touch.getPointScaled();
    log_i("Touchscreen Pressed at x=%d, y=%d, z=%d", t.x,t.y,t.z);
        if(rotation==3) { 
          t.y = (HEIGHT+20)-t.y;
          t.x = WIDTH-t.x;
        }

    if(t.y>(HEIGHT) && t.x>terco*bot && t.x<terco*(1+bot)) { 
      t.x=WIDTH+1;
      t.y=HEIGHT+11;
      return true;
    } else return false;
  } else return false;
}
#endif

#if defined(MARAUDERV4)
struct TouchPoint {
  uint16_t x;
  uint16_t y;
};

bool menuPress(int bot) {
  //0 - prev
  //1 - Sel
  //2 - next
  TouchPoint t;
  bool touched = tft.getTouch(&t.x, &t.y, 600);

  int terco=WIDTH/3;
  if(touched) { 
    log_i("Touchscreen Pressed at x=%d, y=%d", t.x,t.y);
    if(rotation==3) { 
      t.y = (HEIGHT+20)-t.y;
      t.x = WIDTH-t.x;
    }

    if(t.y>(HEIGHT) && t.x>terco*bot && t.x<terco*(1+bot)) { 
      t.x=WIDTH+1;
      t.y=HEIGHT+11;
      return true;
    } else return false;
  } else return false;
}
#endif


/* Verifies Upper Btn to go to previous item */
bool checkNextPress(){
  #if defined (CARDPUTER)
    Keyboard.update();
    if(Keyboard.isKeyPressed('/') || Keyboard.isKeyPressed('.'))
  #elif defined(STICK_C_PLUS) || defined(STICK_C_PLUS2) || defined(STICK_C)
    if(digitalRead(DW_BTN)==BTN_ACT) 
  #elif defined(M5STACK)
    M5.update();
    if(M5.BtnC.isHolding() || M5.BtnC.isPressed())               // read touchscreen
  #elif defined(T_DISPLAY_S3)
    if(digitalRead(DW_BTN)==BTN_ACT || menuPress(2)) 
  #elif defined(CYD) || defined(MARAUDERV4) || defined(T_EMBED)  
    if(menuPress(2)) 
  #elif defined(T_DECK)
    //if(digitalRead(UP_BTN)==BTN_ACT)// || digitalRead(R_BTN)==BTN_ACT)
    if(menuPress(1) || menuPress(3))
  //if(true==false) // returnes false
  #elif defined(HAS_BTN) && DW_BTN>=0
    if(digitalRead(DW_BTN)==BTN_ACT)
  #else
    if(true==false) // returnes false
  #endif
    { 
      if(dimmer) {
        resetDimmer();
        return false;
      }
      dimmerTemp=millis(); 
      return true; 
    }

  else return false;
}

/* Verifies Down Btn to go to next item */
bool checkPrevPress() {
  #if defined(STICK_C_PLUS2)
    if(digitalRead(UP_BTN)==BTN_ACT) 
  #elif defined(STICK_C_PLUS)
    if(axp192.GetBtnPress())
  #elif defined(CARDPUTER)
    Keyboard.update();
    if(Keyboard.isKeyPressed(',') || Keyboard.isKeyPressed(';'))
  #elif defined(M5STACK)
    M5.update();
    if(M5.BtnA.isHolding() || M5.BtnA.isPressed())               // read touchscreen
  #elif defined(T_DISPLAY_S3) || defined(CYD) || defined(MARAUDERV4) || defined(T_EMBED)  
    if(menuPress(0))  
  #elif defined(T_DECK)
    //if(true==false) // returnes false
    //if(digitalRead(DW_BTN)==BTN_ACT)// || digitalRead(L_BTN)==BTN_ACT)    
    if(menuPress(0) || menuPress(2))
  #elif defined(HAS_BTN) && UP_BTN>=0
    if(digitalRead(UP_BTN)==BTN_ACT)     
  #else
    if(true==false) // returnes false
  #endif
  { 
    if(dimmer) {
      resetDimmer();
      return false;
    }
    dimmerTemp=millis(); 
    return true; 
    }

  else return false;
}

/* Verifies if Select or OK was pressed */
bool checkSelPress(bool dimmOff){
  if((dimmerTemp+dimmerSet*1000) < millis() && !dimmer) {
    dimmer = true;
    setBrightness(5,false);
    if(!dimmOff) setCpuFrequencyMhz(80);
  }  
  #if defined (CARDPUTER)
    Keyboard.update();
    if(Keyboard.isKeyPressed(KEY_ENTER))
  
  #elif defined(STICK_C_PLUS) || defined(STICK_C_PLUS2) || defined(STICK_C) || defined(MARAUDERMINI)
    if(digitalRead(SEL_BTN)==BTN_ACT) 
  
  #elif defined(M5STACK)
    M5.update();
    if(M5.BtnB.isHolding() || M5.BtnB.isPressed())               // read touchscreen
  
  #elif defined(T_DISPLAY_S3)
    if(digitalRead(SEL_BTN)==BTN_ACT || menuPress(1)) 

  #elif defined(CYD) || defined(MARAUDERV4) || defined(T_EMBED)  
    if(menuPress(1)) 
  
  #elif defined(T_DECK)
    char keyValue = 0;
    Wire.requestFrom(LILYGO_KB_SLAVE_ADDRESS, 1);
    while (Wire.available() > 0) {
        keyValue = Wire.read();
    }
    if(digitalRead(SEL_BTN)==BTN_ACT || keyValue==0x0D) //Enter Key

  #elif defined(HAS_BTN) && SEL_BTN>=0
    if(digitalRead(SEL_BTN)==BTN_ACT) 

  #endif
    { 
    if(dimmer) {
      resetDimmer();
      return false;
    }
    dimmerTemp=millis(); 
    return true; 
    }

  else return false;
}


/***************************************************************************************
** Function name: getBattery()
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() {
  int percent=0;
  #if defined(STICK_C_PLUS) || defined(STICK_C)
  float b = axp192.GetBatVoltage();
  percent = ((b - 3.0) / 1.2) * 100;

  #elif defined(M5STACK)
  percent = M5.Power.getBatteryLevel();

  #elif defined(T_DISPLAY_S3)

  esp_adc_cal_characteristics_t adc_chars;

  // Get the internal calibration value of the chip
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  uint32_t raw = analogRead(BAT_PIN);
  uint32_t v1 = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2; //The partial pressure is one-half
  if(v1<4300) {
    float mv = v1 * 2;
    percent = (mv - 3300) * 100 / (float)(4150 - 3350);
  } else  { percent = 0; } 

  #elif defined(CARDPUTER) || defined(STICK_C_PLUS2)
  
    #if defined(CARDPUTER)
      uint8_t _batAdcCh = ADC1_GPIO10_CHANNEL;
      uint8_t _batAdcUnit = 1;
    #elif defined(STICK_C_PLUS2)
      uint8_t _batAdcCh = ADC1_GPIO38_CHANNEL;
      uint8_t _batAdcUnit = 1;
    #endif
  
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten((adc1_channel_t)_batAdcCh, ADC_ATTEN_DB_12);
    static esp_adc_cal_characteristics_t* adc_chars = nullptr;
    static constexpr int BASE_VOLATAGE = 3600;
    adc_chars = (esp_adc_cal_characteristics_t*)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize((adc_unit_t)_batAdcUnit, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, BASE_VOLATAGE, adc_chars);
    int raw;
    raw = adc1_get_raw((adc1_channel_t)_batAdcCh);
    uint32_t volt = esp_adc_cal_raw_to_voltage(raw, adc_chars);

    float mv = volt * 2;
    percent = (mv - 3300) * 100 / (float)(4150 - 3350);
    #elif defined(T_EMBED_1101)
    percent=(PPM.getSystemVoltage()-3300)*100/(float)(4150-3350);
    
    #elif defined(T_EMBED) || defined(T_DECK)
    uint8_t _batAdcCh = ADC1_GPIO4_CHANNEL;
    uint8_t _batAdcUnit = 1;
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten((adc1_channel_t)_batAdcCh, ADC_ATTEN_DB_12);
    static esp_adc_cal_characteristics_t* adc_chars = nullptr;
    static constexpr int BASE_VOLATAGE = 3600;
    adc_chars = (esp_adc_cal_characteristics_t*)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize((adc_unit_t)_batAdcUnit, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, BASE_VOLATAGE, adc_chars);
    int raw;
    raw = adc1_get_raw((adc1_channel_t)_batAdcCh);
    uint32_t volt = esp_adc_cal_raw_to_voltage(raw, adc_chars);

    float mv = volt * 2;
    percent = (mv - 3300) * 100 / (float)(4150 - 3350);


  #else
    percent=0;

  #endif
  return  (percent < 0) ? 0
        : (percent >= 100) ? 100
        :  percent;
}



#ifndef STICK_C
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

#if defined(M5STACK) || defined(T_DISPLAY_S3) || defined(CYD) || defined(MARAUDERV4)
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
#endif


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
      dimmerTemp=millis(); // reset dimmer without delaying 200ms after each keypress
    #if defined(T_DISPLAY_S3)
      touch.read();
    #endif      
    }
  #if defined(M5STACK)
    coreFooter();
  #endif
  #if defined(T_DISPLAY_S3) || defined(CYD) || defined(MARAUDERV4)
    TdisplayS3Footer();
  #endif
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

    /* When Select a key in keyboard */
    #if defined (CARDPUTER)

    Keyboard.update();
    if (Keyboard.isPressed()) {
      resetDimmer();
      tft.setCursor(cX,cY);
      Keyboard_Class::KeysState status = Keyboard.keysState();
      for (auto i : status.word) {
        if(mytext.length()<maxSize) {
          mytext += i;
          if(mytext.length()!=20 && mytext.length()!=20) tft.print(i);
          cX=tft.getCursorX();
          cY=tft.getCursorY();
          if(mytext.length()==20) redraw = true;
          if(mytext.length()==39) redraw = true;
        }
      }
      if (status.del && mytext.length() > 0) {
        // Handle backspace key
        mytext.remove(mytext.length() - 1);
        int fS=FM;
        if(mytext.length()>19) { tft.setTextSize(FP); fS=FP; }
        else tft.setTextSize(FM);
        tft.setCursor((cX-fS*LW),cY);
        tft.setTextColor(FGCOLOR,BGCOLOR);
        tft.print(" "); 
        tft.setTextColor(~BGCOLOR, 0x5AAB);
        tft.setCursor(cX-fS*LW,cY);
        cX=tft.getCursorX();
        cY=tft.getCursorY();
        if(mytext.length()==19) redraw = true;
        if(mytext.length()==38) redraw = true;        
      }
      if (status.enter) {
        break;
      }
      delay(200);
    }
    if(checkSelPress()) break;

  #elif defined(T_DECK)

    char keyValue = 0;
    Wire.requestFrom(LILYGO_KB_SLAVE_ADDRESS, 1);
    while (Wire.available() > 0) {
        keyValue = Wire.read();
    }
    if (keyValue != (char)0x00) {
        Serial.print("keyValue : ");
        Serial.print(keyValue);
        Serial.print(" -> Hex  0x");
        Serial.println(keyValue,HEX);
        resetDimmer();
        tft.setCursor(cX,cY);

        if(mytext.length()<maxSize && keyValue!=0x08 && keyValue!=0x0D) {
          mytext += keyValue;
          if(mytext.length()!=20 && mytext.length()!=20) tft.print(keyValue);
          cX=tft.getCursorX();
          cY=tft.getCursorY();
          if(mytext.length()==20) redraw = true;
          if(mytext.length()==39) redraw = true;
        }
        if (keyValue==0x08 && mytext.length() > 0) { // delete 0x08
          // Handle backspace key
          mytext.remove(mytext.length() - 1);
          int fS=FM;
          if(mytext.length()>19) { tft.setTextSize(FP); fS=FP; }
          else tft.setTextSize(FM);
          tft.setCursor((cX-fS*LW),cY);
          tft.setTextColor(FGCOLOR,BGCOLOR);
          tft.print(" "); 
          tft.setTextColor(~BGCOLOR, 0x5AAB);
          tft.setCursor(cX-fS*LW,cY);
          cX=tft.getCursorX();
          cY=tft.getCursorY();
          if(mytext.length()==19) redraw = true;
          if(mytext.length()==38) redraw = true;        
        }
        if (keyValue==0x0D) {
          break;
        }
        //delay(200);
    }
    if(checkSelPress()) break;
    
    delay(5);


  #else

    int z=0;
  #if defined(M5STACK) || defined(T_DISPLAY_S3) || defined(CYD) || defined(MARAUDERV4)
    #if defined(M5STACK)
    M5.update();
    auto t = M5.Touch.getDetail();
    if (t.wasClicked()) 
    #elif defined(T_DISPLAY_S3)
    if (touch.read())
    #elif defined(CYD)
    if (touch.touched())
    #elif defined(MARAUDERV4)
    TouchPoint t;
    bool touched = tft.getTouch(&t.x, &t.y, 600);

    if(rotation==3) { 
      t.y = (HEIGHT+20)-t.y;
      t.x = WIDTH-t.x;
    }
    if(touched)
    #endif
     {
      #if defined(T_DISPLAY_S3)
        auto t = touch.getPoint(0);
        if(rotation==3) {
          //t.y = HEIGHT-t.y;
          t.x = WIDTH-t.x;
        } else if (rotation==1) {
          t.y = (HEIGHT+20)-t.y;
        }
      #elif defined(CYD)
        auto t = touch.getPointScaled();
        if(rotation==3) { 
          t.y = (HEIGHT+20)-t.y;
          t.x = WIDTH-t.x;
        }
      #endif
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
      #if defined(T_DISPLAY_S3)
      t.x=WIDTH+1;
      t.y=HEIGHT+11;
      #endif      
      redraw=true;
    }

  #endif

    if(checkSelPress())  { 
      tft.setCursor(cX,cY);
      if(caps) z=1;
      else z=0;
      if(x==0 && y==-1) break;
      else if(x==1 && y==-1) { caps=!caps; tft.fillRect(0,54,WIDTH,HEIGHT-54,BGCOLOR); }
      else if(x==2 && y==-1 && mytext.length() > 0) {
        DEL:
        mytext.remove(mytext.length()-1);
        int fS=FM;
        if(mytext.length()>19) { tft.setTextSize(FP); fS=FP; }
        else tft.setTextSize(FM);
        tft.setCursor((cX-fS*LW),cY);
        tft.setTextColor(FGCOLOR,BGCOLOR);
        tft.print(" "); 
        tft.setTextColor(~BGCOLOR, 0x5AAB);
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
      redraw = true;
      delay(200);
    }

    /* Down Btn to move in X axis (to the right) */  
    if(checkNextPress()) 
    { 
      #if defined(T_EMBED)
      // To handle Encoder devices such as T-EMBED
      if(digitalRead(BK_BTN) == BTN_ACT) { y++; }
      else x++;

      if(y>3) { y=-1; }
      else if(y<-1) y=3;

      #else
      delay(250);
      if(checkNextPress()) { x--; delay(250); } // Long Press
      else x++; // Short Press
      #endif// To handle Encoder devices such as T-EMBED

      if(y<0 && x>3) x=0;
      if(x>11) x=0;
      else if (x<0) x=11;
      redraw = true;
    }
    /* UP Btn to move in Y axis (Downwards) */
    if(checkPrevPress()) { 
      #if defined(T_EMBED)
      // To handle Encoder devices such as T-EMBED
      if(digitalRead(BK_BTN) == BTN_ACT) { y--; }
      else x--;

      if(y<0 && x<0) x=3;
      if(x>11) x=0;
      else if (x<0) x=11;
      
      // To handle Encoder devices such as T-EMBED
      #else 
      delay(250);
      if(checkPrevPress()) { y--; delay(250);  }// Long press
      else y++; // short press
      #endif // To handle Encoder devices such as T-EMBED
      if(y>3) { y=-1; }
      else if(y<-1) y=3;
      redraw = true;
    }

    #endif

  }
  
  //Resets screen when finished writing
  tft.fillRect(0,0,tft.width(),tft.height(),BGCOLOR);
  resetTftDisplay();

  return mytext;
}

#else

/* Starts keyboard to type data */
String keyboard(String mytext, int maxSize, String msg) {

  resetTftDisplay();
  bool caps=false;
  int x=0;
  int y=-1;
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

  int i=0;
  int j=0;
  delay(200);
  bool redraw = true;
  while(1) {
    if(redraw) {
      tft.setCursor(0,0);
      tft.fillScreen(BGCOLOR);
      tft.setTextColor(~BGCOLOR, BGCOLOR);
      tft.setTextSize(FM);

      //Draw the rectangles
      tft.drawRect(7,2,20,10,~BGCOLOR);       // Ok Rectangle
      tft.drawRect(27,2,25,10,~BGCOLOR);      // CAP Rectangle
      tft.drawRect(52,2,25,10,~BGCOLOR);     // DEL Rectangle
      tft.drawRect(77,2,50,10,~BGCOLOR);     // SPACE Rectangle
      tft.drawRect(3,22,WIDTH-3,11,FGCOLOR); // mystring Rectangle


      if(x==0 && y==-1) { tft.setTextColor(BGCOLOR, ~BGCOLOR); tft.fillRect(7,2,20,10,~BGCOLOR); }
      else tft.setTextColor(~BGCOLOR, BGCOLOR);
      tft.drawString("OK", 10, 4);

      
      if(x==1 && y==-1) { tft.setTextColor(BGCOLOR, ~BGCOLOR); tft.fillRect(27,2,25,10,~BGCOLOR); }
      else if(caps) { tft.fillRect(55,2,50,20,TFT_DARKGREY); tft.setTextColor(~BGCOLOR, TFT_DARKGREY); }
      else tft.setTextColor(~BGCOLOR, BGCOLOR);
      tft.drawString("CAP", 30, 4);
    

      if(x==2 && y==-1) { tft.setTextColor(BGCOLOR, ~BGCOLOR); tft.fillRect(52,2,25,10,~BGCOLOR); }
      else tft.setTextColor(~BGCOLOR, BGCOLOR);
      tft.drawString("DEL", 55, 4);

      if(x>2 && y==-1) { tft.setTextColor(BGCOLOR, ~BGCOLOR); tft.fillRect(77,2,50,10,~BGCOLOR); }
      else tft.setTextColor(~BGCOLOR, BGCOLOR);
      tft.drawString("SPACE", 80, 4);

      tft.setTextSize(FP);
      tft.setTextColor(~BGCOLOR);
      tft.drawString(msg.substring(0,38), 3, 14);
      
      tft.setTextSize(FM);


      tft.setTextColor(~BGCOLOR, 0x5AAB);    
      if(mytext.length()>19) { 
        tft.setTextSize(FP);
        if(mytext.length()>38) { 
          tft.drawString(mytext.substring(0,38), 5, 24);
          tft.drawString(mytext.substring(38,mytext.length()), 5, 32);
        } 
        else {
          tft.drawString(mytext, 5, 24);
        }
      } else {
        tft.drawString(mytext, 5, 24);
      }
      

      tft.setTextColor(~BGCOLOR, BGCOLOR);    
      tft.setTextSize(FM);


      for(i=0;i<4;i++) {
        for(j=0;j<12;j++) {
          /* If selected, change font color and draw Rectangle*/
          if(x==j && y==i) { tft.setTextColor(BGCOLOR, ~BGCOLOR); tft.fillRect(j*11+15,i*9+34,10,10,~BGCOLOR);}
          
          /* Print the letters */
          if(!caps) tft.drawChar(keys[i][j][0], (j*11+18), (i*9+36));
          else tft.drawChar(keys[i][j][1], (j*11+18), (i*9+36));

          /* Return colors to normal to print the other letters */
          if(x==j && y==i) { tft.setTextColor(~BGCOLOR, BGCOLOR); }
        }
      }
      redraw = false;
    }


    /* When Select a key in keyboard */

    if(checkSelPress())  { 
      int z=0;
      if(caps) z=1;
      else z=0;
      if(x==0 && y==-1) break;
      else if(x==1 && y==-1) caps=!caps;
      else if(x==2 && y==-1 && mytext.length() > 0) mytext.remove(mytext.length()-1);
      else if(x>2 && y==-1 && mytext.length()<maxSize) mytext += " ";
      else if(y>-1 && mytext.length()<maxSize) mytext += keys[y][x][z];
      redraw = true;
      delay(200);
    }

    /* Down Btn to move in X axis (to the right) */  
    if(checkNextPress()) 
    { 
      delay(200);  
      if(checkNextPress()) { x--; delay(250); } // Long Press
      else x++; // Short Press

      if(y<0 && x>3) x=0;
      if(x>11) x=0;
      else if (x<0) x=11;
      redraw = true;
    }
    /* UP Btn to move in Y axis (Downwards) */
    if(checkPrevPress()) { 
      delay(200);

      if(checkPrevPress()) { y--; delay(250);  }// Long press
      else y++; // short press
      
      if(y>3) { y=-1; }
      else if(y<-1) y=3;      
      redraw = true;
    }


  }
  
  //Resets screen when finished writing
  tft.fillRect(0,0,tft.width(),tft.height(),BGCOLOR);
  resetTftDisplay();

  return mytext;
}


#endif //If not STICK_C
