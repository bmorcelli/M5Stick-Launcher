// Globals.h
// https://github.com/espressif/esp-idf/blob/master/components/soc/esp32/include/soc/reset_reasons.h for further refference
#ifndef GLOBALS_H
#define GLOBALS_H
#include <pre_compiler.h>
#include <interface.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include <LittleFS.h> // to make M5GFX compile in Core, Core2 and CoreS3 devices
#include <vector>
#if !defined(SDM)
    #define SDM SD
    #define SDM_SD
#endif

struct keyStroke { // DO NOT CHANGE IT!!!!!
    bool pressed=false;
    bool exit_key=false;
    bool fn = false;
    bool del = false;
    bool enter = false;
    uint8_t modifiers = 0;
    std::vector<char> word;
    std::vector<uint8_t> hid_keys;
    std::vector<uint8_t> modifier_keys;
    

    // Clear function
    void Clear() {
        pressed = false;
        exit_key = false;
        fn = false;
        del = false;
        enter = false;
        modifiers = 0;
        word.clear();
        hid_keys.clear();
        modifier_keys.clear();
    }
};

struct TouchPoint {
    bool pressed = false;
    uint16_t x;
    uint16_t y;

    // clear touch to better handle tasks
    void Clear(void) {
        pressed = false;
        x=0;
        y=0;
    }
};

extern TouchPoint touchPoint;
extern keyStroke KeyStroke;
// Navigation Variables
extern volatile bool NextPress;
extern volatile bool PrevPress;
extern volatile bool UpPress;
extern volatile bool DownPress;
extern volatile bool SelPress;
extern volatile bool EscPress;
extern volatile bool AnyKeyPress;

extern volatile uint16_t tftHeight;
extern volatile uint16_t tftWidth;

extern TaskHandle_t xHandle;
extern inline bool check(volatile bool &btn) {
#ifndef DONT_USE_INPUT_TASK
  if(!btn) return false;
  vTaskSuspend( xHandle );
  btn=false;
  AnyKeyPress=false;
  delay(10);
  vTaskResume( xHandle );
  return true;
#else
    InputHandler();
    if(!btn) return false;
    btn=false;
    AnyKeyPress=false;
    return true;
#endif
}

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