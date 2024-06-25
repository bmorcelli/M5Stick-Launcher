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

extern size_t MAX_SPIFFS;
extern size_t MAX_APP;
extern size_t MAX_FAT0;
extern size_t MAX_FAT1;


extern int prog_handler;    // 0 - Flash, 1 - SPIFFS, 2 - Download

extern bool sdcardMounted;

extern std::vector<std::pair<std::string, std::function<void()>>> options;

extern  String ssid;

extern  String pwd;

extern int currentIndex;

extern JsonDocument doc;

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