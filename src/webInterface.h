#ifndef __WEBINTERFACE_H
#define __WEBINTERFACE_H

#include <SD_MMC.h>
#include <SD.h>

#include <SPI.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <webFiles.h>


// function defaults
String humanReadableSize(uint64_t bytes);
String listFiles(String folder);
String processor(const String& var);
String readLineFromFile(File myFile);

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void notFound(AsyncWebServerRequest *request);

void configureWebServer();
void startWebUi(String ssid, int encryptation, bool mode_ap = false);


void webUIMyNet();

void loopOptionsWebUi() ;

#endif //__WEBINTERFACE_H