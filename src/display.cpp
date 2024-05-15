
#define LAUNCHER_VERSION "2.0.1"

#include "display.h"
#include "globals.h"
#include "mykeyboard.h"
#include "onlineLauncher.h"
#include "sd_functions.h"

TFT_eSPI tft = TFT_eSPI();         // Invoke custom library
TFT_eSprite sprite = TFT_eSprite(&tft);
TFT_eSprite menu_op = TFT_eSprite(&tft);

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
** Function name: resetSpriteDisplay -> Sprite
** Description:   set cursor to 0,0, screen and text to default color
***************************************************************************************/
void resetSpriteDisplay(int x, int y, uint16_t fc, int size, uint16_t bg, uint16_t screen) {
    if(x<0) x=0;
    if(y<0) y=0;
    sprite.setCursor(x,y);
    sprite.fillScreen(screen);
    sprite.setTextSize(size);
    sprite.setTextColor(fc,bg);
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
** Function name: setSpriteDisplay -> SPRITE
** Description:   set cursor, font color, size and bg font color of Sprite
***************************************************************************************/
void setSpriteDisplay(int x, int y, uint16_t fc, int size, uint16_t bg) {
    if (x>=0 && y<0)        sprite.setCursor(x,sprite.getCursorY());    // if -1 on x, sets only y
    else if (x<0 && y>=0)   sprite.setCursor(sprite.getCursorX(),y);    // if -1 on y, sets only x
    else if (x>=0 && y>=0)  sprite.setCursor(x,y);                      // if x and y > 0, sets both
    sprite.setTextSize(size);
    sprite.setTextColor(fc,bg);
}

/***************************************************************************************
** Function name: BootScreen
** Description:   Start Display functions and display bootscreen
***************************************************************************************/
void initDisplay() {

    sprite.fillRect(0,0,WIDTH,HEIGHT,BGCOLOR);

    srand(time(0));
    sprite.setTextSize(FONT_P);
    sprite.setCursor(0,0);
    while(sprite.getCursorY()<sprite.height()) {
        int cor = rand() % 3;
        if (cor==0) sprite.setTextColor(0x35e5);
        else if (cor==1) sprite.setTextColor(0x33c5);
        else sprite.setTextColor(0x0ce0);
        sprite.print(random(0,9));
    }
    sprite.setTextSize(FONT_G);
    sprite.setTextColor(TFT_GREEN);

    #ifndef STICK_C
    sprite.drawCentreString("M5Launcher",sprite.width()/2,sprite.height()/2-10,SMOOTH_FONT); //SMOOTH_FONT
    #else
    sprite.drawCentreString("Launcher",sprite.width()/2,sprite.height()/2-10,SMOOTH_FONT); //SMOOTH_FONT
    #endif

    sprite.pushSprite(10,10);
    delay(50);

    tft.drawSmoothRoundRect(5,5,5,5,WIDTH-10,HEIGHT-10,FGCOLOR,BGCOLOR);
    
}

/***************************************************************************************
** Function name: displayCurrentItem
** Description:   Display Item on Screen before instalation
***************************************************************************************/
void displayCurrentItem(JsonDocument doc, int currentIndex) {
  JsonObject item = doc[currentIndex];

  const char* name = item["name"];
  const char* author = item["author"];

  tft.fillScreen(BGCOLOR);
  tft.drawSmoothRoundRect(5,5,5,5,WIDTH-10,HEIGHT-10,FGCOLOR,BGCOLOR);
  sprite.fillRect(0,0,sprite.width(),sprite.height(),BGCOLOR);

  resetSpriteDisplay(0, 0, FGCOLOR,FONT_P);
  sprite.print("Firmware: ");
  
  setSpriteDisplay(0, 12, TFT_WHITE, FONT_M);
  sprite.println(String(name).substring(0,56));
  
  setSpriteDisplay(0, 12+4*FONT_M*8, FGCOLOR);
  sprite.print("by: ");
  sprite.setTextColor(TFT_WHITE);
  sprite.println(String(author).substring(0,14));


  sprite.setTextColor(FGCOLOR);
  sprite.drawChar('<', 0, sprite.height()-FONT_M*9);
  sprite.drawChar('>', sprite.width()-FONT_M*6, sprite.height()-FONT_M*9);
  
  #ifndef STICK_C
  String texto = "More information";
  setSpriteDisplay(int(sprite.width()/2 - 3*texto.length()), 98, FGCOLOR+0x2222, FONT_P,BGCOLOR);
  sprite.println(texto);

  texto = String(currentIndex+1) + " of " + String(doc.size());
  sprite.setCursor(int(sprite.width()/2 - 3*texto.length()),sprite.getCursorY());
  //setSpriteDisplay(int(sprite.width()/2 - 3*texto.length()), 108, TFT_DARKGREEN, 1,BGCOLOR);
  sprite.println(texto);
  sprite.drawRoundRect(sprite.width()/2 - 6*11, 96, 12*11,19,3,FGCOLOR+0x2222);
  #else

  String texto = String(currentIndex+1) + " of " + String(doc.size());
  setSpriteDisplay(int(sprite.width()/2 - 3*texto.length()), sprite.height()-FONT_M*6, FGCOLOR+0x2222, FONT_P,BGCOLOR);
  sprite.println(texto);

  #endif


  sprite.pushSprite(10,10);
  int bar = int(WIDTH/(doc.size()));
  if (bar<5) bar = 5;
  tft.fillRect((WIDTH*currentIndex)/doc.size(),HEIGHT-5,bar,5,FGCOLOR);

}

/***************************************************************************************
** Function name: displayCurrentVersion
** Description:   Display Version on Screen before instalation
***************************************************************************************/
void displayCurrentVersion(String name, String author, String version, String published_at, int versionIndex, JsonArray versions) {
    tft.fillScreen(BGCOLOR);
    tft.drawSmoothRoundRect(5,5,5,5,WIDTH-10,HEIGHT-10,ALCOLOR,BGCOLOR);
    sprite.fillRect(0,0,sprite.width(),sprite.height(),BGCOLOR);
    resetSpriteDisplay(0, 0, TFT_WHITE,FONT_M);
    sprite.println(String(name).substring(0,36));
    #ifndef STICK_C
    setSpriteDisplay(0,40,ALCOLOR,FONT_M);
    #endif
    sprite.print("by: ");
    sprite.setTextColor(TFT_WHITE);
    if (String(author).length()>14) sprite.println(String(author).substring(0,14));
    else sprite.println(String(author));
    
    sprite.setTextColor(ALCOLOR);
    sprite.print("v: ");
    sprite.setTextColor(TFT_WHITE);
    if (String(version).length()>15) sprite.println(String(version).substring(0,15));
    else sprite.println(String(version));

    sprite.setTextColor(ALCOLOR);
    sprite.print("from: ");
    sprite.setTextColor(TFT_WHITE);
    sprite.println(String(published_at));

    if(versions.size()>1) {
        sprite.setTextColor(ALCOLOR);
        sprite.drawChar('<', 0, sprite.height()-FONT_M*9);
        sprite.drawChar('>', sprite.width()-FONT_M*6, sprite.height()-FONT_M*9);
        sprite.setTextColor(TFT_WHITE);
    }

    setSpriteDisplay(-1, -1,ALCOLOR,FONT_M,BGCOLOR);
    sprite.drawCentreString("Options",sprite.width()/2,sprite.height()-FONT_M*9,1);
    sprite.drawRoundRect(sprite.width()/2 - 3*FONT_M*11, sprite.height()-FONT_M*9-2, FONT_M*6*11,FONT_M*8+3,3,ALCOLOR);
    sprite.pushSprite(10,10);

    int div = versions.size();
    if(div==0) div = 1;

    int bar = int(WIDTH/div);
    if (bar<5) bar = 5;
    tft.fillRect((WIDTH*versionIndex)/div,HEIGHT-5,bar,5,ALCOLOR);
}

/***************************************************************************************
** Function name: displayRedStripe
** Description:   Display Red Stripe with information
***************************************************************************************/
void displayRedStripe(String text) {
#ifndef STICK_C
    int size;
    if(text.length()<19) size = FONT_M;
    else size = FONT_P;
    tft.fillRect(10, 55, WIDTH - 20, 26, ALCOLOR);
    if(size==2) setTftDisplay(WIDTH/2 - FONT_M*3*text.length(), 60, TFT_WHITE, size, ALCOLOR);
    else setTftDisplay(WIDTH/2 - FONT_P*3*text.length(), 65, TFT_WHITE, size, ALCOLOR);
    tft.println(text);
#else
    int size;
    if(text.length()<20) size = FONT_M;
    else size = FONT_P;
    tft.fillRect(10, 5, WIDTH - 20, 20, ALCOLOR);
    if(size==2) setTftDisplay(WIDTH/2 - FONT_M*3*text.length(), 7, TFT_WHITE, size, ALCOLOR);
    else setTftDisplay(WIDTH/2 - FONT_P*3*text.length(), 7, TFT_WHITE, size, ALCOLOR);
    tft.println(text);
#endif

}

/***************************************************************************************
** Function name: progressHandler
** Description:   Função para manipular o progresso da atualização
** Dependencia: prog_handler =>>    0 - Flash, 1 - SPIFFS
***************************************************************************************/
void progressHandler(int progress, size_t total) {
#ifndef STICK_C
  int barWidth = map(progress, 0, total, 0, 200);
  if(barWidth <7) {
    tft.fillRect(6, 6, WIDTH-12, HEIGHT-12, BGCOLOR);
    if (prog_handler == 1) tft.drawRect(18, HEIGHT - 28, 204, 17, ALCOLOR);
    else tft.drawRect(18, HEIGHT - 47, 204, 17, FGCOLOR);
    
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
  if(barWidth <5) {
    tft.fillRect(6, 6, WIDTH-12, HEIGHT-12, BGCOLOR);
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
void drawOptions(int index,const std::vector<std::pair<std::string, std::function<void()>>>& options, uint16_t fgcolor, uint16_t bgcolor) {
    int menuSize = options.size();
    if(options.size()>MAX_MENU_SIZE) menuSize = MAX_MENU_SIZE;

    menu_op.createSprite(WIDTH*0.7, (FONT_M*8+4)*menuSize + 10);
    menu_op.fillRoundRect(0,0,WIDTH*0.7,(FONT_M*8+4)*menuSize+10,5,bgcolor);
    
    menu_op.setTextColor(fgcolor,bgcolor);
    menu_op.setTextSize(FONT_M);
    menu_op.setCursor(5,5);

    int i=0;
    int init = 0;
    menuSize = options.size();
    if(index>=MAX_MENU_SIZE) init=index-MAX_MENU_SIZE+1;
    for(i=0;i<menuSize;i++) {
      if(i>=init) {
        String text="";
        if(i==index) text+=">";
        else text +=" ";
        text += String(options[i].first.c_str());
        menu_op.setCursor(5,menu_op.getCursorY()+4);
        menu_op.println(text.substring(0,13));
      }
    }

    if(options.size()>MAX_MENU_SIZE) menuSize = MAX_MENU_SIZE;
    menu_op.drawRoundRect(0,0,WIDTH*0.7,(FONT_M*8+4)*menuSize+10,5,fgcolor);
    menu_op.pushSprite(WIDTH*0.15,HEIGHT/2-menuSize*(FONT_M*8+4)/2 -5);
    menu_op.deleteSprite();
}

void drawSection(int x, int y, int w, int h, uint16_t color, const char* text, bool isSelected);
void drawDeviceBorder();
void drawBatteryStatus();

/***************************************************************************************
** Function name: drawMainMenu
** Description:   Função para desenhar e mostrar o menu principal
***************************************************************************************/
void drawMainMenu(int index) {
    const int border = 10;
    const uint16_t colors[3] = {
        static_cast<uint16_t>(sdcardMounted ? FGCOLOR : TFT_DARKGREY), 
        static_cast<uint16_t>(!stopOta ? FGCOLOR : TFT_DARKGREY), 
        static_cast<uint16_t>(FGCOLOR)
    };

    const char* texts[3] = { "SD", "OTA", "CFG" };
#ifndef STICK_C
    const char* messages[3] = { "Launch from or mng SDCard", 
                                "Install/download from M5Burner", 
                                "Change Launcher settings." };
#else
    const char* messages[3] = { "Launch from SDCard", 
                                "Online Installer", 
                                "Launcher settings." };

#endif
    sprite.deleteSprite();
    sprite.createSprite(WIDTH - 20, HEIGHT - 20);
    sprite.fillRect(0, 0, WIDTH, HEIGHT, BGCOLOR);
    setSpriteDisplay(2, 2, FGCOLOR, 1, BGCOLOR);
#ifndef STICK_C
    sprite.print("M5Launcher 2.0.1");
    sprite.setTextSize(FONT_G);
#else
    sprite.print("Launcher 2.0.1");
    sprite.setTextSize(FONT_M);
#endif
    for (int i = 0; i < 3; ++i) {
        int x = border / 2 + i * (sprite.width() / 3);
        int y = 20 + border;
        int w = sprite.width() / 3 - border;
        int h = sprite.height() / 2;
        drawSection(x, y, w, h, colors[i], texts[i], index == i);
    }

    setSpriteDisplay(-1, -1, FGCOLOR, 1, BGCOLOR);
#ifndef STICK_C
    sprite.drawCentreString(messages[index], sprite.width() / 2, sprite.height() - 15, 1);
#else
    sprite.drawCentreString(messages[index], sprite.width() / 2, 18, 1);
#endif
    sprite.pushSprite(10,10);
    drawDeviceBorder();
    drawBatteryStatus();
}

void drawSection(int x, int y, int w, int h, uint16_t color, const char* text, bool isSelected) {
    sprite.setTextColor(isSelected ? BGCOLOR : color);
    if (isSelected) {
        sprite.fillRoundRect(x+5, y+5, w, h, 5,  color + 0x2222);
        sprite.fillRoundRect(x, y, w, h, 5, color);
    }
    sprite.drawRoundRect(x, y, w, h, 5, color);
#ifndef STICK_C
    sprite.drawCentreString(text, x + w/2, sprite.height() / 3 + 8, SMOOTH_FONT);
#else
    sprite.drawCentreString(text,x + w/2, 2 * sprite.height() / 3, SMOOTH_FONT);
#endif
}

void drawDeviceBorder() {
    tft.drawRoundRect(5, 5, WIDTH - 10, HEIGHT - 10, 5, FGCOLOR);
    tft.drawLine(5, 25, WIDTH - 5, 25, FGCOLOR);
}

void drawBatteryStatus() {
    tft.drawRoundRect(WIDTH - 42, 7, 34, 17, 2, FGCOLOR);
    int bat = getBattery();
    tft.setTextSize(FONT_P);
    tft.setTextColor(FGCOLOR, BGCOLOR);
    tft.drawRightString(String(bat) + "%", WIDTH - 45, 12, 1);
    tft.fillRoundRect(WIDTH - 40, 9, 30 * bat / 100, 13, 2, FGCOLOR);
    tft.drawLine(WIDTH - 30, 9, WIDTH - 30, 9 + 13, BGCOLOR);
    tft.drawLine(WIDTH - 20, 9, WIDTH - 20, 9 + 13, BGCOLOR);
}


/***************************************************************************************
** Function name: listFiles
** Description:   Função para desenhar e mostrar o menu principal
***************************************************************************************/
#define MAX_ITEMS 7
void listFiles(int index, String fileList[][3]) {
    sprite.fillRect(0,0,sprite.width(),sprite.height(),BGCOLOR);
    sprite.setCursor(0,0);
    sprite.setTextSize(FONT_M);
    int arraySize = 0;
    while(fileList[arraySize][2]!="" && arraySize < MAXFILES) arraySize++;
    int i=0;
    int start=0;
    if(index>=MAX_ITEMS) {
        start=index-MAX_ITEMS+1;
        if(start<0) start=0;
    }
    
    while(i<arraySize) {
        if(i>=start && fileList[i][2]!="") {
            if(fileList[i][2]=="folder") sprite.setTextColor(FGCOLOR-0x1111);
            else if(fileList[i][2]=="operator") sprite.setTextColor(ALCOLOR);
            else sprite.setTextColor(FGCOLOR);

            if (index==i) sprite.print(">");
            else sprite.print(" ");
            sprite.println(fileList[i][0].substring(0,16));
            
        }
        i++;
        if (i==(start+MAX_ITEMS) || fileList[i][2]=="") break;
    }
    
    sprite.pushSprite(10,10);
}


void displayScanning() {
    //Show Scanning display
    menu_op.deleteSprite();
    menu_op.createSprite(WIDTH*0.7, FONT_M*16);
    menu_op.fillRoundRect(0,0,WIDTH*0.7,FONT_M*16,5,BGCOLOR);
    
    menu_op.setTextColor(ALCOLOR,BGCOLOR);
    menu_op.setTextSize(FONT_M);
    menu_op.drawCentreString("Scanning..",menu_op.width()/2,5,1);
    menu_op.drawRoundRect(0,0,WIDTH*0.7,FONT_M*16,5,ALCOLOR);
    menu_op.pushSprite(WIDTH*0.15,HEIGHT/2-10 -5);
    menu_op.deleteSprite();
}


/*********************************************************************
**  Function: loopOptions                             
**  Where you choose among the options in menu
**********************************************************************/
void loopOptions(const std::vector<std::pair<std::string, std::function<void()>>>& options, bool bright){
  bool redraw = true;
  int index = 0;
  while(1){
    if (redraw) { 
      drawOptions(index,options, TFT_RED, BGCOLOR);
      if(bright){
        #if !defined(STICK_C_PLUS)
        int bl = MINBRIGHT + round(((255 - MINBRIGHT) * (4 - index) * 0.25)); // 4 is the number of options
        analogWrite(BACKLIGHT, bl);
        #else
        axp192.ScreenBreath(100*(4 - index) * 0.25);  // 4 is the number of options
        #endif
      }
      redraw=false;
      delay(200); 
    }

    if(checkPrevPress()) {
    #ifdef CARDPUTER  
      if(index==0) index = options.size() - 1;
      else if(index>0) index--;
      redraw = true;
    #else
      break;
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

    #ifdef CARDPUTER
    Keyboard.update();
    if(Keyboard.isKeyPressed('`')) break;
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
    bool spiffs = Version["spiffs"].as<bool>();
    bool nb = Version["nb"].as<bool>();
    uint32_t app_size = Version["app_size"].as<uint32_t>();
    uint32_t spiffs_size = Version["spiffs_size"].as<uint32_t>();
    uint32_t spiffs_offset = Version["spiffs_offset"].as<uint32_t>();
    if(redraw){
      
      displayCurrentVersion(String(name), String(author), String(version), String(published_at), versionIndex, versions);
      redraw = false;
      delay(200);
    }
    /* DW Btn to next item */
    if(checkNextPress()) { 
      versionIndex++;
      if(versionIndex>versions.size()-1) versionIndex = 0;
      redraw = true;
      delay(200);
    }

    /* UP Btn go back to FW menu and ´<´ go to previous version item */
    if(checkPrevPress()) 
    #ifdef CARDPUTER
        /* UP Btn go to previous item */
    { 
      versionIndex--;
      if(versionIndex<0) versionIndex = versions.size()-1;
      redraw = true;
      delay(200);
    }
    Keyboard.update();
    if(Keyboard.isKeyPressed('`'))
    #endif
    {
      delay(200);
      goto SAIR;
    }

    /* Select to install */
    if(checkSelPress()) { 

      // Definição da matriz "Options"
      std::vector<std::pair<std::string, std::function<void()>>> options = {
          {"OTA Install", [=]() { installFirmware(String(file), app_size, spiffs, spiffs_offset, spiffs_size, nb); }},
          {"Download->SD", [=]() { downloadFirmware(String(file), String(name)); }},
          {"Main Menu", [=]() { returnToMenu=true; }},
      };
      delay(200);

      loopOptions(options);
      sprite.createSprite(WIDTH-20,HEIGHT-20);
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
  sprite.deleteSprite();
  delay(200); //debounce from previous btn press
  sprite.createSprite(WIDTH-20,HEIGHT-20);
  displayCurrentItem(doc, currentIndex);

  while(1){
    if(returnToMenu) break; // break the loop and gets back to Main Menu

    if (WiFi.status() == WL_CONNECTED) {
    /* UP Btn go to previous item */
      if(checkPrevPress()) {
        if(currentIndex==0) currentIndex = doc.size() - 1;
        else if(currentIndex>0) currentIndex--;
        displayCurrentItem(doc, currentIndex);
        delay(200);

      }
      /* DW Btn to next item */
      if(checkNextPress()) { 
        currentIndex++;
        if((currentIndex+1)>doc.size()) currentIndex = 0;
        displayCurrentItem(doc, currentIndex);
        delay(200);
      }

      /* Select to install */
      if(checkSelPress()) { 
        
        //Checks for long press to get back to Main Menu, only for StickCs.. Cardputer uses Esc btn
        #ifndef CARDPUTER
          int time = millis();          // Saves the moment when the btn was pressed
          while(checkSelPress()) { 

            if((millis()-time)>150) tft.drawArc(WIDTH/2,HEIGHT/2,25,15,0,360*(millis()-time)/450,ALCOLOR,BGCOLOR,false);
          }  // while pressed the btn, hold the code to count the time
          if((millis()-time)>450) break;// check how many ms it was kept held on and stop the loop if more than 250ms
          else loopVersions();          // goes to the Version information
        #else
        loopVersions();
        #endif

        delay(200);
      }

      #ifdef CARDPUTER
      Keyboard.update();
      if(Keyboard.isKeyPressed('`')) break; //  Esc btn to get back to Main Menu.
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

