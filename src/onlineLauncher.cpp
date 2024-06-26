#include "onlineLauncher.h"
#include "sd_functions.h"
#include "globals.h"
#include "display.h"
#include "mykeyboard.h"


const char* root_ca3 = 
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
"-----END CERTIFICATE-----\n";


/***************************************************************************************
** Function name: wifiConnect
** Description:   Connects to wifiNetwork
***************************************************************************************/
void wifiConnect(String ssid, int encryptation, bool isAP) {

  if(!isAP) {

    EEPROM.begin(EEPROMSIZE);
    pwd = EEPROM.readString(10); //43

    delay(200);
    if(encryptation>0) pwd = keyboard(pwd,63, "Network Password:");

    if (pwd!=EEPROM.readString(10)) { //43
      EEPROM.writeString(10, pwd); //43
      EEPROM.commit(); // Store data to EEPROM
      EEPROM.end(); // Free EEPROM memory
    }

    WiFi.begin(ssid, pwd);

    resetTftDisplay(10, 10, FGCOLOR,FONT_P);

    tftprint("Connecting to: " + ssid + ".",10);
    tft.drawSmoothRoundRect(5,5,5,5,WIDTH-10,HEIGHT-10,FGCOLOR,BGCOLOR);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      tftprint(".",10);
    }

  } else { //Running in Access point mode
    IPAddress AP_GATEWAY(172, 0, 0, 1);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_GATEWAY, AP_GATEWAY, IPAddress(255, 255, 255, 0));
    WiFi.softAP("M5Launcher", "",6,0,1,false);
    Serial.print("IP: "); Serial.println(WiFi.softAPIP());
  } 

  delay(200);

}

/***************************************************************************************
** Function name: GetJsonFromM5
** Description:   Gets JSON from github server
***************************************************************************************/
bool GetJsonFromM5() {
  #if defined(CARDPUTER) 
  const char* serverUrl = "https://raw.githubusercontent.com/bmorcelli/M5Stack-json-fw/main/cardputer.json";
  #elif defined(STICK_C_PLUS) || defined(STICK_C_PLUS2)
  const char* serverUrl = "https://raw.githubusercontent.com/bmorcelli/M5Stack-json-fw/main/stickc.json";
  #elif defined(CORE2)
  const char* serverUrl = "https://raw.githubusercontent.com/bmorcelli/M5Stack-json-fw/main/core2.json";  
  #elif defined(CORE)
  const char* serverUrl = "https://raw.githubusercontent.com/bmorcelli/M5Stack-json-fw/main/core.json";  
  #endif

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    int httpResponseCode=-1;
    resetTftDisplay(WIDTH/2 - 6*String("Getting info from").length(),32);
    tft.fillSmoothRoundRect(6,6,WIDTH-12,HEIGHT-12,5,BGCOLOR);
    tft.drawSmoothRoundRect(5,5,5,5,WIDTH-10,HEIGHT-10,FGCOLOR,BGCOLOR);
    tft.drawCentreString("Getting info from", WIDTH/2, HEIGHT/3,1);
    tft.drawCentreString("M5Burner repo", WIDTH/2, HEIGHT/3+FONT_M*9,1);

    tft.setCursor(18,  HEIGHT/3+FONT_M*9*2);
    while(httpResponseCode<0) { 
      http.begin(serverUrl);
      http.useHTTP10(true);
      httpResponseCode = http.GET();
      if (httpResponseCode > 0) {
        DeserializationError error;
        deserializeJson(doc, http.getStream());
        delay(100);
        return true;
        break;
      } else {
        tftprint(".",10);
        http.end();
        delay(1000);
      }
    }
    http.end();
  } return false;
}


/***************************************************************************************
** Function name: downloadFirmware
** Description:   Downloads the firmware and save into the SDCard
***************************************************************************************/
void downloadFirmware(String file_str, String fileName, String folder) {
  String fileAddr = "https://m5burner-cdn.m5stack.com/firmware/" + file_str;
  prog_handler = 2;
  if(!setupSdCard()) {
    displayRedStripe("SDCard Not Found");
    delay(2000);
    return;
  }

  tft.fillRect(7, 40, WIDTH - 14, 88, BGCOLOR); // Erase the information below the firmware name
  displayRedStripe("Connecting FW");

  WiFiClientSecure *client = new WiFiClientSecure;
  client->setCACert(root_ca3);  
  if(client) {
    HTTPClient http;
    int httpResponseCode=-1;

    while(httpResponseCode<0) { 
      http.begin(*client, fileAddr);
      http.useHTTP10(true);
      httpResponseCode = http.GET();
      if (httpResponseCode > 0) {
        setupSdCard();
        if (!SD.exists("/downloads")) SD.mkdir("/downloads");

        File file = SD.open(folder + fileName + ".bin", FILE_WRITE);
        int size = http.getSize();
        displayRedStripe("Downloading FW");
        if(file) {

          int downloaded = 0;
          WiFiClient* stream = http.getStreamPtr();
          int len = size;

          prog_handler = 2; //Download handler

          // Ler dados enquanto disponível
          progressHandler(downloaded, size);
          while (http.connected() && (len > 0 || len == -1)) {
            // Ler dados em partes
            int size_av = stream->available();
            if (size_av) {
              int c = stream->readBytes(buff, std::min(size_av, bufSize));
              file.write(buff, c);

              if (len > 0) {
                len -= c;
              }

              downloaded += c;
              progressHandler(downloaded, size);  // Chama a função de progresso
            }
            delay(1);
          }          

          delay(2500);
        }
        break;
      } else {
        http.end();
        delay(1000);
      }
    }
    http.end();

  } else { displayRedStripe("Couldn't Connect"); }

}

/***************************************************************************************
** Function name: installFirmware
** Description:   installs Firmware using OTA
***************************************************************************************/
void installFirmware(String file, uint32_t app_size, bool spiffs, uint32_t spiffs_offset, uint32_t spiffs_size, bool nb) {
  uint32_t app_offset = 0x10000;

  //Release RAM Memory from Json Objects
  if(spiffs && askSpiffs) {
    options = {
      {"SPIFFS No", [&](){ spiffs = false; }},
      {"SPIFFS Yes", [&](){ spiffs = true; }},
    };
    delay(200);
    loopOptions(options);
  }

  if(spiffs && spiffs_size>MAX_SPIFFS) spiffs_size=MAX_SPIFFS;
  if (app_size>MAX_APP) app_size = MAX_APP;
  String fileAddr;

  tft.fillRect(7, 40, WIDTH - 14, 88, BGCOLOR); // Erase the information below the firmware name
  displayRedStripe("Connecting FW");

  WiFiClientSecure *client = new WiFiClientSecure;
  fileAddr = "https://m5burner-cdn.m5stack.com/firmware/" + file;
  client->setCACert(root_ca3);  
  if(!client) { displayRedStripe("Couldn't Connect *.aliyuncs.com"); goto SAIR; }

  httpUpdate.rebootOnUpdate(false);
  /* Install App */
  prog_handler = 0;
  tft.fillSmoothRoundRect(6,6,WIDTH-12,HEIGHT-12,5,BGCOLOR);
  progressHandler(0, 500);
  httpUpdate.onProgress(progressHandler);
  httpUpdate.setLedPin(LED, LED_ON);

#ifndef STICK_C_PLUS
  //Erase FAT partition
  eraseFAT();
#endif

#ifndef STICK_C
  tft.drawRect(18, HEIGHT - 47, 204, 17, FGCOLOR);        
  if (spiffs) tft.drawRect(18, HEIGHT - 28, 204, 17, ALCOLOR);
#else
  tft.drawRect(28, HEIGHT - 47, 104, 17, FGCOLOR);
  if (spiffs) tft.drawRect(28, HEIGHT - 28, 104, 17, ALCOLOR);
#endif
  if(nb) app_offset = 0;
  if(!httpUpdate.updateFromOffset(*client, fileAddr, app_offset, app_size)) {
    displayRedStripe("Instalation Failed");
    goto SAIR;
  }
  if(!client) { displayRedStripe("Couldn't Connect *.m5stack.com"); goto SAIR; }

  if(spiffs) {
    prog_handler = 1;
    tft.fillRect(5, 60, WIDTH - 10, 16, ALCOLOR);
    setTftDisplay(5, 60, TFT_WHITE, FONT_M, ALCOLOR);

    tft.println(" Preparing SPIFFS");
    // Format Spiffs partition
    if(!SPIFFS.begin(true)) { 
      displayRedStripe("Fail to start SPIFFS");
      delay(2500);
    }
    else  {
      displayRedStripe("Formatting SPIFFS");
      SPIFFS.format();
      SPIFFS.end();
    }
    displayRedStripe("Connecting SPIFFs");
    
    // Install Spiffs
    progressHandler(0, 500);
    httpUpdate.onProgress(progressHandler);
    
    if(!httpUpdate.updateSpiffsFromOffset(*client, fileAddr, spiffs_offset, spiffs_size)) {
      displayRedStripe("SPIFFS Failed -> Restarting");
      delay(2500);
    }
  }

  Sucesso:
  esp_restart();

  // Só chega aqui se der errado
  SAIR:
  delay(5000);
}

