#include <WiFi.h>
#include <M5-HTTPUpdate.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>


void installFirmware(String fileAddr, uint32_t app_size, bool spiffs, uint32_t spiffs_offset, uint32_t spiffs_size, bool nb);

void downloadFirmware(String fileAddr, String fileName, String folder = "/downloads/");

void wifiConnect(String ssid, int encryptation, bool isAP = false);

bool GetJsonFromM5();

