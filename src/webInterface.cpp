
#include "webInterface.h"
#include "globals.h"
#include "sd_functions.h"
#include "onlineLauncher.h"
#include "display.h"
#include "mykeyboard.h"

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
String default_httpuser = "admin";  
String default_httppassword = "m5launcher";
const int default_webserverporthttp = 80;

//WiFi as an Access Point
IPAddress AP_GATEWAY(172, 0, 0, 1);  // Gateway

Config config;                        // configuration

AsyncWebServer *server;               // initialise webserver
const char* host = "m5launcher";
const String fileconf = "/M5Launcher.txt";
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
    delay(200);
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
      {"AP mode", [=]()    { startWebUi("M5Launcher", 0, true); }},
  };
  delay(200);

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

  File root = SD.open(folder);
  File foundfile = root.openNextFile();
  if (folder=="//") folder = "/";
  uploadFolder = folder;
  String PreFolder = folder;
  PreFolder = PreFolder.substring(0, PreFolder.lastIndexOf("/"));
  if(PreFolder=="") PreFolder= "/";

  if (folder=="/") folder = "";
  while (foundfile) {
    if(foundfile.isDirectory()) returnText+="Fo:" + String(foundfile.name()) + ":0\n";
    foundfile = root.openNextFile();
  }
  root.close();
  foundfile.close();

  if (folder=="") folder = "/";
  root = SD.open(folder);
  foundfile = root.openNextFile();
  while (foundfile) {
    if(!(foundfile.isDirectory())) returnText+="Fi:" + String(foundfile.name()) + ":" + humanReadableSize(foundfile.size()) + "\n";
    foundfile = root.openNextFile();
  }
  root.close();
  foundfile.close();

  //log_i("ListFiles End");
  return returnText;
}

// parses and processes webpages
// if the webpage has %SOMETHING% or %SOMETHINGELSE% it will replace those strings with the ones defined
String processor(const String& var) {
  if (var == "FIRMWARE") return String(LAUNCHER);
  else if (var == "FREESD") return humanReadableSize(SD.totalBytes() - SD.usedBytes());
  else if (var == "USEDSD") return humanReadableSize(SD.usedBytes());
  else if (var == "TOTALSD") return humanReadableSize(SD.totalBytes());
  else return "Attribute not configured"; 
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
        request->_tempFile = SD.open(uploadFolder + "/" + filename, "w");
      } else {
        runOnce=false;
        // open the file on first call and store the file handle in the request object
        if(Update.begin(file_size,command)) {
          if(command == 0) prog_handler = 0;
          else prog_handler = 1;
        #ifndef STICK_C_PLUS
          //Erase FAT partition
          //eraseFAT();
        #endif
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
      enableCore0WDT();
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
    request->send_P(401, "text/html", logout_html, processor);

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
        if(!SD.begin()) {
            request->send(200, "text/plain", "Fail starting SD Card.");
        } else {
          // Rename the file of folder
          if (SD.rename(filePath, filePath2)) {
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


  server->on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (checkUserWebAuth(request)) {
      request->send_P(200, "text/html", index_html, processor);
    } else {
      return request->requestAuthentication();
    }
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

        if (!SD.exists(fileName)) {
          if (strcmp(fileAction, "create") == 0) {
            //log_i("New Folder: %s",fileName);
            if(!SD.mkdir(fileName)) { request->send(200, "text/plain", "FAIL creating folder: " + String(fileName));}
            else { request->send(200, "text/plain", "Created new folder: " + String(fileName)); }
          } 
          else {
              request->send(400, "text/plain", "ERROR: file does not exist");
          }
        } 
        else {
          if (strcmp(fileAction, "download") == 0) {
            request->send(SD, fileName, "application/octet-stream");
          } else if (strcmp(fileAction, "delete") == 0) {
            if(deleteFromSd(fileName)) { request->send(200, "text/plain", "Deleted : " + String(fileName)); }
            else { request->send(200, "text/plain", "FAIL delating: " + String(fileName));}
            
          } else if (strcmp(fileAction, "create") == 0) {
            //i("Folder Exists: %s",fileName);
            if(SD.mkdir(fileName)) {
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

  server->on("/wifi", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (checkUserWebAuth(request)) {
      if (request->hasParam("usr") && request->hasParam("pwd")) {
        const char *ssid = request->getParam("usr")->value().c_str();
        const char *pwd = request->getParam("pwd")->value().c_str();
        SD.remove(fileconf);
        File file = SD.open(fileconf, FILE_WRITE);        
        file.print(String(ssid) + ";" + String(pwd) + ";\n");
        config.httpuser = ssid;
        config.httppassword = pwd;
        file.print("#ManagerUser;ManagerPassword;");
        file.close();
        request->send(200, "text/plain", "User: " + String(ssid) + " configured with password: " + String(pwd));
      }
      } else {
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



void startWebUi(String ssid, int encryptation, bool mode_ap) {


  config.httpuser     = default_httpuser;
  config.httppassword = default_httppassword;
  config.webserverporthttp = default_webserverporthttp;
  file_size = 0;
  //log_i("Recovering User info from M5Launcher.txt");
  if(setupSdCard()) {
    if(SD.exists(fileconf)) {
      Serial.println("File Exists, reading " + fileconf);
      File file = SD.open(fileconf, FILE_READ);
      if(file) {
        default_httpuser = readLineFromFile(file);
        default_httppassword = readLineFromFile(file);
        config.httpuser     = default_httpuser;
        config.httppassword = default_httppassword;

        file.close();
      }
    }
    else {
      File file = SD.open(fileconf, FILE_WRITE);
      file.print( default_httpuser + ";" + default_httppassword + ";\n");
      file.print("#ManagerUser;ManagerPassword;");
      file.close();
    }
  }

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

  //log_i("Disabling WatchDogs");
  disableCore0WDT(); // disable WDT
  disableCore1WDT(); // disable WDT
  disableLoopWDT();  // disable WDT

  tft.drawSmoothRoundRect(5,5,5,5,WIDTH-10,HEIGHT-10,ALCOLOR,BGCOLOR);
  tft.fillSmoothRoundRect(6,6,WIDTH-12,HEIGHT-12,5,BGCOLOR);
  setTftDisplay(7,7,ALCOLOR,FONT_P,BGCOLOR);
  tft.drawCentreString("-= M5Launcher WebUI =-",WIDTH/2,0,8);
  String txt;
  if(!mode_ap) txt = WiFi.localIP().toString();
  else txt = WiFi.softAPIP().toString();
  
#ifndef STICK_C
  tft.drawCentreString("http://m5launcher.local", WIDTH/2,22,1);
  setTftDisplay(7,47,TFT_WHITE,FONT_P,BGCOLOR);
#else
  tft.drawCentreString("http://m5launcher.local", WIDTH/2,17,1);
  setTftDisplay(7,26,TFT_WHITE,FONT_P,BGCOLOR);
#endif
  tft.setTextSize(FONT_M);
  tft.print("IP ");   tftprintln(txt,10,1);
  tftprintln("Usr: " + String(default_httpuser),10,1);
  tftprintln("Pwd: " + String(default_httppassword),10,1);

  setTftDisplay(7,HEIGHT-39,ALCOLOR,FONT_P);

  tft.drawCentreString("press " + String(BTN_ALIAS) + " to stop", WIDTH/2,HEIGHT-15,1);

  while (!checkSelPress())
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
  
  tft.fillScreen(BGCOLOR);
}

