
#include "globals.h"
#include "display.h"
#include "settings.h"
#include "sd_functions.h"
#include "mykeyboard.h"


/*********************************************************************
**  Function: setBrightness                             
**  save brightness value into EEPROM
**********************************************************************/
void setBrightness(int brightval, bool save) {
  if(brightval>100) brightval=100;

  #if defined(STICK_C_PLUS2) || defined(CARDPUTER)
  int bl = MINBRIGHT + round(((255 - MINBRIGHT) * brightval/100 )); ; // 4 is the number of options
  analogWrite(BACKLIGHT, bl);
  #elif defined(STICK_C) || defined(STICK_C_PLUS)
  axp192.ScreenBreath(brightval);
  #elif defined(M5STACK)
  M5.Display.setBrightness(brightval);  
  #endif

 if (save) {
  EEPROM.begin(EEPROMSIZE); // open eeprom
  bright = brightval;
  EEPROM.write(2, brightval); //set the byte
  EEPROM.commit(); // Store data to EEPROM
  EEPROM.end(); // Free EEPROM memory
 }
}

/*********************************************************************
**  Function: getBrightness                             
**  save brightness value into EEPROM
**********************************************************************/
void getBrightness() {
  EEPROM.begin(EEPROMSIZE);
  bright = EEPROM.read(2);
  EEPROM.end(); // Free EEPROM memory
  if(bright>100) { 
    bright = 100;

#if defined(STICK_C_PLUS2) || defined(CARDPUTER)
  int bl = MINBRIGHT + round(((255 - MINBRIGHT) * bright/100 )); 
  analogWrite(BACKLIGHT, bl);
#elif defined(STICK_C) || defined(STICK_C_PLUS)
  axp192.ScreenBreath(bright);
#elif defined(M5STACK)
  M5.Display.setBrightness(bright);  
#endif
    setBrightness(100);
  }

#if defined(STICK_C_PLUS2) || defined(CARDPUTER)
  int bl = MINBRIGHT + round(((255 - MINBRIGHT) * bright/100 )); 
  analogWrite(BACKLIGHT, bl);
#elif defined(STICK_C) || defined(STICK_C_PLUS)
  axp192.ScreenBreath(bright);
#elif defined(M5STACK)
  M5.Display.setBrightness(bright);
#endif

}

/*********************************************************************
**  Function: gsetOnlyBins                             
**  get onlyBins from EEPROM
**********************************************************************/
bool gsetOnlyBins(bool set, bool value) {
  EEPROM.begin(EEPROMSIZE);
  int onlyBin = EEPROM.read(9);
  bool result = false;

  if(onlyBin>1) { 
    set=true;
  } 
   
  if(onlyBin==0) result = false;
  else result = true;

  if(set) {
    result=value;
    onlyBins=value;         //update the global variable
    EEPROM.write(9, result);
    EEPROM.commit();
  }
  EEPROM.end(); // Free EEPROM memory
  return result;
}

/*********************************************************************
**  Function: gsetAskSpiffs                             
**  get onlyBins from EEPROM
**********************************************************************/
bool gsetAskSpiffs(bool set, bool value) {
  EEPROM.begin(EEPROMSIZE);
  int spiffs = EEPROM.read(EEPROMSIZE-1);
  bool result = false;

  if(spiffs>1) { 
    set=true;
  } 
   
  if(spiffs==0) result = false;
  else result = true;

  if(set) {
    result=value;
    askSpiffs=value;         //update the global variable
    EEPROM.write(EEPROMSIZE-1, result);
    EEPROM.commit();
  }
  EEPROM.end(); // Free EEPROM memory
  return result;
}

/*********************************************************************
**  Function: gsetRotation                             
**  get onlyBins from EEPROM
**********************************************************************/
int gsetRotation(bool set){
  EEPROM.begin(EEPROMSIZE);
  int getRot = EEPROM.read(0);
  int result = ROTATION;
  
  if(getRot==1 && set) result = 3;
  else if(getRot==3 && set) result = 1;
  else if(getRot<=3) result = getRot;
  else {
    set=true;
    result = ROTATION;
  } 

  if(set) {
    rotation = result;
    tft.setRotation(result);
    EEPROM.write(0, result);    // Left rotation
    EEPROM.commit();
    tft.fillScreen(BGCOLOR);
  }
  EEPROM.end(); // Free EEPROM memory
  return result;
}
/*********************************************************************
**  Function: setBrightnessMenu                             
**  Handles Menu to set brightness
**********************************************************************/
void setBrightnessMenu() {
  options = {
    {"100%", [=]() { setBrightness(100); }},
    {"75 %", [=]() { setBrightness(75); }},
    {"50 %", [=]() { setBrightness(50); }},
    {"25 %", [=]() { setBrightness(25); }},
    {" 0 %", [=]() { setBrightness(1); }},
  };
  delay(200);
  loopOptions(options, true);
  saveConfigs();
  delay(200);
}
/*********************************************************************
**  Function: setUiColor
**  Change Ui Color scheme
**********************************************************************/
void setUiColor() {
  options = {
    {"Default",   [&]() { FGCOLOR=0x07E0; BGCOLOR=0x0000; ALCOLOR=0xF800; odd_color=0x30c5; even_color=0x32e5; }},
    {"Red",       [&]() { FGCOLOR=0xF800; BGCOLOR=0x0000; ALCOLOR=0xE3E0; odd_color=0xFBC0; even_color=0xAAC0; }},
    {"Blue",      [&]() { FGCOLOR=0x94BF; BGCOLOR=0x0000; ALCOLOR=0xd81f; odd_color=0xd69f; even_color=0x079F; }},
    {"Yellow",    [&]() { FGCOLOR=0xFFE0; BGCOLOR=0x0000; ALCOLOR=0xFB80; odd_color=0x9480; even_color=0xbae0; }},
    {"Purple",    [&]() { FGCOLOR=0xe01f; BGCOLOR=0x0000; ALCOLOR=0xF800; odd_color=0xf57f; even_color=0x89d3; }},
    {"White",     [&]() { FGCOLOR=0xFFFF; BGCOLOR=0x0000; ALCOLOR=0x6b6d; odd_color=0x630C; even_color=0x8410; }},
    {"Black",     [&]() { FGCOLOR=0x0000; BGCOLOR=0xFFFF; ALCOLOR=0x6b6d; odd_color=0x8c71; even_color=0xb596; }},
  };
  delay(200);
  loopOptions(options);
  displayRedStripe("Saving...");
  EEPROM.begin(EEPROMSIZE);

  EEPROM.write(EEPROMSIZE-3, int((FGCOLOR >> 8) & 0x00FF));
  EEPROM.write(EEPROMSIZE-4, int(FGCOLOR & 0x00FF));

  EEPROM.write(EEPROMSIZE-5, int((BGCOLOR >> 8) & 0x00FF));
  EEPROM.write(EEPROMSIZE-6, int(BGCOLOR & 0x00FF));

  EEPROM.write(EEPROMSIZE-7, int((ALCOLOR >> 8) & 0x00FF));
  EEPROM.write(EEPROMSIZE-8, int(ALCOLOR & 0x00FF));

  EEPROM.write(EEPROMSIZE-9, int((odd_color >> 8) & 0x00FF));
  EEPROM.write(EEPROMSIZE-10, int(odd_color & 0x00FF));

  EEPROM.write(EEPROMSIZE-11, int((even_color >> 8) & 0x00FF));
  EEPROM.write(EEPROMSIZE-12, int(even_color & 0x00FF));

  EEPROM.commit();       // Store data to EEPROM
  EEPROM.end();

  saveConfigs();
  delay(200);
}
/*********************************************************************
**  Function: setdimmerSet
**  set dimmerSet time
**********************************************************************/
void setdimmerSet() {
  int time = 20;
  options = {
    {"10s", [&]() { time=10; }},
    {"15s", [&]() { time=15; }},
    {"30s", [&]() { time=30; }},
    {"45s", [&]() { time=45; }},
    {"60s", [&]() { time=60; }},
  };
  delay(200);
  loopOptions(options);
  dimmerSet=time;
  EEPROM.begin(EEPROMSIZE);
  EEPROM.write(1, dimmerSet);  // 20s Dimm time
  EEPROM.commit();
  EEPROM.end();
  saveConfigs();
  delay(200);
}

/*********************************************************************
**  Function: chargeMode
**  Enter in Charging mode
**********************************************************************/
void chargeMode() {
  setCpuFrequencyMhz(80);
  setBrightness(5,false);
  delay(500);
  tft.fillScreen(BGCOLOR);
  unsigned long tmp=0;
  while(!checkSelPress(true)) {
    if(millis()-tmp>5000) {
      displayRedStripe(String(getBattery()) + " %");
      tmp=millis();
    }
  }
  setCpuFrequencyMhz(240);
  setBrightness(bright,false);
  delay(200);
}

/*********************************************************************
**  Function: getConfigs                             
**  getConfigurations from EEPROM or JSON
**********************************************************************/
void getConfigs() {
  if(setupSdCard()) {
    if(!SD.exists("/config.conf")) {
      File conf = SD.open("/config.conf", FILE_WRITE);
      if(conf) {
        #if ROTATION >1
        conf.print("[{\"rot\":3,\"onlyBins\":1,\"bright\":100,\"askSpiffs\":1,\"wui_usr\":\"admin\",\"wui_pwd\":\"m5launcher\",\"dwn_path\":\"/downloads/\",\"FGCOLOR\":2016,\"BGCOLOR\":0,\"ALCOLOR\":63488,\"even\":13029,\"odd\":12485,\"wifi\":[{\"ssid\":\"myNetSSID\",\"pwd\":\"myNetPassword\"}]}]");
        #else
        conf.print("[{\"rot\":1,\"onlyBins\":1,\"bright\":100,\"askSpiffs\":1,\"wui_usr\":\"admin\",\"wui_pwd\":\"m5launcher\",\"dwn_path\":\"/downloads/\",\"FGCOLOR\":2016,\"BGCOLOR\":0,\"ALCOLOR\":63488,\"even\":13029,\"odd\":12485,\"wifi\":[{\"ssid\":\"myNetSSID\",\"pwd\":\"myNetPassword\"}]}]");
        #endif
      }
      conf.close();
    }
    File file = SD.open("/config.conf");
      if(file && file.size()>10) {
        // Deserialize the JSON document
        DeserializationError error = deserializeJson(settings, file);
        if (error) { 
          log_i("Failed to read file, using default configuration");
          goto Default;
        }
        //JsonObject setting = settings[0];
        int count=0;
        if(settings.containsKey("onlyBins"))  { onlyBins  = gsetOnlyBins(settings["onlyBins"].as<bool>()); } else count++;
        if(settings.containsKey("askSpiffs")) { askSpiffs = gsetAskSpiffs(settings["askSpiffs"].as<bool>()); } else count++;
        if(settings.containsKey("bright"))    { bright    = settings["bright"].as<int>(); } else count++;
        if(settings.containsKey("dimmerSet")) { dimmerSet = settings["dimmerSet"].as<int>(); } else count++;
        if(settings.containsKey("rot"))       { rotation  = settings["rot"].as<int>(); } else count++;
        if(settings.containsKey("FGCOLOR"))   { FGCOLOR   = settings["FGCOLOR"].as<uint16_t>(); } else count++;
        if(settings.containsKey("BGCOLOR"))   { BGCOLOR   = settings["BGCOLOR"].as<uint16_t>(); } else count++;
        if(settings.containsKey("ALCOLOR"))   { ALCOLOR   = settings["ALCOLOR"].as<uint16_t>(); } else count++;
        if(settings.containsKey("odd"))       { odd_color = settings["odd"].as<uint16_t>(); } else count++;
        if(settings.containsKey("even"))      { even_color= settings["even"].as<uint16_t>(); } else count++;
        if(settings.containsKey("wui_usr"))   { wui_usr   = settings["wui_usr"].as<String>(); } else count++;
        if(settings.containsKey("wui_pwd"))   { wui_pwd   = settings["wui_pwd"].as<String>(); } else count++;
        if(settings.containsKey("dwn_path"))  { dwn_path  = settings["dwn_path"].as<String>(); } else count++;
        if(!settings.containsKey("wifi")) count++;

        if(count>0) saveConfigs();
        log_i("Brightness: %d", bright);
        setBrightness(bright);
        if(dimmerSet<10) dimmerSet=10;
        file.close();

        
        
        EEPROM.begin(EEPROMSIZE); // open eeprom
        EEPROM.write(0, rotation);
        EEPROM.write(1, dimmerSet);
        EEPROM.write(2, bright);
        EEPROM.write(9, int(onlyBins));
        EEPROM.write(EEPROMSIZE-2, int(askSpiffs));

        EEPROM.write(EEPROMSIZE-3, int((FGCOLOR >> 8) & 0x00FF));
        EEPROM.write(EEPROMSIZE-4, int(FGCOLOR & 0x00FF));

        EEPROM.write(EEPROMSIZE-5, int((BGCOLOR >> 8) & 0x00FF));
        EEPROM.write(EEPROMSIZE-6, int(BGCOLOR & 0x00FF));

        EEPROM.write(EEPROMSIZE-7, int((ALCOLOR >> 8) & 0x00FF));
        EEPROM.write(EEPROMSIZE-8, int(ALCOLOR & 0x00FF));

        EEPROM.write(EEPROMSIZE-9, int((odd_color >> 8) & 0x00FF));
        EEPROM.write(EEPROMSIZE-10, int(odd_color & 0x00FF));

        EEPROM.write(EEPROMSIZE-11, int((even_color >> 8) & 0x00FF));
        EEPROM.write(EEPROMSIZE-12, int(even_color & 0x00FF));
        if(!EEPROM.commit()) log_i("fail to write EEPROM");      // Store data to EEPROM
        EEPROM.end();
        log_i("Using config.conf setup file");
    } else {
Default:
        file.close();
        saveConfigs();
        
        log_i("Using settings stored on EEPROM");
    }
  }
}
/*********************************************************************
**  Function: saveConfigs                             
**  save configs into JSON config.conf file
**********************************************************************/
void saveConfigs() {
  // Delete existing file, otherwise the configuration is appended to the file
  if(setupSdCard()) {
    SD.remove("/config.conf");

    //JsonObject setting = settings[0].add<JsonObject>();
    //if(!settings[0].containsKey("onlyBins")) 
    settings["onlyBins"] = onlyBins;
    settings["askSpiffs"] = askSpiffs;
    settings["bright"] = bright;
    settings["dimmerSet"] = dimmerSet;    
    settings["rot"] = rotation;
    settings["FGCOLOR"] = FGCOLOR;
    settings["BGCOLOR"] = BGCOLOR;
    settings["ALCOLOR"] = ALCOLOR;
    settings["odd"] = odd_color;
    settings["even"] = even_color;
    settings["wui_usr"] = wui_usr;
    settings["wui_pwd"] = wui_pwd;
    settings["dwn_path"] = dwn_path;
    if(!settings.containsKey("wifi")) {
      JsonArray WifiList = settings["wifi"].to<JsonArray>();
      if(WifiList.size()<1) {
        JsonObject WifiObj = WifiList.add<JsonObject>();
        WifiObj["ssid"] = "myNetSSID";
        WifiObj["pwd"] = "myNetPassword";
      } 
    }
    // Open file for writing
    File file = SD.open("/config.conf", FILE_WRITE);
    if (!file) {
      Serial.println(F("Failed to create file"));
      return;
    }
    // Serialize JSON to file
    if (serializeJsonPretty(settings, file) < 5) {
      log_i("Failed to write to file");
    }

    // Close the file
    file.close();
  }
}
