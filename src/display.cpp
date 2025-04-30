#include "display.h"
#include <globals.h>
#include "mykeyboard.h"
#include "onlineLauncher.h"
#include "sd_functions.h"
#include "settings.h"

#if defined(HEADLESS)
SerialDisplayClass *tft= new SerialDisplayClass();
#elif defined(E_PAPER_DISPLAY) || defined(USE_TFT_ESPI) || defined(USE_LOVYANGFX) || defined(GxEPD2_DISPLAY)
Ard_eSPI *tft = new Ard_eSPI();
#else
  #ifdef TFT_PARALLEL_8_BIT
    Arduino_DataBus *bus = new Arduino_ESP32PAR8Q(TFT_DC, TFT_CS,TFT_WR, TFT_RD, TFT_D0, TFT_D1, TFT_D2, TFT_D3, TFT_D4, TFT_D5, TFT_D6, TFT_D7);
  #elif RGB_PANEL // 16-par connections
    Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
      #if defined(DISPLAY_ST7262_PAR)
      ST7262_PANEL_CONFIG_DE_GPIO_NUM /* DE */, 
      ST7262_PANEL_CONFIG_VSYNC_GPIO_NUM /* VSYNC */, 
      ST7262_PANEL_CONFIG_HSYNC_GPIO_NUM /* HSYNC */, 
      ST7262_PANEL_CONFIG_PCLK_GPIO_NUM /* PCLK */,
      ST7262_PANEL_CONFIG_DATA_GPIO_B0 /* R0 */, // for ST7262 panels (SUNTON boards) R and B are changed
      ST7262_PANEL_CONFIG_DATA_GPIO_B1 /* R1 */, 
      ST7262_PANEL_CONFIG_DATA_GPIO_B2 /* R2 */, 
      ST7262_PANEL_CONFIG_DATA_GPIO_B3 /* R3 */, 
      ST7262_PANEL_CONFIG_DATA_GPIO_B4 /* R4 */,
      ST7262_PANEL_CONFIG_DATA_GPIO_G0 /* G0 */, 
      ST7262_PANEL_CONFIG_DATA_GPIO_G1 /* G1 */, 
      ST7262_PANEL_CONFIG_DATA_GPIO_G2 /* G2 */, 
      ST7262_PANEL_CONFIG_DATA_GPIO_G3 /* G3 */, 
      ST7262_PANEL_CONFIG_DATA_GPIO_G4 /* G4 */, 
      ST7262_PANEL_CONFIG_DATA_GPIO_G5 /* G5 */,
      ST7262_PANEL_CONFIG_DATA_GPIO_R0 /* B0 */, 
      ST7262_PANEL_CONFIG_DATA_GPIO_R1 /* B1 */, 
      ST7262_PANEL_CONFIG_DATA_GPIO_R2 /* B2 */, 
      ST7262_PANEL_CONFIG_DATA_GPIO_R3 /* B3 */, 
      ST7262_PANEL_CONFIG_DATA_GPIO_R4 /* B4 */,
      0 /* hsync_polarity */, 
      ST7262_PANEL_CONFIG_TIMINGS_HSYNC_FRONT_PORCH /* hsync_front_porch */, 
      ST7262_PANEL_CONFIG_TIMINGS_HSYNC_PULSE_WIDTH /* hsync_pulse_width */, 
      ST7262_PANEL_CONFIG_TIMINGS_HSYNC_BACK_PORCH /* hsync_back_porch */,
      0 /* vsync_polarity */, 
      ST7262_PANEL_CONFIG_TIMINGS_VSYNC_FRONT_PORCH /* vsync_front_porch */, 
      ST7262_PANEL_CONFIG_TIMINGS_VSYNC_PULSE_WIDTH /* vsync_pulse_width */, 
      ST7262_PANEL_CONFIG_TIMINGS_VSYNC_BACK_PORCH /* vsync_back_porch */,
      ST7262_PANEL_CONFIG_TIMINGS_FLAGS_PCLK_ACTIVE_NEG /* pclk_active_neg */, 
      16000000 /* prefer_speed */
      #elif defined(DISPLAY_ST7701_PAR)
      ST7701_PANEL_CONFIG_DE_GPIO_NUM /* DE */, 
      ST7701_PANEL_CONFIG_VSYNC_GPIO_NUM /* VSYNC */, 
      ST7701_PANEL_CONFIG_HSYNC_GPIO_NUM /* HSYNC */, 
      ST7701_PANEL_CONFIG_PCLK_GPIO_NUM /* PCLK */,
      ST7701_PANEL_CONFIG_DATA_GPIO_R0 /* R0 */,
      ST7701_PANEL_CONFIG_DATA_GPIO_R1 /* R1 */, 
      ST7701_PANEL_CONFIG_DATA_GPIO_R2 /* R2 */, 
      ST7701_PANEL_CONFIG_DATA_GPIO_R3 /* R3 */, 
      ST7701_PANEL_CONFIG_DATA_GPIO_R4 /* R4 */,
      ST7701_PANEL_CONFIG_DATA_GPIO_G0 /* G0 */, 
      ST7701_PANEL_CONFIG_DATA_GPIO_G1 /* G1 */, 
      ST7701_PANEL_CONFIG_DATA_GPIO_G2 /* G2 */, 
      ST7701_PANEL_CONFIG_DATA_GPIO_G3 /* G3 */, 
      ST7701_PANEL_CONFIG_DATA_GPIO_G4 /* G4 */, 
      ST7701_PANEL_CONFIG_DATA_GPIO_G5 /* G5 */,
      ST7701_PANEL_CONFIG_DATA_GPIO_B0 /* B0 */, 
      ST7701_PANEL_CONFIG_DATA_GPIO_B1 /* B1 */, 
      ST7701_PANEL_CONFIG_DATA_GPIO_B2 /* B2 */, 
      ST7701_PANEL_CONFIG_DATA_GPIO_B3 /* B3 */, 
      ST7701_PANEL_CONFIG_DATA_GPIO_B4 /* B4 */,
      1 /* hsync_polarity */, 
      ST7701_PANEL_CONFIG_TIMINGS_HSYNC_FRONT_PORCH /* hsync_front_porch */, 
      ST7701_PANEL_CONFIG_TIMINGS_HSYNC_PULSE_WIDTH /* hsync_pulse_width */, 
      ST7701_PANEL_CONFIG_TIMINGS_HSYNC_BACK_PORCH /* hsync_back_porch */,
      1 /* vsync_polarity */, 
      ST7701_PANEL_CONFIG_TIMINGS_VSYNC_FRONT_PORCH /* vsync_front_porch */, 
      ST7701_PANEL_CONFIG_TIMINGS_VSYNC_PULSE_WIDTH /* vsync_pulse_width */, 
      ST7701_PANEL_CONFIG_TIMINGS_VSYNC_BACK_PORCH /* vsync_back_porch */,
      ST7701_PANEL_CONFIG_TIMINGS_FLAGS_PCLK_ACTIVE_NEG /* pclk_active_neg */
      #endif
      );
  #elif AXS15231B_QSPI
    Arduino_DataBus *bus = new Arduino_ESP32QSPI(TFT_CS, TFT_SCLK, TFT_D0, TFT_D1, TFT_D2, TFT_D3);
  #else // SPI Data Bus shared with SDCard and other SPIClass devices
    Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO,&SPI);
  #endif
  
  Ard_eSPI *tft = new Ard_eSPI(bus,TFT_RST,ROTATION,TFT_IPS,TFT_WIDTH,TFT_HEIGHT,TFT_COL_OFS1,TFT_ROW_OFS1,TFT_COL_OFS2,TFT_ROW_OFS2);
#endif

/***************************************************************************************
** Function name: displayScrollingText
** Description:   Scroll large texts into screen
***************************************************************************************/
void displayScrollingText(const String& text, Opt_Coord& coord) {
  int len = text.length();
  String displayText = text + "        "; // Add spaces for smooth looping
  int scrollLen = len + 8; // Full text plus space buffer
  static int i=0;
  static long _lastmillis=0;
  tft->setTextColor(coord.fgcolor,coord.bgcolor);
  if (len < coord.size) {
    // Text fits within limit, no scrolling needed
    return;
  } else if(millis()>_lastmillis+200) {
    String scrollingPart = displayText.substring(i, i + (coord.size - 1)); // Display charLimit characters at a time
    tft->fillRect(coord.x, coord.y, (coord.size-1) * LW * tft->getTextsize(), LH * tft->getTextsize(), BGCOLOR); // Clear display area
    tft->setCursor(coord.x, coord.y);
    tft->setCursor(coord.x, coord.y);
    tft->print(scrollingPart);
    if (i >= scrollLen - coord.size) i = -1; // Loop back
    _lastmillis=millis();
    i++;
    if(i==1) _lastmillis=millis()+1000;
  }
}

/***************************************************************************************
** Function name: resetTftDisplay
** Description:   set cursor to 0,0, screen and text to default color
***************************************************************************************/
void resetTftDisplay(int x, int y, uint16_t fc, int size, uint16_t bg, uint16_t screen) {
    tft->setCursor(x,y);
    tft->fillScreen(screen);
    tft->setTextSize(size);
    tft->setTextColor(fc,bg);
}

/***************************************************************************************
** Function name: setTftDisplay
** Description:   set cursor, font color, size and bg font color
***************************************************************************************/
void setTftDisplay(int x, int y, uint16_t fc, int size, uint16_t bg) {
    if (x>=0 && y<0)        tft->setCursor(x,tft->getCursorY());          // if -1 on x, sets only y
    else if (x<0 && y>=0)   tft->setCursor(tft->getCursorX(),y);          // if -1 on y, sets only x
    else if (x>=0 && y>=0)  tft->setCursor(x,y);                         // if x and y > 0, sets both
    tft->setTextSize(size);
    tft->setTextColor(fc,bg);
}

/***************************************************************************************
** Function name: TouchFooter
** Description:   Draw touch screen footer
***************************************************************************************/
void TouchFooter(uint16_t color) {
  tft->drawRoundRect(5,tftHeight+2,tftWidth-10,43,5,color);
  tft->setTextColor(color);
  tft->setTextSize(FM);
  tft->drawCentreString("PREV",tftWidth/6,tftHeight+4,1);
  tft->drawCentreString("SEL",tftWidth/2,tftHeight+4,1);
  tft->drawCentreString("NEXT",5*tftWidth/6,tftHeight+4,1);
}

/***************************************************************************************
** Function name: TouchFooter
** Description:   Draw touch screen footer
***************************************************************************************/
void TouchFooter2(uint16_t color) {
  tft->drawRoundRect(5,tftHeight+2,tftWidth-10,43,5,color);
  tft->setTextColor(color);
  tft->setTextSize(FM);
  tft->drawCentreString("Skip",tftWidth/6,tftHeight+4,1);
  tft->drawCentreString("LAUNCHER",tftWidth/2,tftHeight+4,1);
  tft->drawCentreString("Skip",5*tftWidth/6,tftHeight+4,1);
}

/***************************************************************************************
** Function name: BootScreen
** Description:   Start Display functions and display bootscreen
***************************************************************************************/
void initDisplay(bool doAll) {
  #ifndef HEADLESS
    static uint8_t _name=random(0,3);
    String name="@Pirata";
    String txt;
    int cor, _x, _y, show;

    #ifdef E_PAPER_DISPLAY // epaper display draws only once
      static bool runOnce=false;
      if(runOnce) goto END;
      else runOnce=true;
      tft->stopCallback();
    #endif

    if(_name == 1) name="u/bmorcelli";
    else if(_name == 2) name="gh/bmorcelli";
    tft->drawRoundRect(3,3,tftWidth-6,tftHeight-6,5,FGCOLOR);
    tft->setTextSize(FP);
    tft->setCursor(10,10);
    cor = 0;
    show = random(0,40);
    _x=tft->getCursorX();
    _y=tft->getCursorY();
    
    while(tft->getCursorY()<(tftHeight-(LH+4))) {
      cor = random(0,11);
      tft->setTextSize(FP);
      show = random(0,40);
      if(show==0 || doAll) {
        if (cor==10) { txt=" "; }
        else if (cor & 1) { tft->setTextColor(odd_color,BGCOLOR); txt=String(cor); }
        else { tft->setTextColor(even_color,BGCOLOR); txt=String(cor); }
        
        if(_x>=(tftWidth-(LW+4))) {_x=10; _y+=LH; }
        else if(_x<10) { _x = 10; }
        if(_y>=(tftHeight-(LH+LH/2))) break;
        tft->setCursor(_x,_y);
        if(_y>(tftHeight-(LH*FM+LH/2)) && _x>=(tftWidth-((LW+4)+LW*name.length()))) {
          tft->setTextColor(FGCOLOR);
          tft->print(name);
          _x+=LW*name.length();
        }
        else {
          tft->print(txt);
          _x+=LW;
        }
      } else { 
        if(_y>(tftHeight-(LH*FM+LH/2)) && _x>=(tftWidth-((LW+4)+LW*name.length()))) _x+=LW*name.length();
        else _x+=LW;
        
        if(_x>=(tftWidth-(LW+4))) { _x=10; _y+=LH; }
        }
        tft->setCursor(_x,_y); 
    }
    tft->setTextSize(FG);
    tft->setTextColor(FGCOLOR);
  #if TFT_HEIGHT>200 
    tft->drawCentreString("Launcher",tftWidth/2,tftHeight/2-10,1);
  #else 
    tft->drawCentreString("Launcher",tftWidth/2,tftHeight/2-10,1);
  #endif
    tft->setTextSize(FG);
    tft->setTextColor(FGCOLOR);
  
  #ifdef E_PAPER_DISPLAY // epaper display draws only once
    tft->display(false);
    tft->startCallback();
  #endif

  END:
    delay(50);
    #endif
}
/***************************************************************************************
** Function name: initDisplayLoop
** Description:   Start Display functions and display bootscreen
***************************************************************************************/
void initDisplayLoop() {
  tft->fillScreen(BGCOLOR);
  initDisplay(true);
  delay(250);
  while(!check(AnyKeyPress)){
    initDisplay();
    delay(50);
  }
  returnToMenu=true;
}

/***************************************************************************************
** Function name: displayCurrentItem
** Description:   Display Item on Screen before instalation
***************************************************************************************/
void displayCurrentItem(JsonDocument doc, int currentIndex) {
  #ifdef E_PAPER_DISPLAY
    tft->stopCallback();
  #endif
  JsonObject item = doc[currentIndex];

  const char* name = item["name"];
  const char* author = item["author"];

  //tft->fillScreen(BGCOLOR);
  tft->fillRect(0,tftHeight-5,tftWidth,5,BGCOLOR);
  tft->drawRoundRect(5,5,tftWidth-10,tftHeight-10,5,FGCOLOR);
  tft->fillRoundRect(6,6,tftWidth-12,tftHeight-12,5,BGCOLOR);

  setTftDisplay(10, 10, FGCOLOR,FP);
  tft->print("Firmware: ");
  
  setTftDisplay(10, 22, ~BGCOLOR, FM,BGCOLOR);
  String name2 = String(name);
  tftprintln(name2,10,3);
  
  setTftDisplay(10, 22+4*FM*8, FGCOLOR);
  tft->print("by: ");
  tft->setTextColor(~BGCOLOR);
  String author2 = String(author).substring(0,14);
  tftprintln(author2,10);

  tft->setTextColor(FGCOLOR);
  tft->setTextSize(FM);
  tft->drawChar2(10, tftHeight-(10+FM*9),'<', FGCOLOR,BGCOLOR);
  tft->drawChar2(tftWidth-(10+FM*6), tftHeight-(10+FM*9),'>', FGCOLOR,BGCOLOR);
  tft->setTextSize(FP);
  #if TFT_HEIGHT>200
  String texto = "More information";
  
  tft->setTextColor(FGCOLOR);
  tft->drawCentreString(texto,tftWidth/2,tftHeight-(10+FM*9),1);

  texto = String(currentIndex+1) + " of " + String(doc.size());
  tft->drawCentreString(texto,tftWidth/2,tftHeight-(2+FM*9),1);
  tft->drawRoundRect(tftWidth/2 - (6*11), tftHeight-(10+FM*10), 12*11,19,3,FGCOLOR);
  #else

  String texto = String(currentIndex+1) + " of " + String(doc.size());
  setTftDisplay(int(tftWidth/2 - 3*texto.length()), tftHeight-(10+FM*6), FGCOLOR, FP,BGCOLOR);
  tft->println(texto);
  #endif

  #if defined(HAS_TOUCH)
  TouchFooter();
  #endif  

  int bar = int(tftWidth/(doc.size()));
  if (bar<5) bar = 5;
  tft->fillRect((tftWidth*currentIndex)/doc.size(),tftHeight-5,bar,5,FGCOLOR);

  #ifdef E_PAPER_DISPLAY
    tft->display(false);
    tft->startCallback();
  #endif

}

/***************************************************************************************
** Function name: displayCurrentVersion
** Description:   Display Version on Screen before instalation
***************************************************************************************/
void displayCurrentVersion(String name, String author, String version, String published_at, int versionIndex, JsonArray versions) {
  #ifdef E_PAPER_DISPLAY
    tft->stopCallback();
  #endif
    //tft->fillScreen(BGCOLOR);
    tft->fillRect(0,tftHeight-5,tftWidth,5,BGCOLOR);
    tft->drawRoundRect(5,5,tftWidth-10,tftHeight-10,5,FGCOLOR);
    tft->fillRoundRect(6,6,tftWidth-12,tftHeight-12,5,BGCOLOR);

    setTftDisplay(10, 10, ~BGCOLOR,FM,BGCOLOR);
    String name2 = String(name);
    tftprintln(name2,10,2);
    #if TFT_HEIGHT>200
    setTftDisplay(10,50,ALCOLOR,FM);
    #endif
    tft->print("by: ");
    tft->setTextColor(~BGCOLOR);
    tft->println(String(author).substring(0,14));
    
    tft->setTextColor(ALCOLOR);
    tft->setCursor(10,tft->getCursorY());
    tft->print("v: ");
    tft->setTextColor(~BGCOLOR);
    tft->println(String(version).substring(0,15));
    
    tft->setTextColor(ALCOLOR);
    tft->setCursor(10,tft->getCursorY());
    tft->print("from: ");
    tft->setTextColor(~BGCOLOR);
    tft->println(String(published_at));

    if(versions.size()>1) {
        tft->setTextColor(ALCOLOR);
        tft->drawChar2(10, tftHeight-(10+FM*9),'<', FGCOLOR,BGCOLOR);
        tft->drawChar2(tftWidth-(10+FM*6), tftHeight-(10+FM*9),'>', FGCOLOR,BGCOLOR);
        tft->setTextColor(~BGCOLOR);
    }

    setTftDisplay(-1, -1,ALCOLOR,FM,BGCOLOR);
    tft->drawCentreString("Options",tftWidth/2,tftHeight-(10+FM*9),1);
    tft->drawRoundRect(tftWidth/2 - 3*FM*11, tftHeight-(12+FM*9), FM*6*11,FM*8+3,3,ALCOLOR);

    int div = versions.size();
    if(div==0) div = 1;

    #if defined(HAS_TOUCH)
    TouchFooter(ALCOLOR);
    #endif

    int bar = int(tftWidth/div);
    if (bar<5) bar = 5;
    tft->fillRect((tftWidth*versionIndex)/div,tftHeight-5,bar,5,ALCOLOR);

    #ifdef E_PAPER_DISPLAY
    tft->display(false);
    tft->startCallback();
    #endif
}

/***************************************************************************************
** Function name: displayRedStripe
** Description:   Display Red Stripe with information
***************************************************************************************/
void displayRedStripe(String text, uint16_t fgcolor, uint16_t bgcolor) {
  //save tft settings before showing the stripe
  int _size = tft->getTextsize();
  int _x = tft->getCursorX();
  int _y = tft->getCursorY();
  uint16_t _color = tft->getTextcolor();
  uint16_t _bgcolor = tft->getTextbgcolor();

  //stripe drwawing
  int size;
  if(text.length()*LW*FM<(tft->width()-2*FM*LW)) size = FM;
  else size = FP;
  tft->fillRoundRect(10,tftHeight/2-13,tftWidth-20,26,7,bgcolor);
  tft->setTextColor(fgcolor,bgcolor);
  if(size==FM) { 
    tft->setTextSize(FM); 
    tft->setCursor(tftWidth/2 - FM*3*text.length(), tftHeight/2-8);
  }
  else {
    tft->setTextSize(FP);
    tft->setCursor(tftWidth/2 - FP*3*text.length(), tftHeight/2-8);
  } 
  tft->println(text);

  //return previous tft settings
  tft->setTextSize(_size);
  tft->setTextColor(_color, _bgcolor);
  tft->setCursor(_x,_y);    

}

/***************************************************************************************
** Function name: progressHandler
** Description:   Função para manipular o progresso da atualização
** Dependencia: prog_handler =>>    0 - Flash, 1 - SPIFFS
***************************************************************************************/
void progressHandler(int progress, size_t total) {

  int barWidth = map(progress, 0, total, 0, tftWidth-40);
  if(progress == 0) {
    tft->setTextSize(FM);
    tft->setTextColor(ALCOLOR);
    tft->fillRoundRect(6,6,tftWidth-12,tftHeight-12,5,BGCOLOR);
#if TFT_HEIGHT>200
    tft->drawCentreString("-=Launcher=-",tftWidth/2,20,1);   
#else
    tft->drawCentreString("-=Launcher=-",tftWidth/2,10,1); 
#endif
    tft->drawRoundRect(5, 5, tftWidth - 10, tftHeight - 10, 5, FGCOLOR);
    if (prog_handler == 1) { 
      tft->drawRect(18, tftHeight - 28, tftWidth-36, 17, ALCOLOR);
      tft->fillRect(20, tftHeight - 26, tftWidth-40, 13, BGCOLOR);
      }
    else tft->drawRect(18, tftHeight - 47, tftWidth-36, 17, FGCOLOR);
    
    String txt;
    switch(prog_handler) {
      case 0: 
          txt = "Installing FW";
          break;
      case 1: 
          txt = "Installing SPIFFS";
          break;
      case 2: 
          txt = "Downloading";
          break;          
    }
    displayRedStripe(txt);
  }
  
  if (prog_handler == 1) tft->fillRect(20, tftHeight - 26, barWidth, 13, ALCOLOR);
  else tft->fillRect(20, tftHeight - 45, barWidth, 13, FGCOLOR);

}



/***************************************************************************************
** Function name: drawOptions
** Description:   Função para desenhar e mostrar as opçoes de contexto
***************************************************************************************/
#ifdef E_PAPER_DISPLAY
  #define MAX_MENU_SIZE 13
  #define FONT_S (FM*(LH+3)+4)
#else
  #define FONT_S (FM*LH+4)
  #define MAX_MENU_SIZE (int)(tftHeight/25)
#endif
Opt_Coord drawOptions(int index,const std::vector<std::pair<std::string, std::function<void()>>>& options, uint16_t fgcolor, uint16_t bgcolor) {
  #ifdef E_PAPER_DISPLAY
    tft->stopCallback();
  #endif
    Opt_Coord coord;
    int menuSize = options.size();
    if(options.size()>MAX_MENU_SIZE) { 
      menuSize = MAX_MENU_SIZE; 
      } 

    if(index==0) tft->fillRoundRect(tftWidth*0.10,tftHeight/2-menuSize*FONT_S/2 -5,tftWidth*0.8,FONT_S*menuSize+10,5,bgcolor);
    
    tft->setTextColor(fgcolor,bgcolor);
    tft->setTextSize(FM);
    tft->setCursor(tftWidth*0.10+5,tftHeight/2-menuSize*FONT_S/2);
    
    int i=0;
    int init = 0;
    int cont = 1;
    if(index==0) tft->fillRoundRect(tftWidth*0.10,tftHeight/2-menuSize*FONT_S/2 -5,tftWidth*0.8,FONT_S*menuSize+10,5,bgcolor);
    menuSize = options.size();
    if(index>=MAX_MENU_SIZE) init=index-MAX_MENU_SIZE+1;
    for(i=0;i<menuSize;i++) {
      if(i>=init) {
        String text="";
        if(i==index) { 
          text+=">";
          coord.x=tftWidth*0.10+5+FM*LW;
          coord.y=tft->getCursorY()+4;
          coord.size=(tftWidth*0.8 - 10)/(LW*FM) - 1;
          coord.fgcolor=fgcolor;
          coord.bgcolor=bgcolor;
        }
        else text +=" ";
        text += String(options[i].first.c_str()) + "              ";
        tft->setCursor(tftWidth*0.10+5,tft->getCursorY()+4);
        tft->println(text.substring(0,(tftWidth*0.8 - 10)/(LW*FM) - 1));  
        cont++;
      }
      if(cont>MAX_MENU_SIZE) goto Exit;
    }
    Exit:
    if(options.size()>MAX_MENU_SIZE) menuSize = MAX_MENU_SIZE;
    tft->drawRoundRect(tftWidth*0.10,tftHeight/2-menuSize*FONT_S/2 -5,tftWidth*0.8,FONT_S*menuSize+10,5,fgcolor);
  #ifdef E_PAPER_DISPLAY
    tft->display(false);
    tft->startCallback();
  #endif
    return coord;
}

/***************************************************************************************
** Function name: drawMainMenu
** Description:   Função para desenhar e mostrar o menu principal
***************************************************************************************/
void drawMainMenu(int index) {
  #ifdef E_PAPER_DISPLAY
    tft->stopCallback();
    tft->setFullWindow();
  #endif
  int offset=0;
  if(index>2) offset=index-2;
  const int border = 10;
  bool usb_av = false;
  #ifdef ARDUINO_USB_MODE
  if(sdcardMounted) usb_av = true;
  #endif
  const uint16_t colors[5] = {
      static_cast<uint16_t>(sdcardMounted ? FGCOLOR : DARKGREY), 
      static_cast<uint16_t>(!stopOta ? FGCOLOR : DARKGREY), 
      static_cast<uint16_t>(FGCOLOR),
      static_cast<uint16_t>(usb_av ? FGCOLOR : DARKGREY),
      static_cast<uint16_t>(FGCOLOR)
  };

    const char* texts[5] = { "SD", "OTA", "WUI", "USB", "CFG" };
#if TFT_HEIGHT<200
    const char* messages[5] = { "Launch from SDCard", 
                                "Online Installer", 
                                "Start WebUI",
                                "SD->USB Interface",
                                "Launcher settings." };
#else
    const char* messages[5] = { "Launch from or mng SDCard", 
                                "Install/download from M5Burner", 
                                "Start Web User Interface",
                                "Manage SD files through USB",
                                "Change Launcher settings." };
#endif
    tft->fillRoundRect(6,26,tftWidth-12,tftHeight-12,5,BGCOLOR);
    setTftDisplay(12, 12, FGCOLOR, 1, BGCOLOR);
    
#if TFT_HEIGHT<200
    tft->print("Launcher " + String(LAUNCHER));
    tft->setTextSize(FM);
#else
    tft->drawString("Launcher " + String(LAUNCHER),12,12);
    tft->setTextSize(FG);
#endif
    for (int i = 0; i < 3; ++i) {
        int x = border / 2 + i * ((tftWidth-20) / 3) + 10;
        int y = 20 + border + 10;
        int w = (tftWidth-20) / 3 - border;
        int h = (tftHeight-20) / 2;
        drawSection(x, y, w, h, colors[i+offset], texts[i+offset], index == (i + offset));
    }

    setTftDisplay(-1, -1, FGCOLOR, 1, BGCOLOR);

#if TFT_WIDTH<130
    tft->drawCentreString(messages[index], tftWidth / 2, 28, 1);
#else
    tft->drawCentreString(messages[index], tftWidth / 2, tftHeight - 25, 1);
#endif

    drawDeviceBorder();
    int bat = getBattery();
    if(bat>0) drawBatteryStatus(bat);
  #ifdef E_PAPER_DISPLAY
    tft->display(false);
    tft->startCallback();
  #endif
}

void drawSection(int x, int y, int w, int h, uint16_t color, const char* text, bool isSelected) {
    tft->setTextColor(isSelected ? BGCOLOR : color);
    if (isSelected) {
        tft->fillRoundRect(x+5, y+5, w, h, 5,  color + 0x2222);
        tft->fillRoundRect(x, y, w, h, 5, color);
    }
    tft->drawRoundRect(x, y, w, h, 5, color);
#if TFT_HEIGHT<200
    tft->drawCentreString(text,x + w/2, y + h/2 - 6, 1);
  #else
    tft->drawCentreString(text, x + w/2, y + h/2 - 12, 1);
#endif
}

void drawDeviceBorder() {
    tft->drawRoundRect(5, 5, tftWidth - 10, tftHeight - 10, 5, FGCOLOR);
    tft->drawLine(5, 25, tftWidth - 6, 25, FGCOLOR);
}


void drawBatteryStatus(uint8_t bat) {
    tft->drawRoundRect(tftWidth - 42, 7, 34, 17, 2, FGCOLOR);
    tft->setTextSize(FP);
    tft->setTextColor(FGCOLOR, BGCOLOR); 
  #if TFT_HEIGHT>140 // Excludes Marauder Mini
    tft->drawRightString("  " + String(bat) + "%", tftWidth - 45, 12, 1);
  #endif
    tft->fillRoundRect(tftWidth - 40, 9, 30, 13, 2, BGCOLOR);
    tft->fillRoundRect(tftWidth - 40, 9, 30 * bat / 100, 13, 2, FGCOLOR);
    tft->drawLine(tftWidth - 30, 9, tftWidth - 30, 9 + 13, BGCOLOR);
    tft->drawLine(tftWidth - 20, 9, tftWidth - 20, 9 + 13, BGCOLOR);
}


/***************************************************************************************
** Function name: listFiles
** Description:   Função para desenhar e mostrar o menu principal
***************************************************************************************/
#ifdef E_PAPER_DISPLAY
  #define MAX_ITEMS 14
#else
  #define MAX_ITEMS (int)(tftHeight-20)/(LH*FM)
#endif
Opt_Coord listFiles(int index, String fileList[][3]) {
  #ifdef E_PAPER_DISPLAY
    tft->stopCallback();
  #endif
    Opt_Coord coord;
    tft->setCursor(10,10);
    tft->setTextSize(FM);
    int i=0;
    int arraySize = 0;
    while(fileList[arraySize][2]!="" && arraySize < MAXFILES) arraySize++;
    int start=0;
    if(index>=MAX_ITEMS) {
        start=index-MAX_ITEMS+1;
        if(start<0) start=0;
    }
    int nchars = (tftWidth-20)/(6*tft->getTextsize());
    String txt=">";
    int j=0;
    while(i<arraySize) {
        if(i>=start && fileList[i][2]!="") {
            tft->setCursor(10,tft->getCursorY());
            if(fileList[i][2]=="folder") tft->setTextColor(FGCOLOR-0x1111, BGCOLOR);
            else if(fileList[i][2]=="operator") tft->setTextColor(ALCOLOR, BGCOLOR);
            else { tft->setTextColor(FGCOLOR,BGCOLOR); }

            if (index==i) { 
              txt=">";
              coord.x=10+FM*LW;
              coord.y=tft->getCursorY();
              coord.size=nchars;
              coord.fgcolor=fileList[i][2]=="folder"? FGCOLOR-0x1111:FGCOLOR;
              coord.bgcolor=BGCOLOR;
            }
            else txt=" ";
            txt+=fileList[i][0] + "                       ";
            tft->println(txt.substring(0,nchars));
            j++;
        }
        i++;
        if (i==(start+MAX_ITEMS) || fileList[i][2]=="") break;
    }

    #if defined(HAS_TOUCH)
    TouchFooter();
    #endif
    #ifdef E_PAPER_DISPLAY
    tft->display(false);
    tft->startCallback();
    #endif
    return coord;
}


/*********************************************************************
**  Function: loopOptions                             
**  Where you choose among the options in menu
**********************************************************************/
void loopOptions(const std::vector<std::pair<std::string, std::function<void()>>>& options, bool bright){
  bool redraw = true;
  int index = 0;
  log_i("Number of options: %d", options.size());
  int numOpt = options.size()-1;
  Opt_Coord coord;
  coord=drawOptions(0,options, ALCOLOR, BGCOLOR);
  LongPressTmp=millis();
  while(1){
    if (redraw) { 
      coord=drawOptions(index,options, ALCOLOR, BGCOLOR);
      if(bright){
        setBrightness(100*(numOpt-index)/numOpt,false);
      }
      redraw=false;
      #ifdef E_PAPER_DISPLAY
      tft->display(false);
      delay(200);
      #endif
    }
    String txt=options[index].first.c_str();
    displayScrollingText(txt, coord);

    #if defined(T_EMBED) || defined(HAS_TOUCH) || defined(HAS_KEYBOARD)
    if(check(PrevPress) || check(UpPress)) {
      if(index==0) index = options.size() - 1;
      else if(index>0) index--;
      redraw = true;
    #else
    if(LongPress || PrevPress) {
      if(!LongPress) {
        LongPress = true;
        LongPressTmp = millis();
      }
      if(LongPress && millis()-LongPressTmp<700) { 
        if(!PrevPress) {
          AnyKeyPress=false;
          if(index==0) index = options.size() - 1;
          else if(index>0) index--;
          LongPress=false;
          redraw = true;
        }
        if(millis()-LongPressTmp>200) tft->drawArc(tftWidth/2, tftHeight/2, 25,15,0,360*(millis()-(LongPressTmp+200))/500,FGCOLOR-0x1111);
        if(millis()-LongPressTmp>700) { // longpress detected to exit
          break;
        } else goto WAITING;

      }
    #endif
    }
    WAITING:
    /* DW Btn to next item */
    if(check(NextPress) || check(DownPress)) { 
      index++;
      if((index+1)>options.size()) index = 0;
      redraw = true;
    }

    /* Select and run function */
    if(check(SelPress)) { 
      options[index].second();
      break;
    }

    #if defined(T_EMBED) || defined(HAS_TOUCH) || defined(HAS_KEYBOARD)
    if(check(EscPress)) break;
    #endif
  }
}

/*********************************************************************
**  Function: loopVersions                             
**  Where you choose which version to install/download **
**********************************************************************/
void loopVersions() {
  JsonObject item = doc[currentIndex];
  
  int versionIndex = 0;
  const char* name = item["name"];
  const char* author = item["author"];
  JsonArray versions = item["versions"];
  bool redraw = true;
  
  LongPressTmp=millis();
  while(1) {
    if(returnToMenu) break; // Stops the loop to get back to Main menu

    JsonObject Version = versions[versionIndex];
    const char* version = Version["version"];
    const char* published_at = Version["published_at"];
    const char* file = Version["file"];  
    bool spiffs = Version["s"].as<bool>();
    bool fat = Version["f"].as<bool>();
    bool fat2 = Version["f2"].as<bool>();
    bool nb = Version["nb"].as<bool>();
    uint32_t app_size = Version["as"].as<uint32_t>();
    uint32_t spiffs_size = Version["ss"].as<uint32_t>();
    uint32_t spiffs_offset = Version["so"].as<uint32_t>();
    uint32_t FAT_size[2] = {0,0,};
    uint32_t FAT_offset[2] = {0,0,}; 
    if(fat) { FAT_size[0] = Version["fs"].as<uint32_t>(); FAT_offset[0] = Version["fo"].as<uint32_t>(); }
    if(fat2) { FAT_size[1] = Version["fs2"].as<uint32_t>(); FAT_offset[1] = Version["fo2"].as<uint32_t>(); }
    if(redraw){
      
      displayCurrentVersion(String(name), String(author), String(version), String(published_at), versionIndex, versions);
      redraw = false;
      #ifdef E_PAPER_DISPLAY
        tft->display(false); 
        delay(200);
      #endif
    }
    /* DW Btn to next item */
    if(check(NextPress)) { 
      versionIndex++;
      if(versionIndex>versions.size()-1) versionIndex = 0;
      redraw = true;
    }

    /* UP Btn go back to FW menu and ´<´ go to previous version item */
    
    #if defined(T_EMBED) || defined(HAS_TOUCH) || defined(HAS_KEYBOARD)
        /* UP Btn go to previous item */
    if(check(PrevPress)) { 
      versionIndex--;
      if(versionIndex<0) versionIndex = versions.size()-1;
      redraw = true;
    }

    if(check(EscPress)) {
      goto SAIR;
    } 
    #else // Esc logic is holding previous btn fot 1 second +-
    if(LongPress || PrevPress) {
      if(!LongPress) {
        LongPress = true;
        LongPressTmp = millis();
      }
      if(LongPress && millis()-LongPressTmp<800) { 
        WAITING:
        vTaskDelay(10/portTICK_PERIOD_MS);
        if(!PrevPress && millis()-LongPressTmp<200) {
          AnyKeyPress=false;
          if(versionIndex==0) versionIndex = versions.size() - 1;
          else if(versionIndex>0) versionIndex--;
          LongPress=false;
          redraw = true;
        }
        if(!PrevPress && millis()-LongPressTmp>200) {
          check(PrevPress);
          redraw=true;
          LongPress=false;
          goto EXIT_CHECK;
        }
        if(millis()-LongPressTmp>200) tft->drawArc(tftWidth/2, tftHeight/2, 25,15,0,360*(millis()-(LongPressTmp+200))/500,FGCOLOR-0x1111);
        if(millis()-LongPressTmp>700) { // longpress detected to exit
          returnToMenu=true;
          check(PrevPress);
          goto SAIR;
        } else goto WAITING;
      }
      EXIT_CHECK:
      yield();
    }
    
    #endif

    /* Select to install */
    if(check(SelPress)) { 

      // Definição da matriz "Options"
      options = {
          {"OTA Install", [=]() { installFirmware(String(file), app_size, spiffs, spiffs_offset, spiffs_size, nb, fat, (uint32_t*)FAT_offset, (uint32_t*)FAT_size); }},
          {"Download->SD", [=]() { downloadFirmware(String(file), String(name) + "." + String(version).substring(0,10), dwn_path); }},
          {"Back to List", [=]() { returnToMenu=true; }},
      };
      loopOptions(options);
      // On fail installing will run the following line
      redraw = true;
    }

  }
  Sucesso:
  if(!returnToMenu) esp_restart();
  
  // quando sair, redesenhar a tela
  SAIR:
  if(!returnToMenu) displayCurrentItem(doc, currentIndex);

}


/*********************************************************************
**  Function: loopFirmware                             
**  Where you choose which Firmware to see more data   
**********************************************************************/
void loopFirmware(){  
  LongPressTmp=millis();
  currentIndex=0;
  displayCurrentItem(doc, currentIndex);

  while(1){
    if (WiFi.status() == WL_CONNECTED) {
    /* UP Btn go to previous item */
      if(check(PrevPress)) {
        if(currentIndex==0) currentIndex = doc.size() - 1;
        else if(currentIndex>0) currentIndex--;
        displayCurrentItem(doc, currentIndex);
        #ifdef E_PAPER_DISPLAY
        tft->display(false);
        delay(200);
        #endif
      }
      /* DW Btn to next item */
      if(check(NextPress)) { 
        currentIndex++;
        if((currentIndex+1)>doc.size()) currentIndex = 0;
        displayCurrentItem(doc, currentIndex);
        #ifdef E_PAPER_DISPLAY	
        tft->display(false);
        delay(200);
        #endif
      }

      
        //Checks for long press to get back to Main Menu, only for StickCs.. Cardputer uses Esc btn
        #if defined(T_EMBED) || defined(HAS_TOUCH) || defined(HAS_KEYBOARD)
        /* Select to install */
        if(check(SelPress)) { 
          loopVersions();
          delay(200);
          returnToMenu=false;
        }
        #else
        if(LongPress || SelPress) {
          if(!LongPress) {
            LongPress = true;
            LongPressTmp = millis();
          }
          if(LongPress && millis()-LongPressTmp<250) goto WAITING;
          LongPress=false;
          if(check(SelPress)) {
            bool exit=false;
            options = {
              {"View version",[=](){ loopVersions(); }},
              {"Main Menu",   [&](){ exit=true;}}
            };
            loopOptions(options);
            returnToMenu=false;
            if(exit) {
              returnToMenu=true;
              goto END;
            }
            displayCurrentItem(doc, currentIndex);
            delay(200);
          } else {
            check(SelPress);
            loopVersions();          // goes to the Version information
            displayCurrentItem(doc, currentIndex);
            delay(200);
            returnToMenu=false;
          }
        }
      WAITING:
      yield();
      #endif

      #if defined(T_EMBED) || defined(HAS_TOUCH) || defined(HAS_KEYBOARD)
      if(check(EscPress)) break; //  Esc btn to get back to Main Menu.
      #endif

      if(returnToMenu) break; // break the loop and gets back to Main Menu

    } 
    else {
      displayRedStripe("WiFi: Disconnected");
      delay(5000);
      break;
    }
  }
  END:
  WiFi.disconnect(true,true);
  WiFi.mode(WIFI_OFF);
  doc.clear();
}

/*********************************************************************
**  Function: tftprintln                             
**  similar to tft->println(), but allows to include margin
**********************************************************************/
void tftprintln(String txt, int margin, int numlines) {
  int size=txt.length();
  if(numlines == 0) numlines = (tftHeight-2*margin) / (tft->getTextsize()*8);
  int nchars = (tftWidth-2*margin)/(6*tft->getTextsize()); // 6 pixels of width fot a letter size 1
  int x = tft->getCursorX();
  int start=0;
  while(size>0 && numlines>0) {
    if(tft->getCursorX()<margin) tft->setCursor(margin,tft->getCursorY());
    nchars = (tftWidth-tft->getCursorX()-margin)/(6*tft->getTextsize()); // 6 pixels of width fot a letter size 1
    tft->println(txt.substring(0,nchars));
    txt=txt.substring(nchars);
    size -= nchars;
    numlines--;
  }
}
/*********************************************************************
**  Function: tftprintln                             
**  similar to tft->println(), but allows to include margin
**********************************************************************/
void tftprint(String txt, int margin, int numlines) {
  int size=txt.length();
  if(numlines == 0) numlines = (tftHeight-2*margin) / (tft->getTextsize()*8);
  int nchars = (tftWidth-2*margin)/(6*tft->getTextsize()); // 6 pixels of width fot a letter size 1
  int x = tft->getCursorX();
  int start=0;
  bool prim=true;
  while(size>0 && numlines>0) {
    if(!prim) { tft->println(); }
    if(tft->getCursorX()<margin) tft->setCursor(margin,tft->getCursorY());
    nchars = (tftWidth-tft->getCursorX()-margin)/(6*tft->getTextsize()); // 6 pixels of width fot a letter size 1
    tft->print(txt.substring(0,nchars));
    txt = txt.substring(nchars);
    size -= nchars;
    numlines--;
    prim = false;
  }
}

/***************************************************************************************
** Function name: getComplementaryColor
** Description:   Get simple complementary color in RGB565 format
***************************************************************************************/
uint16_t getComplementaryColor(uint16_t color) {
  int r = 31-((color >> 11) & 0x1F);
  int g = 63-((color >> 5) & 0x3F);
  int b = 31-(color & 0x1F);
  return (r<<11) | (g<<5) | b;
}