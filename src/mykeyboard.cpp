#include "mykeyboard.h"
#include "display.h"
#include "globals.h"

#if defined(CARDPUTER) || defined(STICK_C_PLUS2)
  #include <driver/adc.h>
  #include <esp_adc_cal.h>
  #include <soc/soc_caps.h>
  #include <soc/adc_channel.h>
#endif

#if defined(CARDPUTER)
  Keyboard_Class Keyboard = Keyboard_Class();

#elif defined (STICK_C_PLUS)
  AXP192 axp192;

#elif defined (CORE2)
  //AXP axp192;

#endif

/* Verifies Upper Btn to go to previous item */

bool checkNextPress(){
  #if defined (CARDPUTER)
    Keyboard.update();
    if(Keyboard.isKeyPressed('/') || Keyboard.isKeyPressed('.'))
  #elif defined(STICK_C_PLUS) || defined(STICK_C_PLUS2) || defined(STICK_C)
    if(digitalRead(DW_BTN)==LOW) 
  #elif defined(CORE2)
    M5.update();
    if(M5.BtnC.isHolding() || M5.BtnC.isPressed())               // read touchscreen
  #endif
  { return true; }

  else return false;
}

/* Verifies Down Btn to go to next item */
bool checkPrevPress() {
  #if defined(STICK_C_PLUS2)
    if(digitalRead(UP_BTN)==LOW) 
  #elif defined(STICK_C_PLUS)
    if(axp192.GetBtnPress())
  #elif defined(CARDPUTER)
    Keyboard.update();
    if(Keyboard.isKeyPressed(',') || Keyboard.isKeyPressed(';'))
  #elif defined(CORE2)
    M5.update();
    if(M5.BtnA.isHolding() || M5.BtnA.isPressed())               // read touchscreen
  #endif
  { return true; }

  else return false;
}

/* Verifies if Select or OK was pressed */
bool checkSelPress(){
  #if defined (CARDPUTER)
    Keyboard.update();
    if(Keyboard.isKeyPressed(KEY_ENTER))
  
  #elif defined(STICK_C_PLUS) || defined(STICK_C_PLUS2) || defined(STICK_C)
    if(digitalRead(SEL_BTN)==LOW) 
  
  #elif defined(CORE2)
    M5.update();
    if(M5.BtnB.isHolding() || M5.BtnB.isPressed())               // read touchscreen
  
  #endif
  { return true; }

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

  #elif defined(CORE2)
  percent = M5.Power.getBatteryLevel();

  #else
  
    #if defined(CARDPUTER)
      uint8_t _batAdcCh = ADC1_GPIO10_CHANNEL;
      uint8_t _batAdcUnit = 1;
    #elif defined(STICK_C_PLUS2)
      uint8_t _batAdcCh = ADC1_GPIO38_CHANNEL;
      uint8_t _batAdcUnit = 1;
    #endif
  
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten((adc1_channel_t)_batAdcCh, ADC_ATTEN_DB_11);
    static esp_adc_cal_characteristics_t* adc_chars = nullptr;
    static constexpr int BASE_VOLATAGE = 3600;
    adc_chars = (esp_adc_cal_characteristics_t*)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize((adc_unit_t)_batAdcUnit, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, BASE_VOLATAGE, adc_chars);
    int raw;
    raw = adc1_get_raw((adc1_channel_t)_batAdcCh);
    uint32_t volt = esp_adc_cal_raw_to_voltage(raw, adc_chars);

    float mv = volt * 2;
    percent = (mv - 3300) * 100 / (float)(4150 - 3350);

  #endif
  return  (percent < 0) ? 0
        : (percent >= 100) ? 100
        :  percent;
}


#ifndef STICK_C
/* Starts keyboard to type data */
String keyboard(String mytext, int maxSize, String msg) {
  sprite.deleteSprite();
  sprite.createSprite(WIDTH,HEIGHT);

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
  while(1) {
    sprite.setCursor(0,0);
    sprite.fillSprite(BGCOLOR);
    sprite.setTextColor(TFT_WHITE, BGCOLOR);
    sprite.setTextSize(FONT_M);

    //Draw the rectangles
    sprite.drawRect(7,2,46,20,TFT_WHITE);       // Ok Rectangle
    sprite.drawRect(55,2,50,20,TFT_WHITE);      // CAP Rectangle
    sprite.drawRect(107,2,50,20,TFT_WHITE);     // DEL Rectangle
    sprite.drawRect(159,2,74,20,TFT_WHITE);     // SPACE Rectangle
    sprite.drawRect(3,32,WIDTH-3,20,FGCOLOR); // mystring Rectangle


    if(x==0 && y==-1) { sprite.setTextColor(BGCOLOR, TFT_WHITE); sprite.fillRect(7,2,50,20,TFT_WHITE); }
    else sprite.setTextColor(TFT_WHITE, BGCOLOR);
    sprite.drawString("OK", 18, 4);

    
    if(x==1 && y==-1) { sprite.setTextColor(BGCOLOR, TFT_WHITE); sprite.fillRect(55,2,50,20,TFT_WHITE); }
    else if(caps) { sprite.fillRect(55,2,50,20,TFT_DARKGREY); sprite.setTextColor(TFT_WHITE, TFT_DARKGREY); }
    else sprite.setTextColor(TFT_WHITE, BGCOLOR);
    sprite.drawString("CAP", 64, 4);
   

    if(x==2 && y==-1) { sprite.setTextColor(BGCOLOR, TFT_WHITE); sprite.fillRect(107,2,50,20,TFT_WHITE); }
    else sprite.setTextColor(TFT_WHITE, BGCOLOR);
    sprite.drawString("DEL", 115, 4);

    if(x>2 && y==-1) { sprite.setTextColor(BGCOLOR, TFT_WHITE); sprite.fillRect(159,2,74,20,TFT_WHITE); }
    else sprite.setTextColor(TFT_WHITE, BGCOLOR);
    sprite.drawString("SPACE", 168, 4);

    sprite.setTextSize(FONT_P);
    sprite.setTextColor(FGCOLOR);
    sprite.drawString(msg.substring(0,38), 3, 24);
    
    sprite.setTextSize(FONT_M);


    sprite.setTextColor(FGCOLOR, 0x5AAB);    
    if(mytext.length()>19) { 
      sprite.setTextSize(FONT_P);
      if(mytext.length()>38) { 
        sprite.drawString(mytext.substring(0,38), 5, 34);
        sprite.drawString(mytext.substring(38,mytext.length()), 5, 42);
      } 
      else {
        sprite.drawString(mytext, 5, 34);
      }
    } else {
      sprite.drawString(mytext, 5, 34);
    }
    

    sprite.setTextColor(TFT_WHITE, BGCOLOR);    
    sprite.setTextSize(FONT_M);


    for(i=0;i<4;i++) {
      for(j=0;j<12;j++) {
        /* If selected, change font color and draw Rectangle*/
        if(x==j && y==i) { sprite.setTextColor(BGCOLOR, TFT_WHITE); sprite.fillRect(j*18+11,i*19+54,21,19,TFT_WHITE);}
        
        /* Print the letters */
        if(!caps) sprite.drawChar(keys[i][j][0], (j*18+16), (i*19+56));
        else sprite.drawChar(keys[i][j][1], (j*18+16), (i*19+56));

        /* Return colors to normal to print the other letters */
        if(x==j && y==i) { sprite.setTextColor(TFT_WHITE, BGCOLOR); }
      }
    }



    // Print the Sprite into screen
    sprite.pushSprite(0,0);


    /* When Select a key in keyboard */
    #if defined (CARDPUTER)
    Keyboard.update();
    if (Keyboard.isChange()) {
      Keyboard_Class::KeysState status = Keyboard.keysState();
      for (auto i : status.word) {
        mytext += i;
      }
      if (status.del && mytext.length() > 0) {
        // Handle backspace key
        mytext.remove(mytext.length() - 1);
      }
      if (status.enter) {
        break;
      }
      delay(150);
    }

    #else
    if(checkSelPress())  { 
      int z=0;
      if(caps) z=1;
      else z=0;
      if(x==0 && y==-1) break;
      else if(x==1 && y==-1) caps=!caps;
      else if(x==2 && y==-1 && mytext.length() > 0) mytext.remove(mytext.length()-1);
      else if(x>2 && y==-1 && mytext.length()<maxSize) mytext += " ";
      else if(y>-1 && mytext.length()<maxSize) mytext += keys[y][x][z];
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
      
    }
    /* UP Btn to move in Y axis (Downwards) */
    if(checkPrevPress()) { 
      delay(200);

      if(checkPrevPress()) { y--; delay(250);  }// Long press
      else y++; // short press
      
      if(y>3) { y=-1; }
      else if(y<-1) y=3;      
    }

    #endif

  }
  
  //Resets screen when finished writing
  sprite.fillRect(0,0,sprite.width(),sprite.height(),BGCOLOR);
  resetTftDisplay();

  return mytext;
}

#else

/* Starts keyboard to type data */
String keyboard(String mytext, int maxSize, String msg) {
  sprite.deleteSprite();
  sprite.createSprite(WIDTH,HEIGHT);

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
  while(1) {
    sprite.setCursor(0,0);
    sprite.fillSprite(BGCOLOR);
    sprite.setTextColor(TFT_WHITE, BGCOLOR);
    sprite.setTextSize(FONT_M);

    //Draw the rectangles
    sprite.drawRect(7,2,20,10,TFT_WHITE);       // Ok Rectangle
    sprite.drawRect(27,2,25,10,TFT_WHITE);      // CAP Rectangle
    sprite.drawRect(52,2,25,10,TFT_WHITE);     // DEL Rectangle
    sprite.drawRect(77,2,50,10,TFT_WHITE);     // SPACE Rectangle
    sprite.drawRect(3,22,WIDTH-3,11,FGCOLOR); // mystring Rectangle


    if(x==0 && y==-1) { sprite.setTextColor(BGCOLOR, TFT_WHITE); sprite.fillRect(7,2,20,10,TFT_WHITE); }
    else sprite.setTextColor(TFT_WHITE, BGCOLOR);
    sprite.drawString("OK", 10, 4);

    
    if(x==1 && y==-1) { sprite.setTextColor(BGCOLOR, TFT_WHITE); sprite.fillRect(27,2,25,10,TFT_WHITE); }
    else if(caps) { sprite.fillRect(55,2,50,20,TFT_DARKGREY); sprite.setTextColor(TFT_WHITE, TFT_DARKGREY); }
    else sprite.setTextColor(TFT_WHITE, BGCOLOR);
    sprite.drawString("CAP", 30, 4);
   

    if(x==2 && y==-1) { sprite.setTextColor(BGCOLOR, TFT_WHITE); sprite.fillRect(52,2,25,10,TFT_WHITE); }
    else sprite.setTextColor(TFT_WHITE, BGCOLOR);
    sprite.drawString("DEL", 55, 4);

    if(x>2 && y==-1) { sprite.setTextColor(BGCOLOR, TFT_WHITE); sprite.fillRect(77,2,50,10,TFT_WHITE); }
    else sprite.setTextColor(TFT_WHITE, BGCOLOR);
    sprite.drawString("SPACE", 80, 4);

    sprite.setTextSize(FONT_P);
    sprite.setTextColor(FGCOLOR);
    sprite.drawString(msg.substring(0,38), 3, 14);
    
    sprite.setTextSize(FONT_M);


    sprite.setTextColor(FGCOLOR, 0x5AAB);    
    if(mytext.length()>19) { 
      sprite.setTextSize(FONT_P);
      if(mytext.length()>38) { 
        sprite.drawString(mytext.substring(0,38), 5, 24);
        sprite.drawString(mytext.substring(38,mytext.length()), 5, 32);
      } 
      else {
        sprite.drawString(mytext, 5, 24);
      }
    } else {
      sprite.drawString(mytext, 5, 24);
    }
    

    sprite.setTextColor(TFT_WHITE, BGCOLOR);    
    sprite.setTextSize(FONT_M);


    for(i=0;i<4;i++) {
      for(j=0;j<12;j++) {
        /* If selected, change font color and draw Rectangle*/
        if(x==j && y==i) { sprite.setTextColor(BGCOLOR, TFT_WHITE); sprite.fillRect(j*11+15,i*9+34,10,10,TFT_WHITE);}
        
        /* Print the letters */
        if(!caps) sprite.drawChar(keys[i][j][0], (j*11+18), (i*9+36));
        else sprite.drawChar(keys[i][j][1], (j*11+18), (i*9+36));

        /* Return colors to normal to print the other letters */
        if(x==j && y==i) { sprite.setTextColor(TFT_WHITE, BGCOLOR); }
      }
    }



    // Print the Sprite into screen
    sprite.pushSprite(0,0);


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
      
    }
    /* UP Btn to move in Y axis (Downwards) */
    if(checkPrevPress()) { 
      delay(200);

      if(checkPrevPress()) { y--; delay(250);  }// Long press
      else y++; // short press
      
      if(y>3) { y=-1; }
      else if(y<-1) y=3;      
    }


  }
  
  //Resets screen when finished writing
  sprite.fillRect(0,0,sprite.width(),sprite.height(),BGCOLOR);
  resetTftDisplay();

  return mytext;
}


#endif //If not STICK_C