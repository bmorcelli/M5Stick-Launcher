#include <WiFi.h>
#include <M5-HTTPUpdate.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>


void installFirmware(String fileAddr, uint32_t app_size, bool spiffs, uint32_t spiffs_offset, uint32_t spiffs_size, bool nb, bool fat, uint32_t fat_offset[2], uint32_t fat_size[2]);

void downloadFirmware(String fileAddr, String fileName, String folder = "/downloads/");

void wifiConnect(String ssid, int encryptation, bool isAP = false);

bool GetJsonFromM5();

bool installFAT_OTA( WiFiClientSecure *client, String fileAddr, uint32_t offset, uint32_t size, const char *label);
