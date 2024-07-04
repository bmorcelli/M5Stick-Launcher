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
const int bufSize = 4096;
uint8_t buff[4096] = {0};


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

  // declare variables
  size_t currentIndex=0;  
  prog_handler=0;
  sdcardMounted=false;
  String fileToCopy;

  //Define variables to identify if there is an app installed after Launcher 
  esp_app_desc_t ota_desc;
  esp_err_t err = esp_ota_get_partition_description(esp_ota_get_next_update_partition(NULL), &ota_desc);  
  tft.setAttribute(PSRAM_ENABLE,true);
  tft.init();

  // Setup GPIOs and stuff
  #if defined(STICK_C_PLUS2)
    pinMode(UP_BTN, INPUT);
  #elif defined(STICK_C_PLUS)
    axp192.begin();
  #endif
  #if defined(M5STACK)
    M5.begin(); // inicia os periféricos do CORE, CORE2 e COREs3 exceto o TFT.
  #endif
  #ifndef CARDPUTER
  pinMode(SEL_BTN, INPUT);
  pinMode(DW_BTN, INPUT);
  pinMode(4, OUTPUT);
  digitalWrite(4,HIGH);
  #else
  Keyboard.begin();
  pinMode(10, INPUT);
  #endif

  rotation = gsetRotation();
  tft.setRotation(rotation);
  partitionCrawler(); // Performs the verification when Launcher is installed through OTA
  
  resetTftDisplay();
  initDisplay(true);  
  getConfigs();

  #if defined(BACKLIGHT)
  pinMode(BACKLIGHT, OUTPUT);
  #endif

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
        tft.fillScreen(TFT_BLACK);
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
  if(err == ESP_OK) { 
	  tft.fillScreen(TFT_BLACK);
	  ESP.restart(); 
  }

  // If M5 or Enter button is pressed, continue from here
  Launcher:
  get_partition_sizes();
  tft.fillScreen(TFT_BLACK);
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
          {"Brightness", [=]() { setBrightnessMenu(); }},
        };
        if(sdcardMounted) {
          if(onlyBins) options.push_back({"All Files",  [=]() { gsetOnlyBins(true, false); saveConfigs();}});
          else         options.push_back({"Only Bins",  [=]() { gsetOnlyBins(true, true); saveConfigs();}});
        }
        
        if(askSpiffs) options.push_back({"Avoid Spiffs",  [=]() { gsetAskSpiffs(true, false); saveConfigs();}});
        else          options.push_back({"Ask Spiffs",    [=]() { gsetAskSpiffs(true, true); saveConfigs();}});

      #if !defined(CARDPUTER)
        options.push_back({"Rotate 180",  [=]() { gsetRotation(true); }});
      #endif
      #if defined(STICK_C_PLUS) || defined(STICK_C_PLUS2) || defined(CARDPUTER)
        options.push_back({"Part Change",  [=]() { partitioner(); }});
        options.push_back({"Part List",  [=]() { partList(); }});
      #endif
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
        redraw=true;
      }
      returnToMenu = false;
      redraw = true;
    }
  }
}

