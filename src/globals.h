#pragma once
// Globals.h
// https://github.com/espressif/esp-idf/blob/master/components/soc/esp32/include/soc/reset_reasons.h for further refference
#ifndef GLOBALS_H
#define GLOBALS_H
#include "pre_compiler.h"
#include "interface.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include <vector>
#if !defined(SDM)
    #define SDM SD
    #define SDM_SD
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

// Screen sleep control variables
extern unsigned long previousMillis;
extern bool isSleeping;
extern bool isScreenOff;
extern bool dev_mode;

extern int dimmerSet;
extern int bright;
extern bool dimmer;

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

extern uint8_t buff[1024];

extern const int bufSize;

//Used to handle the update in webUI
extern bool update;

//Used to choose SPIFFS or not
extern bool askSpiffs;

//Don't let open OTA after use WebUI due t oRAM handling
extern bool stopOta;

//size o the file in the webInterface
extern size_t file_size;

#if defined(HEADLESS)
extern uint8_t _miso;
extern uint8_t _mosi;
extern uint8_t _sck;
extern uint8_t _cs;
#endif

#endif