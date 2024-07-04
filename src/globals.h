// Globals.h
#ifndef GLOBALS_H
#define GLOBALS_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include <vector>

#if defined (M5STACK)
  #include <util/Unified/M5Unified.h>
#endif

#define U_FAT_vfs 300 
#define U_FAT_sys 400 

extern uint16_t FGCOLOR;
extern uint16_t ALCOLOR;
extern uint16_t BGCOLOR;
extern uint16_t odd_color;
extern uint16_t even_color;

extern uint32_t MAX_APP;
extern uint32_t MAX_SPIFFS;
extern uint32_t MAX_FAT_vfs;
extern uint32_t MAX_FAT_sys;


extern int prog_handler;    // 0 - Flash, 1 - SPIFFS, 2 - Download

extern bool sdcardMounted;

extern std::vector<std::pair<std::string, std::function<void()>>> options;

extern  String ssid;

extern  String pwd;

extern  String wui_usr;

extern  String wui_pwd;

extern  String dwn_path;

extern int currentIndex;

extern JsonDocument doc;

extern JsonDocument settings;

extern String fileToCopy;

extern bool onlyBins;

extern int rotation;

extern bool returnToMenu;

extern uint8_t buff[4096];

extern const int bufSize;

//Used to handle the update in webUI
extern bool update;

//Used to choose SPIFFS or not
extern bool askSpiffs;

//Don't let open OTA after use WebUI due t oRAM handling
extern bool stopOta;

//size o the file in the webInterface
extern size_t file_size;

#endif