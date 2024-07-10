#include <EEPROM.h>
//#include <ArduinoJson.h>

/*
config.conf JSON structure

[
   {
      "rot": 1,
      "onlyBins":1,
      "bright":100,
      "askSpiffs":1,
      "wui_usr":"admin",
      "wui_pwd":"m5launcher",
      "dwn_path": "/downloads/",
      "FGCOLOR":2016,
      "BGCOLOR":0,
      "ALCOLOR":63488,
      "even":13029,
      "odd": 12485,
      "wifi":[
         {
            "ssid":"Pirata",
            "pwd":"11111111",
         },
         {
            "ssid":"Pirata2",
            "pwd":"12345678",
         },
      ]
   }
]

*/

//Functions in this file;

void setBrightnessMenu();
void setBrightness(int bright, bool save = true);
void getBrightness(); 
bool gsetOnlyBins(bool set = false, bool value = true);
bool gsetAskSpiffs(bool set = false, bool value = true);
int gsetRotation(bool set = false);
void getConfigs();
void saveConfigs();
void setdimmerSet();
void setUiColor();