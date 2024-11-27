
#include "display.h"
#include "globals.h"
#include "mykeyboard.h"
#include "onlineLauncher.h"
#include "sd_functions.h"
#include "settings.h"

#if defined(HEADLESS)
SerialDisplayClass tft;
#else
TFT_eSPI tft = TFT_eSPI();         // Invoke custom library
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
  tft.setTextColor(coord.fgcolor,coord.bgcolor);
  if (len < coord.size) {
    // Text fits within limit, no scrolling needed
    return;
  } else if(millis()>_lastmillis+200) {
    String scrollingPart = displayText.substring(i, i + (coord.size - 1)); // Display charLimit characters at a time
    tft.fillRect(coord.x, coord.y, (coord.size-1) * LW * tft.textsize, LH * tft.textsize, BGCOLOR); // Clear display area
    tft.setCursor(coord.x, coord.y);
    tft.setCursor(coord.x, coord.y);
    tft.print(scrollingPart);
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
    tft.setCursor(x,y);
    tft.fillScreen(screen);
    tft.setTextSize(size);
    tft.setTextColor(fc,bg);
}

/***************************************************************************************
** Function name: setTftDisplay
** Description:   set cursor, font color, size and bg font color
***************************************************************************************/
void setTftDisplay(int x, int y, uint16_t fc, int size, uint16_t bg) {
    if (x>=0 && y<0)        tft.setCursor(x,tft.getCursorY());          // if -1 on x, sets only y
    else if (x<0 && y>=0)   tft.setCursor(tft.getCursorX(),y);          // if -1 on y, sets only x
    else if (x>=0 && y>=0)  tft.setCursor(x,y);                         // if x and y > 0, sets both
    tft.setTextSize(size);
    tft.setTextColor(fc,bg);
}

/***************************************************************************************
** Function name: TouchFooter
** Description:   Draw touch screen footer
***************************************************************************************/
void TouchFooter(uint16_t color) {
  tft.drawRoundRect(5,HEIGHT+2,WIDTH-10,43,5,color);
  tft.setTextColor(color);
  tft.setTextSize(FM);
  tft.drawCentreString("PREV",WIDTH/6,HEIGHT+4,1);
  tft.drawCentreString("SEL",WIDTH/2,HEIGHT+4,1);
  tft.drawCentreString("NEXT",5*WIDTH/6,HEIGHT+4,1);
}

/***************************************************************************************
** Function name: TouchFooter
** Description:   Draw touch screen footer
***************************************************************************************/
void TouchFooter2(uint16_t color) {
  tft.drawRoundRect(5,HEIGHT+2,WIDTH-10,43,5,color);
  tft.setTextColor(color);
  tft.setTextSize(FM);
  tft.drawCentreString("Skip",WIDTH/6,HEIGHT+4,1);
  tft.drawCentreString("LAUNCHER",WIDTH/2,HEIGHT+4,1);
  tft.drawCentreString("Skip",5*WIDTH/6,HEIGHT+4,1);
}

/***************************************************************************************
** Function name: BootScreen
** Description:   Start Display functions and display bootscreen
***************************************************************************************/
void initDisplay(bool doAll) {
    tft.drawRoundRect(3,3,WIDTH-6,HEIGHT-6,5,FGCOLOR);
    tft.setTextSize(FONT_P);
    tft.setCursor(10,10);
    int cor = 0;
    String txt;
    int show = random(0,40);
    int _x=tft.getCursorX();
    int _y=tft.getCursorY();
    
    while(tft.getCursorY()<(HEIGHT-12)) {
      cor = random(0,11);
      tft.setTextSize(FONT_P);
      show = random(0,40);
      if(show==0 || doAll) {
        if (cor==10) { txt=" "; }
        else if (cor & 1) { tft.setTextColor(odd_color,BGCOLOR); txt=String(cor); }
        else { tft.setTextColor(even_color,BGCOLOR); txt=String(cor); }
        
        if(_x>=(WIDTH-10)) {_x=10; _y+=8; }
        else if(_x<10) { _x = 10; }
        if(_y>=(HEIGHT-12)) break;
        tft.setCursor(_x,_y);
        if(_y>(HEIGHT-20) && _x>=(WIDTH-(10+LW*7))) {
          tft.setTextColor(FGCOLOR);
          tft.print("@Pirata");
          _x+=42;
        }
        else {
          tft.print(txt);
          _x+=6;
        }
      } else { 
        if(_y>(HEIGHT-20) && _x>=(WIDTH-(10+LW*7))) _x+=42;
        else _x+=6;
        
        if(_x>=(WIDTH-10)) { _x=10; _y+=8; }
        }
        tft.setCursor(_x,_y); 
    }
    tft.setTextSize(FONT_G);
    tft.setTextColor(FGCOLOR);
    #if WIDTH>200
    tft.drawCentreString("Launcher",WIDTH/2,HEIGHT/2-10,1); //SMOOTH_FONT
    #else
    tft.drawCentreString("Launcher",WIDTH/2,HEIGHT/2-10,SMOOTH_FONT); //SMOOTH_FONT
    #endif
    tft.setTextSize(FONT_G);
    tft.setTextColor(FGCOLOR);

    delay(50);
    
}

/***************************************************************************************
** Function name: displayCurrentItem
** Description:   Display Item on Screen before instalation
***************************************************************************************/
void displayCurrentItem(JsonDocument doc, int currentIndex) {
  JsonObject item = doc[currentIndex];

  const char* name = item["name"];
  const char* author = item["author"];

  //tft.fillScreen(BGCOLOR);
  tft.fillRect(0,HEIGHT-5,WIDTH,5,BGCOLOR);
  tft.drawRoundRect(5,5,WIDTH-10,HEIGHT-10,5,FGCOLOR);
  tft.fillRoundRect(6,6,WIDTH-12,HEIGHT-12,5,BGCOLOR);

  setTftDisplay(10, 10, FGCOLOR,FONT_P);
  tft.print("Firmware: ");
  
  setTftDisplay(10, 22, ~BGCOLOR, FONT_M,BGCOLOR);
  String name2 = String(name);
  tftprintln(name2,10,3);
  
  setTftDisplay(10, 22+4*FONT_M*8, FGCOLOR);
  tft.print("by: ");
  tft.setTextColor(~BGCOLOR);
  String author2 = String(author).substring(0,14);
  tftprintln(author2,10);

  tft.setTextColor(FGCOLOR);
  tft.setTextSize(FONT_M);
  tft.drawChar('<', 10, HEIGHT-(10+FONT_M*9));
  tft.drawChar('>', WIDTH-(10+FONT_M*6), HEIGHT-(10+FONT_M*9));
  tft.setTextSize(FONT_P);
  #if WIDTH>200
  String texto = "More information";
  
  tft.setTextColor(FGCOLOR);
  tft.drawCentreString(texto,WIDTH/2,HEIGHT-(10+FONT_M*9),1);

  texto = String(currentIndex+1) + " of " + String(doc.size());
  tft.drawCentreString(texto,WIDTH/2,HEIGHT-(2+FONT_M*9),1);
  tft.drawRoundRect(WIDTH/2 - (6*11), HEIGHT-(10+FONT_M*10), 12*11,19,3,FGCOLOR);
  #else

  String texto = String(currentIndex+1) + " of " + String(doc.size());
  setTftDisplay(int(WIDTH/2 - 3*texto.length()), HEIGHT-(10+FONT_M*6), FGCOLOR, FONT_P,BGCOLOR);
  tft.println(texto);
  #endif


  

  #if defined(HAS_TOUCH)
  TouchFooter();
  #endif  

  int bar = int(WIDTH/(doc.size()));
  if (bar<5) bar = 5;
  tft.fillRect((WIDTH*currentIndex)/doc.size(),HEIGHT-5,bar,5,FGCOLOR);

}

/***************************************************************************************
** Function name: displayCurrentVersion
** Description:   Display Version on Screen before instalation
***************************************************************************************/
void displayCurrentVersion(String name, String author, String version, String published_at, int versionIndex, JsonArray versions) {
    //tft.fillScreen(BGCOLOR);
    tft.fillRect(0,HEIGHT-5,WIDTH,5,BGCOLOR);
    tft.drawRoundRect(5,5,WIDTH-10,HEIGHT-10,5,FGCOLOR);
    tft.fillSmoothRoundRect(6,6,WIDTH-12,HEIGHT-12,5,BGCOLOR);

    setTftDisplay(10, 10, ~BGCOLOR,FONT_M,BGCOLOR);
    String name2 = String(name);
    tftprintln(name2,10,2);
    #if WIDTH>200
    setTftDisplay(10,50,ALCOLOR,FONT_M);
    #endif
    tft.print("by: ");
    tft.setTextColor(~BGCOLOR);
    tft.println(String(author).substring(0,14));
    
    tft.setTextColor(ALCOLOR);
    tft.setCursor(10,tft.getCursorY());
    tft.print("v: ");
    tft.setTextColor(~BGCOLOR);
    tft.println(String(version).substring(0,15));
    
    tft.setTextColor(ALCOLOR);
    tft.setCursor(10,tft.getCursorY());
    tft.print("from: ");
    tft.setTextColor(~BGCOLOR);
    tft.println(String(published_at));

    if(versions.size()>1) {
        tft.setTextColor(ALCOLOR);
        tft.drawChar('<', 10, HEIGHT-(10+FONT_M*9));
        tft.drawChar('>', WIDTH-(10+FONT_M*6), HEIGHT-(10+FONT_M*9));
        tft.setTextColor(~BGCOLOR);
    }

    setTftDisplay(-1, -1,ALCOLOR,FONT_M,BGCOLOR);
    tft.drawCentreString("Options",WIDTH/2,HEIGHT-(10+FONT_M*9),1);
    tft.drawRoundRect(WIDTH/2 - 3*FONT_M*11, HEIGHT-(12+FONT_M*9), FONT_M*6*11,FONT_M*8+3,3,ALCOLOR);

    int div = versions.size();
    if(div==0) div = 1;

    #if defined(HAS_TOUCH)
    TouchFooter(ALCOLOR);
    #endif

    int bar = int(WIDTH/div);
    if (bar<5) bar = 5;
    tft.fillRect((WIDTH*versionIndex)/div,HEIGHT-5,bar,5,ALCOLOR);
}

/***************************************************************************************
** Function name: displayRedStripe
** Description:   Display Red Stripe with information
***************************************************************************************/
void displayRedStripe(String text, uint16_t fgcolor, uint16_t bgcolor) {
  //save tft settings before showing the stripe
  int _size = tft.textsize;
  int _x = tft.getCursorX();
  int _y = tft.getCursorY();
  uint16_t _color = tft.textcolor;
  uint16_t _bgcolor = tft.textbgcolor;

  //stripe drwawing
  int size;
  if(text.length()*LW*FONT_M<(tft.width()-2*FONT_M*LW)) size = FONT_M;
  else size = FONT_P;
  tft.fillSmoothRoundRect(10,HEIGHT/2-13,WIDTH-20,26,7,bgcolor);
  tft.setTextColor(fgcolor,bgcolor);
  if(size==FONT_M) { 
    tft.setTextSize(FONT_M); 
    tft.setCursor(WIDTH/2 - FONT_M*3*text.length(), HEIGHT/2-8);
  }
  else {
    tft.setTextSize(FONT_P);
    tft.setCursor(WIDTH/2 - FONT_P*3*text.length(), HEIGHT/2-8);
  } 
  tft.println(text);

  //return previous tft settings
  tft.setTextSize(_size);
  tft.setTextColor(_color, _bgcolor);
  tft.setCursor(_x,_y);    

}

/***************************************************************************************
** Function name: progressHandler
** Description:   Função para manipular o progresso da atualização
** Dependencia: prog_handler =>>    0 - Flash, 1 - SPIFFS
***************************************************************************************/
void progressHandler(int progress, size_t total) {
#if WIDTH>200
  int barWidth = map(progress, 0, total, 0, WIDTH-40);
  if(progress == 0) {
    tft.setTextSize(FONT_M);
    tft.setTextColor(ALCOLOR);
    tft.fillSmoothRoundRect(6,6,WIDTH-12,HEIGHT-12,5,BGCOLOR);
    tft.drawCentreString("-=Launcher=-",WIDTH/2,20,1);    
    tft.drawRoundRect(5, 5, WIDTH - 10, HEIGHT - 10, 5, FGCOLOR);
    if (prog_handler == 1) { 
      tft.drawRect(18, HEIGHT - 28, WIDTH-36, 17, ALCOLOR);
      tft.fillRect(20, HEIGHT - 26, WIDTH-40, 13, BGCOLOR);
      }
    else tft.drawRect(18, HEIGHT - 47, WIDTH-36, 17, FGCOLOR);
    
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
  
  if (prog_handler == 1) tft.fillRect(20, HEIGHT - 26, barWidth, 13, ALCOLOR);
  else tft.fillRect(20, HEIGHT - 45, barWidth, 13, FGCOLOR);


#else
  
  int barWidth = map(progress, 0, total, 0, 100);
  if(barWidth <=1) {
    tft.fillSmoothRoundRect(6,6,WIDTH-12,HEIGHT-12,5,BGCOLOR);
    tft.drawRect(6, 6, WIDTH-12, HEIGHT-12, BGCOLOR);
    if (prog_handler == 1) tft.drawRect(28, HEIGHT - 28, 104, 17, ALCOLOR);
    else tft.drawRect(28, HEIGHT - 47, 104, 17, FGCOLOR);
    
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
  if (prog_handler == 1) tft.fillRect(30, HEIGHT - 26, barWidth, 13, ALCOLOR);
  else tft.fillRect(30, HEIGHT - 45, barWidth, 13, FGCOLOR);

#endif

}



/***************************************************************************************
** Function name: drawOptions
** Description:   Função para desenhar e mostrar as opçoes de contexto
***************************************************************************************/
#define MAX_MENU_SIZE (int)(HEIGHT/25)
Opt_Coord drawOptions(int index,const std::vector<std::pair<std::string, std::function<void()>>>& options, uint16_t fgcolor, uint16_t bgcolor) {
    Opt_Coord coord;
    int menuSize = options.size();
    if(options.size()>MAX_MENU_SIZE) { 
      menuSize = MAX_MENU_SIZE; 
      } 

    if(index==0) tft.fillRoundRect(WIDTH*0.10,HEIGHT/2-menuSize*(FONT_M*8+4)/2 -5,WIDTH*0.8,(FONT_M*8+4)*menuSize+10,5,bgcolor);
    
    tft.setTextColor(fgcolor,bgcolor);
    tft.setTextSize(FONT_M);
    tft.setCursor(WIDTH*0.10+5,HEIGHT/2-menuSize*(FONT_M*8+4)/2);
    
    int i=0;
    int init = 0;
    int cont = 1;
    if(index==0) tft.fillRoundRect(WIDTH*0.10,HEIGHT/2-menuSize*(FONT_M*8+4)/2 -5,WIDTH*0.8,(FONT_M*8+4)*menuSize+10,5,bgcolor);
    menuSize = options.size();
    if(index>=MAX_MENU_SIZE) init=index-MAX_MENU_SIZE+1;
    for(i=0;i<menuSize;i++) {
      if(i>=init) {
        String text="";
        if(i==index) { 
          text+=">";
          coord.x=WIDTH*0.10+5+FM*LW;
          coord.y=tft.getCursorY()+4;
          coord.size=(WIDTH*0.8 - 10)/(LW*FONT_M) - 1;
          coord.fgcolor=fgcolor;
          coord.bgcolor=bgcolor;
        }
        else text +=" ";
        text += String(options[i].first.c_str()) + "              ";
        tft.setCursor(WIDTH*0.10+5,tft.getCursorY()+4);
        tft.println(text.substring(0,(WIDTH*0.8 - 10)/(LW*FONT_M) - 1));  
        cont++;
      }
      if(cont>MAX_MENU_SIZE) goto Exit;
    }
    Exit:
    if(options.size()>MAX_MENU_SIZE) menuSize = MAX_MENU_SIZE;
    tft.drawRoundRect(WIDTH*0.10,HEIGHT/2-menuSize*(FONT_M*8+4)/2 -5,WIDTH*0.8,(FONT_M*8+4)*menuSize+10,5,fgcolor);
    return coord;
}

/***************************************************************************************
** Function name: drawMainMenu
** Description:   Função para desenhar e mostrar o menu principal
***************************************************************************************/
void drawMainMenu(int index) {
  int offset=0;
  if(index>2) offset=index-2;
  const int border = 10;
  const uint16_t colors[4] = {
      static_cast<uint16_t>(sdcardMounted ? FGCOLOR : TFT_DARKGREY), 
      static_cast<uint16_t>(!stopOta ? FGCOLOR : TFT_DARKGREY), 
      static_cast<uint16_t>(FGCOLOR),
      static_cast<uint16_t>(FGCOLOR)
  };

    const char* texts[4] = { "SD", "OTA", "WUI","CFG" };
#if WIDTH<200
    const char* messages[4] = { "Launch from SDCard", 
                                "Online Installer", 
                                "Start WebUI",
                                "Launcher settings." };
#else
    const char* messages[4] = { "Launch from or mng SDCard", 
                                "Install/download from M5Burner", 
                                "Start Web User Interface",
                                "Change Launcher settings." };
#endif
    tft.fillSmoothRoundRect(6,26,WIDTH-12,HEIGHT-12,5,BGCOLOR);
    setTftDisplay(12, 12, FGCOLOR, 1, BGCOLOR);
    
#if WIDTH<200
    tft.print("Launcher " + String(LAUNCHER));
    tft.setTextSize(FONT_M);
#else
    tft.print("Launcher " + String(LAUNCHER));
    tft.setTextSize(FONT_G);
#endif
    for (int i = 0; i < 3; ++i) {
        int x = border / 2 + i * ((WIDTH-20) / 3) + 10;
        int y = 20 + border + 10;
        int w = (WIDTH-20) / 3 - border;
        int h = (HEIGHT-20) / 2;
        drawSection(x, y, w, h, colors[i+offset], texts[i+offset], index == (i + offset));
    }

    setTftDisplay(-1, -1, FGCOLOR, 1, BGCOLOR);

#if WIDTH<200
  tft.drawCentreString(messages[index], WIDTH / 2, 18, 1);
#else
    tft.drawCentreString(messages[index], WIDTH / 2, HEIGHT - 25, 1);
#endif

    drawDeviceBorder();
    drawBatteryStatus();
}

void drawSection(int x, int y, int w, int h, uint16_t color, const char* text, bool isSelected) {
    tft.setTextColor(isSelected ? BGCOLOR : color);
    if (isSelected) {
        tft.fillRoundRect(x+5, y+5, w, h, 5,  color + 0x2222);
        tft.fillRoundRect(x, y, w, h, 5, color);
    }
    tft.drawRoundRect(x, y, w, h, 5, color);
#if WIDTH<200
    tft.drawCentreString(text,x + w/2, y + h/2 - 6, SMOOTH_FONT);
  #else
    tft.drawCentreString(text, x + w/2, y + h/2 - 12, 1);
#endif
}

void drawDeviceBorder() {
    tft.drawRoundRect(5, 5, WIDTH - 10, HEIGHT - 10, 5, FGCOLOR);
    tft.drawLine(5, 25, WIDTH - 6, 25, FGCOLOR);
}

void drawBatteryStatus() {
    tft.drawRoundRect(WIDTH - 42, 7, 34, 17, 2, FGCOLOR);
    int bat = getBattery();
    tft.setTextSize(FONT_P);
    tft.setTextColor(FGCOLOR, BGCOLOR);
    tft.drawRightString("  " + String(bat) + "%", WIDTH - 45, 12, 1);
    tft.fillRoundRect(WIDTH - 40, 9, 30, 13, 2, BGCOLOR);
    tft.fillRoundRect(WIDTH - 40, 9, 30 * bat / 100, 13, 2, FGCOLOR);
    tft.drawLine(WIDTH - 30, 9, WIDTH - 30, 9 + 13, BGCOLOR);
    tft.drawLine(WIDTH - 20, 9, WIDTH - 20, 9 + 13, BGCOLOR);
}


/***************************************************************************************
** Function name: listFiles
** Description:   Função para desenhar e mostrar o menu principal
***************************************************************************************/
#define MAX_ITEMS (int)(HEIGHT-20)/(LH*2)
Opt_Coord listFiles(int index, String fileList[][3]) {
    Opt_Coord coord;
    tft.setCursor(10,10);
    tft.setTextSize(FONT_M);
    int i=0;
    int arraySize = 0;
    while(fileList[arraySize][2]!="" && arraySize < MAXFILES) arraySize++;
    int start=0;
    if(index>=MAX_ITEMS) {
        start=index-MAX_ITEMS+1;
        if(start<0) start=0;
    }
    int nchars = (WIDTH-20)/(6*tft.textsize);
    String txt=">";
    int j=0;
    while(i<arraySize) {
        if(i>=start && fileList[i][2]!="") {
            tft.setCursor(10,tft.getCursorY());
            if(fileList[i][2]=="folder") tft.setTextColor(FGCOLOR-0x1111, BGCOLOR);
            else if(fileList[i][2]=="operator") tft.setTextColor(ALCOLOR, BGCOLOR);
            else { tft.setTextColor(FGCOLOR,BGCOLOR); }

            if (index==i) { 
              txt=">";
              coord.x=10+FM*LW;
              coord.y=tft.getCursorY();
              coord.size=nchars;
              coord.fgcolor=fileList[i][2]=="folder"? FGCOLOR-0x1111:FGCOLOR;
              coord.bgcolor=BGCOLOR;
            }
            else txt=" ";
            txt+=fileList[i][0] + "                       ";
            tft.println(txt.substring(0,nchars));
            j++;
        }
        i++;
        if (i==(start+MAX_ITEMS) || fileList[i][2]=="") break;
    }

    #if defined(HAS_TOUCH)
    TouchFooter();
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
  
  while(1){
    if (redraw) { 
      coord=drawOptions(index,options, ALCOLOR, BGCOLOR);
      if(bright){
        setBrightness(100*(numOpt-index)/numOpt,false);
      }
      redraw=false;
      delay(REDRAW_DELAY); 
    }
    String txt=options[index].first.c_str();
    displayScrollingText(txt, coord);

    if(checkPrevPress()) {
    #if defined(ESC_LOGIC)
      if(index==0) index = options.size() - 1;
      else if(index>0) index--;
      redraw = true;
    #else
    long _tmp=millis();
    while(checkPrevPress()) { if(millis()-_tmp>200) tft.drawArc(WIDTH/2, HEIGHT/2, 25,15,0,360*(millis()-(_tmp+200))/500,FGCOLOR-0x1111,BGCOLOR,true); }
    if(millis()-_tmp>700) { // longpress detected to exit
      delay(200);
      break;
    } 
    else {
      if(index==0) index = options.size() - 1;
      else if(index>0) index--;
      redraw = true;
    }
    #endif
    }
    /* DW Btn to next item */
    if(checkNextPress()) { 
      index++;
      if((index+1)>options.size()) index = 0;
      redraw = true;
    }

    /* Select and run function */
    if(checkSelPress()) { 
      options[index].second();
      break;
    }

    #if defined(ESC_LOGIC)
    if(checkEscPress()) break;
    #endif
  }
  delay(200);
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
      delay(REDRAW_DELAY);
    }
    /* DW Btn to next item */
    if(checkNextPress()) { 
      versionIndex++;
      if(versionIndex>versions.size()-1) versionIndex = 0;
      redraw = true;
      delay(REDRAW_DELAY);
    }

    /* UP Btn go back to FW menu and ´<´ go to previous version item */
    
    #ifdef ESC_LOGIC
        /* UP Btn go to previous item */
    if(checkPrevPress()) { 
      versionIndex--;
      if(versionIndex<0) versionIndex = versions.size()-1;
      redraw = true;
      delay(200);
    }

    if(checkEscPress()) {
      delay(200);
      goto SAIR;
    } 
    #else // Esc logic is holding previous btn fot 1 second +-

    if(checkPrevPress()) {
      long _tmp=millis();
      while(checkPrevPress()) { if(millis()-_tmp>200) tft.drawArc(WIDTH/2, HEIGHT/2, 25,15,0,360*(millis()-(_tmp+200))/500,FGCOLOR-0x1111,BGCOLOR,true); }
      if(millis()-_tmp>700) { // longpress detected to exit
        delay(200);
        goto SAIR;
      }
      else {
        if(versionIndex==0) versionIndex = versions.size() - 1;
        else if(versionIndex>0) versionIndex--;
        redraw = true;
      }
    }
    #endif

    /* Select to install */
    if(checkSelPress()) { 

      // Definição da matriz "Options"
      options = {
          {"OTA Install", [=]() { installFirmware(String(file), app_size, spiffs, spiffs_offset, spiffs_size, nb, fat, (uint32_t*)FAT_offset, (uint32_t*)FAT_size); }},
          {"Download->SD", [=]() { downloadFirmware(String(file), String(name) + "." + String(version).substring(0,10), dwn_path); }},
          {"Back to List", [=]() { returnToMenu=true; }},
      };
      delay(200);

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
  currentIndex=0;
  delay(200); //debounce from previous btn press
  displayCurrentItem(doc, currentIndex);

  while(1){
    if(returnToMenu) break; // break the loop and gets back to Main Menu

    if (WiFi.status() == WL_CONNECTED) {
    /* UP Btn go to previous item */
      if(checkPrevPress()) {
        if(currentIndex==0) currentIndex = doc.size() - 1;
        else if(currentIndex>0) currentIndex--;
        displayCurrentItem(doc, currentIndex);
        delay(REDRAW_DELAY);

      }
      /* DW Btn to next item */
      if(checkNextPress()) { 
        currentIndex++;
        if((currentIndex+1)>doc.size()) currentIndex = 0;
        displayCurrentItem(doc, currentIndex);
        delay(REDRAW_DELAY);
      }

      /* Select to install */
      if(checkSelPress()) { 
        
        //Checks for long press to get back to Main Menu, only for StickCs.. Cardputer uses Esc btn
        #ifndef ESC_LOGIC
          int time = millis();          // Saves the moment when the btn was pressed
          while(checkSelPress()) { 

            if((millis()-time)>150) tft.drawArc(WIDTH/2,HEIGHT/2,25,15,0,360*(millis()-time)/1000,ALCOLOR,BGCOLOR,false);
          }  // while pressed the btn, hold the code to count the time
          if((millis()-time)>1000) break;// check how many ms it was kept held on and stop the loop if more than 250ms
          else { 
            loopVersions();          // goes to the Version information
            returnToMenu=false;
          }
        #else
        loopVersions();
        returnToMenu=false;

        #endif

        delay(200);
      }

      #ifdef ESC_LOGIC
      if(checkEscPress()) break; //  Esc btn to get back to Main Menu.
      #endif
    } 
    else {
      displayRedStripe("WiFi: Disconnected");
      delay(5000);
      break;
    }
  }
  WiFi.disconnect(true,true);
  WiFi.mode(WIFI_OFF);
  doc.clear();
}

/*********************************************************************
**  Function: tftprintln                             
**  similar to tft.println(), but allows to include margin
**********************************************************************/
void tftprintln(String txt, int margin, int numlines) {
  int size=txt.length();
  if(numlines == 0) numlines = (HEIGHT-2*margin) / (tft.textsize*8);
  int nchars = (WIDTH-2*margin)/(6*tft.textsize); // 6 pixels of width fot a letter size 1
  int x = tft.getCursorX();
  int start=0;
  while(size>0 && numlines>0) {
    if(tft.getCursorX()<margin) tft.setCursor(margin,tft.getCursorY());
    nchars = (WIDTH-tft.getCursorX()-margin)/(6*tft.textsize); // 6 pixels of width fot a letter size 1
    tft.println(txt.substring(0,nchars));
    txt=txt.substring(nchars);
    size -= nchars;
    numlines--;
  }
}
/*********************************************************************
**  Function: tftprintln                             
**  similar to tft.println(), but allows to include margin
**********************************************************************/
void tftprint(String txt, int margin, int numlines) {
  int size=txt.length();
  if(numlines == 0) numlines = (HEIGHT-2*margin) / (tft.textsize*8);
  int nchars = (WIDTH-2*margin)/(6*tft.textsize); // 6 pixels of width fot a letter size 1
  int x = tft.getCursorX();
  int start=0;
  bool prim=true;
  while(size>0 && numlines>0) {
    if(!prim) { tft.println(); }
    if(tft.getCursorX()<margin) tft.setCursor(margin,tft.getCursorY());
    nchars = (WIDTH-tft.getCursorX()-margin)/(6*tft.textsize); // 6 pixels of width fot a letter size 1
    tft.print(txt.substring(0,nchars));
    txt = txt.substring(nchars);
    size -= nchars;
    numlines--;
    prim = false;
  }
}