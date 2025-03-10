
#include "webInterface.h"
#include <globals.h>
#include "sd_functions.h"
#include "onlineLauncher.h"
#include "display.h"
#include "mykeyboard.h"
#include "settings.h"
#include "esp_task_wdt.h"

struct Config {
  String httpuser;
  String httppassword;       // password to access web admin
  int webserverporthttp;     // http port number for web admin
};

// variables
// command = U_SPIFFS = 100
// command = U_FLASH = 0
int command = 0;
bool updateFromSd_var = false;

  // WiFi as a Client
const int default_webserverporthttp = 80;

//WiFi as an Access Point
IPAddress AP_GATEWAY(172, 0, 0, 1);  // Gateway

Config config;                        // configuration

AsyncWebServer *server;               // initialise webserver
const char* host = "launcher";
bool shouldReboot = false;            // schedule a reboot
String uploadFolder="";


/**********************************************************************
**  Function: webUIMyNet
**  Display options to launch the WebUI
**********************************************************************/
void webUIMyNet() {
  if (WiFi.status() != WL_CONNECTED) {
    int nets;
    WiFi.mode(WIFI_MODE_STA);
    displayRedStripe("Scanning...");
    nets=WiFi.scanNetworks();
    options = { };
    for(int i=0; i<nets; i++){
      options.push_back({WiFi.SSID(i).c_str(), [=]() { startWebUi(WiFi.SSID(i).c_str(),int(WiFi.encryptionType(i))); }});
    }
    options.push_back({"Hidden SSID", [=]() { String __ssid=keyboard("", 32, "Your SSID"); wifiConnect(__ssid.c_str(),8); }});
    options.push_back({"Main Menu", [=]() { returnToMenu=true; }});
    loopOptions(options);

  } else {
    //If it is already connected, just start the network
    startWebUi("",0,false); 
  }
  // On fail installing will run the following line
}


/**********************************************************************
**  Function: loopOptionsWebUi
**  Display options to launch the WebUI
**********************************************************************/
void loopOptionsWebUi() {
  // Definição da matriz "Options"
  std::vector<std::pair<std::string, std::function<void()>>> options = {
      {"my Network", [=]() { webUIMyNet(); }},
      {"AP mode", [=]()    { startWebUi("Launcher", 0, true); }},
      {"Main Menu", [=]() { returnToMenu=true; }},
  };

  loopOptions(options);
  // On fail installing will run the following line
}


// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
String humanReadableSize(uint64_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " kB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

// list all of the files, if ishtml=true, return html rather than simple text
String listFiles(String folder) {
  //log_i("Listfiles Start");
  String returnText = "pa:" + folder + ":0\n";
  Serial.println("Listing files stored on SD");

  File root = SDM.open(folder);
  File foundfile = root.openNextFile();
  if (folder=="//") folder = "/";
  uploadFolder = folder;
  String PreFolder = folder;
  PreFolder = PreFolder.substring(0, PreFolder.lastIndexOf("/"));
  if(PreFolder=="") PreFolder= "/";

  if (folder=="/") folder = "";
  while (foundfile) {
    if(esp_get_free_heap_size()>(String("Fo:" + String(foundfile.name()) + ":0\n").length())+1024) {
      if(foundfile.isDirectory()) returnText+="Fo:" + String(foundfile.name()) + ":0\n";
    } else break;
    foundfile = root.openNextFile();
    esp_task_wdt_reset();
  }
  root.close();
  foundfile.close();

  if (folder=="") folder = "/";
  root = SDM.open(folder);
  foundfile = root.openNextFile();
  while (foundfile) {
    if(esp_get_free_heap_size()>(String("Fo:" + String(foundfile.name()) + ":0\n").length())+1024) {
      if(!(foundfile.isDirectory())) returnText+="Fi:" + String(foundfile.name()) + ":" + humanReadableSize(foundfile.size()) + "\n";
    } else break;
    foundfile = root.openNextFile();
    esp_task_wdt_reset();
  }
  root.close();
  foundfile.close();

  //log_i("ListFiles End");
  return returnText;
}

// used by server.on functions to discern whether a user has the correct httpapitoken OR is authenticated by username and password
bool checkUserWebAuth(AsyncWebServerRequest * request) {
  bool isAuthenticated = false;
  if (request->authenticate(config.httpuser.c_str(), config.httppassword.c_str())) {
    isAuthenticated = true;
  }
  return isAuthenticated;
}
bool runOnce = false;
// handles uploads to the filserver
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  // make sure authenticated before allowing upload
  Serial.println("Folder: " + uploadFolder);  
  if (uploadFolder=="/") uploadFolder = "";

  if (checkUserWebAuth(request)) {
    if (!index || runOnce) {
      if(!update) {
        // open the file on first call and store the file handle in the request object
        request->_tempFile = SDM.open(uploadFolder + "/" + filename, "w");
      } else {
        runOnce=false;
        // open the file on first call and store the file handle in the request object
        if(Update.begin(file_size,command)) {
          if(command == 0) prog_handler = 0;
          else prog_handler = 1;

          progressHandler(0, 500);
          Update.onProgress(progressHandler);
        } else { displayRedStripe("FAIL 160: " + String(Update.getError())); delay(3000); }
      }

    }

    if (len) {
      // stream the incoming chunk to the opened file
      if(!update) {
        request->_tempFile.write(data, len);
      } else {
        if(!Update.write(data,len)) displayRedStripe("FAIL 172");
      }
    }

    if (final) {
      if(!update) {
        // close the file handle as the upload is now done
        request->_tempFile.close();
        request->redirect("/");
      } else {

        if(!Update.end()) { displayRedStripe("Fail 183: " + String(Update.getError())); delay(3000); }
        else displayRedStripe("Restart your device");
      }
    }
  } else {
    return request->requestAuthentication();
  }
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}


void configureWebServer() {
  // configure web server
  
  MDNS.begin(host);
  
  // if url isn't found
  server->onNotFound([](AsyncWebServerRequest * request) {
    request->redirect("/");
  });

  // visiting this page will cause you to be logged out
  server->on("/logout", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->requestAuthentication();
    request->send(401);
  });

  // presents a "you are now logged out webpage
  server->on("/logged-out", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);
    #ifdef PART_04MB
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", "", 0);
    #else
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", logout_html, logout_html_size);
    response->addHeader("Content-Encoding", "gzip");
    #endif
    request->send(response);

  });

  server->on("/UPDATE", HTTP_POST, [](AsyncWebServerRequest * request) {
      if (request->hasParam("fileName", true))  { 
        fileToCopy = request->getParam("fileName", true)->value().c_str();
        request->send(200, "text/plain", "Starting Update");
        updateFromSd_var=true;
      }
  });

  server->on("/rename", HTTP_POST, [](AsyncWebServerRequest * request) {
      if (request->hasParam("fileName", true) && request->hasParam("filePath", true))  { 
        String fileName = request->getParam("fileName", true)->value().c_str();
        String filePath = request->getParam("filePath", true)->value().c_str();
        String filePath2 = filePath.substring(0,filePath.lastIndexOf('/')+1) + fileName;
        if(!setupSdCard()) {
            request->send(200, "text/plain", "Fail starting SD Card.");
        } else {
          // Rename the file of folder
          if (SDM.rename(filePath, filePath2)) {
              request->send(200, "text/plain", filePath + " renamed to " + filePath2);
          } else {
              request->send(200, "text/plain", "Fail renaming file.");
          }
        }
      }
  });
  server->on("/OTAFILE", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Aqui você pode tratar parâmetros que não são parte do upload
  }, handleUpload);
  server->on("/OTA", HTTP_POST, [](AsyncWebServerRequest * request) {
      if (request->hasParam("update", true))  { 
        update = true;
        request->send(200, "text/plain", "Update");
      }
      
      if (request->hasParam("command", true))  { 
        command = request->getParam("command", true)->value().toInt();
        if (request->hasParam("size", true)) {
          file_size = request->getParam("size", true)->value().toInt();
          if(file_size>0) {
            update=true;
            runOnce=true;
            request->send(200, "text/plain", "OK");
          }
        }
      }
  });

  // run handleUpload function when any file is uploaded
  server->onFileUpload(handleUpload);


  server->on("/scripts.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (checkUserWebAuth(request)) {
      AsyncWebServerResponse *response = request->beginResponse_P(200, "application/javascript", scripts_js, scripts_js_size);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    } else {
      return request->requestAuthentication();
    }
  });
  server->on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (checkUserWebAuth(request)) {
      #ifdef PART_04MB
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", style_4mb_css, style_4mb_css_size);
      #else
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", style_css, style_css_size);
      #endif
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    } else {
      return request->requestAuthentication();
    }
  });
  server->on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (checkUserWebAuth(request)) {
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html, index_html_size);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    } else {
      return request->requestAuthentication();
    }
  });
  server->on("/systeminfo", HTTP_GET,[](AsyncWebServerRequest * request) {
    char response_body[300];
    uint64_t SDTotalBytes = SDM.totalBytes();
    uint64_t SDUsedBytes = SDM.usedBytes();
    sprintf(response_body,
      "{\"%s\":\"%s\",\"SD\":{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}}",
      "VERSION", LAUNCHER,
      "free", humanReadableSize(SDTotalBytes - SDUsedBytes).c_str(),
      "used", humanReadableSize(SDUsedBytes).c_str(),
      "total", humanReadableSize(SDTotalBytes).c_str()
    );
    request->send(200, "application/json", response_body);
  });
  server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (checkUserWebAuth(request)) {
      shouldReboot = true;
      request->send(200, "text/html", "Rebooting");
    } else {
      return request->requestAuthentication();
    }
  });

  server->on("/listfiles", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    if (checkUserWebAuth(request)) {
      update = false;
      String folder = "";
      if (request->hasParam("folder")) {
        folder = request->getParam("folder")->value().c_str();
      } else {
        String folder = "/";
      }
      request->send(200, "text/plain", listFiles(folder));

    } else {
      return request->requestAuthentication();
    }
  });

  server->on("/file", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (checkUserWebAuth(request)) {
      if (request->hasParam("name") && request->hasParam("action")) {
        const char *fileName = request->getParam("name")->value().c_str();
        const char *fileAction = request->getParam("action")->value().c_str();

        if (!SDM.exists(fileName)) {
          if (strcmp(fileAction, "create") == 0) {
            //log_i("New Folder: %s",fileName);
            if(!SDM.mkdir(fileName)) { request->send(200, "text/plain", "FAIL creating folder: " + String(fileName));}
            else { request->send(200, "text/plain", "Created new folder: " + String(fileName)); }
          } 
          else {
              request->send(400, "text/plain", "ERROR: file does not exist");
          }
        } 
        else {
          if (strcmp(fileAction, "download") == 0) {
            request->send(SDM, fileName, "application/octet-stream");
          } else if (strcmp(fileAction, "delete") == 0) {
            if(deleteFromSd(fileName)) { request->send(200, "text/plain", "Deleted : " + String(fileName)); }
            else { request->send(200, "text/plain", "FAIL delating: " + String(fileName));}
            
          } else if (strcmp(fileAction, "create") == 0) {
            //i("Folder Exists: %s",fileName);
            if(SDM.mkdir(fileName)) {
            } else { request->send(200, "text/plain", "FAIL creating existing folder: " + String(fileName));}
            request->send(200, "text/plain", "Created new folder: " + String(fileName));
          } else {
            request->send(400, "text/plain", "ERROR: invalid action param supplied");
          }
        }
      } else {
        request->send(400, "text/plain", "ERROR: name and action params required");
      }
    } else {
      return request->requestAuthentication();
    }
  });

  server->on("/sdpins", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (checkUserWebAuth(request)) {
      if (request->hasParam("miso") && request->hasParam("mosi") && request->hasParam("sck") && request->hasParam("cs")) {
        #if defined(HEADLESS)
        int miso = request->getParam("miso")->value().toInt();
        int mosi = request->getParam("mosi")->value().toInt();
        int sck = request->getParam("sck")->value().toInt();
        int cs = request->getParam("cs")->value().toInt();

        // Verifica se os pinos são válidos (entre 0 e 44)
        if (miso > 44 || mosi > 44 || sck > 44 || cs > 44 ||  miso < 0 || mosi < 0 || sck < 0 || cs < 0) {
            request->send(200, "text/plain", "Pins not configured.");
            goto error;
        }

        // Grava os valores no EEPROM
        EEPROM.begin(EEPROMSIZE + 32);
        EEPROM.write(90, static_cast<uint8_t>(miso));
        EEPROM.write(91, static_cast<uint8_t>(mosi));
        EEPROM.write(92, static_cast<uint8_t>(sck));
        EEPROM.write(93, static_cast<uint8_t>(cs));
        EEPROM.commit();
        EEPROM.end();
        _sck=sck;
        _miso=miso;
        _mosi=mosi;
        _cs=cs;
        setupSdCard();
        request->send(200, "text/plain", "Pins configured.");
        error:
        delay(1);
        #else
        request->send(200, "text/plain", "Functionality exclusive for Headless environment (devices with no screen)");
        #endif
      }
    }
    else {
        return request->requestAuthentication();
    }
  });

  server->on("/wifi", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (checkUserWebAuth(request)) {
      if (request->hasParam("usr") && request->hasParam("pwd")) {
        const char *usr = request->getParam("usr")->value().c_str();
        const char *pwdd = request->getParam("pwd")->value().c_str();
        wui_pwd = pwdd;
        wui_usr = usr;
        saveConfigs();
        config.httpuser = usr;
        config.httppassword = pwdd;
        
        request->send(200, "text/plain", "User: " + String(ssid) + " configured with password: " + String(pwd));
      }
      else if(request->hasParam("ssid") && request->hasParam("pwd")) {
        const char *ssidd = request->getParam("ssid")->value().c_str();
        const char *pwdd = request->getParam("pwd")->value().c_str();
        pwd = pwdd;
        ssid = ssidd;
        saveConfigs();
      }
    }
       else {
        return request->requestAuthentication();
    }
  });
}

String readLineFromFile(File myFile) {
  String line = "";
  char character;

  while (myFile.available()) {
    character = myFile.read();
    if (character == ';') {
      break;
    }
    line += character;
  }
  return line;
}


#ifndef HEADLESS
void startWebUi(String ssid, int encryptation, bool mode_ap) {
  file_size = 0;
  //log_i("Recovering User info from config.conf");
  getConfigs();
  config.httpuser     = wui_usr;
  config.httppassword = wui_pwd;
  config.webserverporthttp = default_webserverporthttp;

  //log_i("Connecting to WiFi");
  if (WiFi.status() != WL_CONNECTED) {
    // Choose wifi access mode
    wifiConnect(ssid, encryptation, mode_ap);
  }
  
  // configure web server
  //log_i("Configuring WebServer");
  Serial.println("Configuring Webserver ...");
  server = new AsyncWebServer(config.webserverporthttp);
  configureWebServer();

  // startup web server
  server->begin();
  delay(500);

  tft->drawRoundRect(5,5,tftWidth-10,tftHeight-10,5,ALCOLOR);
  tft->fillRoundRect(6,6,tftWidth-12,tftHeight-12,5,BGCOLOR);
  setTftDisplay(7,7,ALCOLOR,FP,BGCOLOR);
  tft->drawCentreString("-= Launcher WebUI =-",tftWidth/2,0,8);
  String txt;
  if(!mode_ap) txt = WiFi.localIP().toString();
  else txt = WiFi.softAPIP().toString();
  
#if TFT_HEIGHT<200
  tft->drawCentreString("http://launcher.local", tftWidth/2,17,1);
  setTftDisplay(7,26,~BGCOLOR,FP,BGCOLOR);
#else
  tft->drawCentreString("http://launcher.local", tftWidth/2,22,1);
  setTftDisplay(7,47,~BGCOLOR,FP,BGCOLOR);
#endif
  tft->setTextSize(FM);
  tft->print("IP ");   tftprintln(txt,10,1);
  tftprintln("Usr: " + String(wui_usr),10,1);
  tftprintln("Pwd: " + String(wui_pwd),10,1);

  setTftDisplay(7,tftHeight-39,ALCOLOR,FP);

  tft->drawCentreString("press " + String(BTN_ALIAS) + " to stop", tftWidth/2,tftHeight-15,1);
  tft_flush();

  while (!check(SelPress))
  {
    if (shouldReboot) {
      ESP.restart();
    }    
    //Perform installation from SD Card
    if (updateFromSd_var) {
        //log_i("Starting Update from SD");
        updateFromSD(fileToCopy);
        updateFromSd_var = false;
        fileToCopy = "";
        displayRedStripe("Restart your Device");
    }
  }

  //log_i("Closing Server and turning off WiFi");
  server->reset();
  server->end();
  delay(100);
  delete server;
  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true,true);
  WiFi.mode(WIFI_OFF);
  stopOta = true; // used to verify if webUI was opened before to stop OTA and request restart
  
  tft->fillScreen(BGCOLOR);
}

#else 

void startWebUi(String ssid, int encryptation, bool mode_ap) {
  file_size = 0;

  config.httpuser     = wui_usr;
  config.httppassword = wui_pwd;
  config.webserverporthttp = default_webserverporthttp;

  if (WiFi.status() != WL_CONNECTED) {
    // Choose wifi access mode
    wifiConnect(ssid, encryptation, mode_ap);
  }
  
  // configure web server
  //log_i("Configuring WebServer");
  Serial.println("Configuring Webserver ...");
  server = new AsyncWebServer(config.webserverporthttp);
  configureWebServer();

  // startup web server
  server->begin();
  delay(500);

  String txt;
  if(!mode_ap) txt = WiFi.localIP().toString();
  else txt = WiFi.softAPIP().toString();
  
  Serial.println("Access: http://launcher.local");
  Serial.print("IP ");   Serial.println(txt);
  Serial.println("Usr: " + String(wui_usr));
  Serial.println("Pwd: " + String(wui_pwd));

  while (1)
  {
    if (shouldReboot) {
      ESP.restart();
    }    
    //Perform installation from SD Card
    if (updateFromSd_var) {
        //log_i("Starting Update from SD");
        updateFromSD(fileToCopy);
        updateFromSd_var = false;
        fileToCopy = "";
        Serial.println("\n\n--------------------\nRestart your Device");
    }
  }

  log_i("Closing Server and turning off WiFi, something went wrong?");
  server->reset();
  server->end();
  delay(100);
  delete server;
  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true,true);
  WiFi.mode(WIFI_OFF);
}

#endif

