#include "globals.h"

#include <EEPROM.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <M5-HTTPUpdate.h>
#include <TFT_eSPI.h>
#include <SPIFFS.h>
#include "esp_ota_ops.h"

#include <iostream>
#include <functional>
#include <vector>
#include <string>


// Public Globals
uint32_t MAX_SPIFFS = 0;
uint32_t MAX_APP = 0;
uint32_t MAX_FAT_vfs = 0;
uint32_t MAX_FAT_sys = 0;
uint16_t FGCOLOR = TFT_GREEN;
uint16_t ALCOLOR = TFT_RED;
uint16_t BGCOLOR = TFT_BLACK;
uint16_t odd_color = 0x30c5;
uint16_t even_color = 0x32e5;
unsigned long dimmerTemp=millis();
int dimmerSet=20;
int bright=100;
bool dimmer=false;
int prog_handler;    // 0 - Flash, 1 - SPIFFS
int currentIndex;
int rotation;
bool sdcardMounted;
bool onlyBins;
bool returnToMenu;
bool update;
bool askSpiffs;
bool stopOta;
//bool command;
size_t file_size;
String ssid;
String pwd;
String wui_usr = "admin";
String wui_pwd = "m5launcher";
String dwn_path = "/downloads/";
JsonDocument doc;
JsonDocument settings;
std::vector<std::pair<std::string, std::function<void()>>> options;
const int bufSize = 1024;
uint8_t buff[1024] = {0};


#include "display.h"
#include "mykeyboard.h"
#include "onlineLauncher.h"
#include "sd_functions.h"
#include "webInterface.h"
#include "partitioner.h"
#include "settings.h"

/*********************************************************************
**  Function: get_partition_sizes                                    
**  Get the size of the partitions to be used when installing
*********************************************************************/
void get_partition_sizes() {
    // Obter a tabela de partições
    const esp_partition_t* partition;
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);

    // Iterar sobre as partições do tipo APP
    while (it != NULL) {
        partition = esp_partition_get(it);
        if (partition != NULL && partition->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_0) {
            MAX_APP = partition->size;
        }
        it = esp_partition_next(it);
    }
    esp_partition_iterator_release(it);

    // Iterar sobre as partições do tipo DATA
    it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
    while (it != NULL) {
        partition = esp_partition_get(it);
        if (partition != NULL) {
            if (partition->subtype == ESP_PARTITION_SUBTYPE_DATA_SPIFFS) {
                MAX_SPIFFS = partition->size;
            } else if (partition->subtype == ESP_PARTITION_SUBTYPE_DATA_FAT) {
              log_i("label: %s",partition->label);
              if(strcmp(partition->label,"vfs")==0) MAX_FAT_vfs = partition->size;
              else if (strcmp(partition->label,"sys")==0) MAX_FAT_sys = partition->size;
            }
        }
        it = esp_partition_next(it);
    }
    esp_partition_iterator_release(it);
    if(MAX_SPIFFS==0 && askSpiffs) gsetAskSpiffs(true, false);

    // Logar os tamanhos das partições
    ESP_LOGI("Partition Sizes", "MAX_APP: %d", MAX_APP);
    ESP_LOGI("Partition Sizes", "MAX_SPIFFS: %d", MAX_SPIFFS);
    ESP_LOGI("Partition Sizes", "MAX_FAT_sys: %d", MAX_FAT_sys);
    ESP_LOGI("Partition Sizes", "MAX_FAT_vfs: %d", MAX_FAT_vfs);
}

/*********************************************************************
**  Function: setup                                    
**  Where the devices are started and variables set    
*********************************************************************/
void setup() {
  Serial.begin(115200);

  // Setup GPIOs and stuff
  #if defined(STICK_C_PLUS2)
    pinMode(UP_BTN, INPUT);
    pinMode(SEL_BTN, INPUT);
    pinMode(DW_BTN, INPUT);
    pinMode(4, OUTPUT);
    digitalWrite(4,HIGH);
  #elif defined(STICK_C_PLUS)
    axp192.begin();
    pinMode(SEL_BTN, INPUT);
    pinMode(DW_BTN, INPUT);
  #elif defined(M5STACK)
    //M5.begin(); // Begin after TFT, for SDCard to work
  #elif defined(CARDPUTER)
    Keyboard.begin();
    pinMode(10, INPUT);
  #endif

  #if defined(BACKLIGHT)
  pinMode(BACKLIGHT, OUTPUT);
  #endif

  EEPROM.begin(EEPROMSIZE); // open eeprom
  if(EEPROM.read(0) > 3 || EEPROM.read(1) > 240 || EEPROM.read(2) > 100 || EEPROM.read(9) > 1 || EEPROM.read(EEPROMSIZE-2) > 1 || (EEPROM.read(EEPROMSIZE-3)==0xFF && EEPROM.read(EEPROMSIZE-4) == 0xFF && EEPROM.read(EEPROMSIZE-5)==0xFF && EEPROM.read(EEPROMSIZE-6) == 0xFF)) {
    log_i("EEPROM back to default\n0=%d\n1=%d\n2=%d\n9=%d\nES-1=%d",EEPROM.read(0),EEPROM.read(1),EEPROM.read(2),EEPROM.read(9),EEPROM.read(EEPROMSIZE-2) );
  #if defined(CARDPUTER) || defined(M5STACK)
    EEPROM.write(0, 1);    // Right rotation for cardputer
  #else
    EEPROM.write(0, 3);    // Left rotation
  #endif
    EEPROM.write(1, 20);  // 20s Dimm time
    EEPROM.write(2, 100);  // 100% brightness
    EEPROM.write(9, 1);    // OnlyBins
    EEPROM.writeString(10,"");
    EEPROM.write(EEPROMSIZE-2, 1);  // AskSpiffs

    //FGCOLOR
    EEPROM.write(EEPROMSIZE-3, 0x07); 
    EEPROM.write(EEPROMSIZE-4, 0xE0);
    //BGCOLOR
    EEPROM.write(EEPROMSIZE-5, 0);
    EEPROM.write(EEPROMSIZE-6, 0);
    //ALCOLOR
    EEPROM.write(EEPROMSIZE-7, 0xF8);
    EEPROM.write(EEPROMSIZE-8, 0x00);
    //odd
    EEPROM.write(EEPROMSIZE-9, 0x30);
    EEPROM.write(EEPROMSIZE-10, 0xC5);
    //even
    EEPROM.write(EEPROMSIZE-11, 0x32);
    EEPROM.write(EEPROMSIZE-12, 0xe5);
    EEPROM.commit();       // Store data to EEPROM
  }
  if(EEPROM.read(0) != 1 && EEPROM.read(0) != 3)  { 
    EEPROM.write(0, 3);    // Left rotation
    EEPROM.commit();       // Store data to EEPROM
  }
  rotation = EEPROM.read(0);
  dimmerSet = EEPROM.read(1);
  bright = EEPROM.read(2);
  onlyBins = EEPROM.read(9);
  askSpiffs = EEPROM.read(EEPROMSIZE-2);
  FGCOLOR =    (EEPROM.read(EEPROMSIZE-3)  << 8) | EEPROM.read(EEPROMSIZE-4);
  BGCOLOR =    (EEPROM.read(EEPROMSIZE-5)  << 8) | EEPROM.read(EEPROMSIZE-6);
  ALCOLOR =    (EEPROM.read(EEPROMSIZE-7)  << 8) | EEPROM.read(EEPROMSIZE-8);
  odd_color =  (EEPROM.read(EEPROMSIZE-9)  << 8) | EEPROM.read(EEPROMSIZE-10);
  even_color = (EEPROM.read(EEPROMSIZE-11) << 8) | EEPROM.read(EEPROMSIZE-12);
  EEPROM.end();  

  // declare variables
  size_t currentIndex=0;  
  prog_handler=0;
  sdcardMounted=false;
  String fileToCopy;

  //Init Display
  tft.setAttribute(PSRAM_ENABLE,true);
  tft.init();
  tft.setRotation(rotation);
  tft.fillScreen(BGCOLOR);
  setBrightness(bright,false);
  initDisplay(true);  

#if defined(M5STACK)
    M5.begin(); // Begin after TFT, for SDCard to work
#endif
  // Performs the verification when Launcher is installed through OTA
  partitionCrawler(); 
  // Checks if the fw in the OTA partition is valid. reading the firstByte looking for 0xE9
  const esp_partition_t* ota_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
  uint8_t firstByte;
  esp_partition_read(ota_partition,0,&firstByte,1);

  //Gets the config.conf from SD Card and fill out the settings JSON
  getConfigs();

  //Start Bootscreen timer
  int i = millis();
  while(millis()<i+5000) { // increased from 2500 to 5000
    initDisplay();        //Inicia o display
    #if defined(M5STACK)
    coreFooter2();
    #endif    
  
  #if defined (CARDPUTER)
    Keyboard.update();
    if(Keyboard.isKeyPressed(KEY_ENTER))
  #elif !defined(M5STACK)
    if(digitalRead(SEL_BTN)==LOW) 
  #elif defined(M5STACK)
    if(checkSelPress())        
  #endif
     {
        tft.fillScreen(BGCOLOR);
        goto Launcher;
      }

  #if defined (CARDPUTER)
    Keyboard.update();
    if (Keyboard.isPressed() && !(Keyboard.isKeyPressed(KEY_ENTER)))
  #elif defined(STICK_C_PLUS2)
    if((digitalRead(UP_BTN)==LOW && (millis()-i>2000)) || digitalRead(DW_BTN)==LOW) 
  #elif defined(STICK_C_PLUS)
    if((axp192.GetBtnPress() && (millis()-i>2000)) || digitalRead(DW_BTN)==LOW)
  #elif defined(M5STACK)
    if(checkNextPress() || checkPrevPress())    
  #endif 
      {
        tft.fillScreen(TFT_BLACK);
        ESP.restart();
      } 
  }
  
  // If nothing is done, check if there are any app installed in the ota partition, if it does, restart device to start installed App.
  if(firstByte==0xE9) {
    tft.fillScreen(TFT_BLACK);
	  ESP.restart();  
  }
  else goto Launcher;


  // If M5 or Enter button is pressed, continue from here
  Launcher:
  get_partition_sizes();
  tft.fillScreen(BGCOLOR);
}

/**********************************************************************
**  Function: loop                                     
**  Main loop                                          
**********************************************************************/
void loop() {
  bool redraw = true;
  int index = 0;
  int opt = 4; // there are 3 options> 1 list SD files, 2 OTA and 3 Config
  stopOta = false; // variable used in WebUI, and to prevent open OTA after webUI without restart
  getBrightness();
  if(sdcardMounted) index=1; //if SD card is not present, paint SD square grey and auto select OTA
  while(1){
    if (redraw) { 
      drawMainMenu(index); 
      #if defined(M5STACK)
      coreFooter();
      #endif
      redraw = false; 
      delay(200); 
    }

    if(checkPrevPress()) {
      if(index==0) index = opt - 1;
      else if(index>0) index--;
      redraw = true;
    }
    // DW Btn to next item 
    if(checkNextPress()) { 
      index++;
      if((index+1)>opt) index = 0;
      redraw = true;
    }

    // Select and run function 
    if(checkSelPress()) { 
      if(index == 0) {  
        if(setupSdCard()) { 
          loopSD(false); 
          tft.fillScreen(BGCOLOR);
          redraw=true;
        }
        else {
          displayRedStripe("Insert SD Card");
          delay(3000);
        }
        
      }
      if(index == 1) {  
        if (!stopOta) {
          if (WiFi.status() != WL_CONNECTED) {
            int nets;
            WiFi.mode(WIFI_MODE_STA);
            displayRedStripe("Scanning...");
            nets=WiFi.scanNetworks();
            //delay(3000);
            options = { };
            for(int i=0; i<nets; i++){
              options.push_back({WiFi.SSID(i).c_str(), [=]() { wifiConnect(WiFi.SSID(i).c_str(),int(WiFi.encryptionType(i))); }});
            }
            delay(200);
            loopOptions(options);
            if (WiFi.status() == WL_CONNECTED) {
              if(GetJsonFromM5()) loopFirmware();
            }

          } else {
            //If it is already connected, download the JSON again... it loses the information once you step out of loopFirmware(), dkw
            closeSdCard();
            if(GetJsonFromM5()) loopFirmware();
          }
          tft.fillScreen(BGCOLOR);
          redraw=true;
        } 
        else {
          displayRedStripe("Restart to open OTA");
          delay(3000);
        } 

      }
      if(index == 2) {
        loopOptionsWebUi();
        tft.fillScreen(BGCOLOR);
        redraw=true;        
      }

      if(index == 3) {  
        options = {
          
          {"Charge Mode", [=](){ chargeMode(); }},
          {"Brightness", [=]() { setBrightnessMenu(); }},
          {"Dim time", [=]()   { setdimmerSet();}},
          {"UI Color", [=]()   { setUiColor();}},
        };
        if(sdcardMounted) {
          if(onlyBins) options.push_back({"All Files",  [=]() { gsetOnlyBins(true, false); saveConfigs();}});
          else         options.push_back({"Only Bins",  [=]() { gsetOnlyBins(true, true); saveConfigs();}});
        }
        
        if(askSpiffs) options.push_back({"Avoid Spiffs",  [=]() { gsetAskSpiffs(true, false); saveConfigs();}});
        else          options.push_back({"Ask Spiffs",    [=]() { gsetAskSpiffs(true, true); saveConfigs();}});
        options.push_back({"Rotate 180",  [=]() { gsetRotation(true); }});
        options.push_back({"Part Change",  [=]() { partitioner(); }});
        options.push_back({"Part List",  [=]() { partList(); }});

      #ifndef STICK_C_PLUS
        options.push_back({"Clear FAT",  [=]() { eraseFAT(); }});
      #endif

        if(MAX_SPIFFS>0) options.push_back({"Save SPIFFS",  [=]() { dumpPartition("spiffs", "/bkp/spiffs.bin"); }});
        //if(MAX_FAT_sys>0) options.push_back({"Bkp FAT sys",  [=]() { dumpPartition("sys", "/bkp/FAT_sys.bin"); }});    //Test only
        if(MAX_FAT_vfs>0) options.push_back({"Save FAT vfs",  [=]() { dumpPartition("vfs", "/bkp/FAT_vfs.bin"); }});
        if(MAX_SPIFFS>0) options.push_back({ "Rest SPIFFS",  [=]() { restorePartition("spiffs"); }});
        //if(MAX_FAT_sys>0) options.push_back({"Rest FAT Sys",  [=]() { restorePartition("sys"); }});                     //Test only
        if(MAX_FAT_vfs>0) options.push_back({"Rest FAT Vfs",  [=]() { restorePartition("vfs"); }});

        options.push_back({"Restart",  [=]() { ESP.restart(); }});
        
        delay(200);
        loopOptions(options);
        tft.fillScreen(BGCOLOR);
        tft.fillScreen(BGCOLOR);
        redraw=true;
      }
      returnToMenu = false;
      redraw = true;
    }
  }
}

