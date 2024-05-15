// display.h
#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>
#include <ArduinoJson.h>
#include <functional>
#include <vector>

#define FGCOLOR TFT_GREEN
#define ALCOLOR TFT_RED
#define BGCOLOR TFT_BLACK

// Declaração dos objetos TFT
extern TFT_eSPI tft; 
extern TFT_eSprite sprite;
extern TFT_eSprite menu_op;

void loopOptions(const std::vector<std::pair<std::string, std::function<void()>>>& options, bool bright = false);
void loopVersions();
void loopFirmware();
void initDisplay(); // Início da função e mostra bootscreen

//Funções para economizar linhas nas outras funções
void resetTftDisplay(int x = 0, int y = 0, uint16_t fc = FGCOLOR, int size = FONT_M, uint16_t bg = BGCOLOR, uint16_t screen = BGCOLOR);
void setTftDisplay(int x = 0, int y = 0, uint16_t fc = tft.textcolor, int size = tft.textsize, uint16_t bg = tft.textbgcolor);

void resetSpriteDisplay(int x = 0, int y = 0, uint16_t fc = FGCOLOR, int size = FONT_M, uint16_t bg = BGCOLOR, uint16_t screen = BGCOLOR);
void setSpriteDisplay(int x = 0, int y = 0, uint16_t fc = sprite.textcolor, int size = sprite.textsize, uint16_t bg = sprite.textbgcolor);

void displayCurrentItem(JsonDocument doc, int currentIndex);
void displayCurrentVersion(String name, String author, String version, String published_at, int versionIndex, JsonArray versions);

void displayRedStripe(String text);

void progressHandler(int progress, size_t total);

void downloadHandler();

void drawOptions(int index,const std::vector<std::pair<std::string, std::function<void()>>>& options, uint16_t fgcolor, uint16_t bgcolor);

void drawMainMenu(int index = 0);

void listFiles(int index, String fileList[][3]);

void displayScanning();

#endif
