
#include "globals.h"
#include "display.h"
#include "settings.h"
#include "sd_functions.h"


/*********************************************************************
**  Function: setBrightness                             
**  save brightness value into EEPROM
**********************************************************************/
void setBrightness(int bright, bool save) {
  if(bright>100) bright=100;

  #if defined(STICK_C_PLUS2) || defined(CARDPUTER)
  int bl = MINBRIGHT + round(((255 - MINBRIGHT) * bright/100 )); ; // 4 is the number of options
  analogWrite(BACKLIGHT, bl);
  #elif defined(STICK_C) || defined(STICK_C_PLUS)
  axp192.ScreenBreath(bright);
  #elif defined(M5STACK)
  M5.Display.setBrightness(bright);  
  #endif

 
  EEPROM.begin(EEPROMSIZE); // open eeprom
  EEPROM.write(2, bright); //set the byte
  EEPROM.commit(); // Store data to EEPROM
  EEPROM.end(); // Free EEPROM memory
}

/*********************************************************************
**  Function: getBrightness                             
**  save brightness value into EEPROM
**********************************************************************/
void getBrightness() {
  EEPROM.begin(EEPROMSIZE);
  int bright = EEPROM.read(2);
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
  int spiffs = EEPROM.read(EEPROMSIZE-2);
  bool result = false;

  if(spiffs>1) { 
    set=true;
  } 
   
  if(spiffs==0) result = false;
  else result = true;

  if(set) {
    result=value;
    askSpiffs=value;         //update the global variable
    EEPROM.write(EEPROMSIZE-2, result);
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
**  Function: getConfigs                             
**  getConfigurations from EEPROM or JSON
**********************************************************************/
void getConfigs() {
  EEPROM.begin(EEPROMSIZE); // open eeprom
  if(EEPROM.read(0) > 3 || EEPROM.read(2) > 100 || EEPROM.read(9) > 1 || EEPROM.read(EEPROMSIZE-2) > 1) {
  #if defined(CARDPUTER) || defined(M5STACK)
    EEPROM.write(0, 1);    // Right rotation for cardputer
  #else
    EEPROM.write(0, 3);    // Left rotation
  #endif
    EEPROM.write(2, 100);  // 100% brightness
    EEPROM.write(9, 1);    // OnlyBins
    EEPROM.write(EEPROMSIZE-2, 1);  // AskSpiffs
    EEPROM.writeString(10,"");
    EEPROM.commit();       // Store data to EEPROM
  }
  if(EEPROM.read(0) != 1 && EEPROM.read(0) != 3)  { 
    EEPROM.write(0, 3);    // Left rotation
    EEPROM.commit();       // Store data to EEPROM
  }
  EEPROM.end(); // Free EEPROM memory


  if(setupSdCard()) {
    File file = SD.open("/config.conf");
      if(file) {
        // Deserialize the JSON document
        DeserializationError error = deserializeJson(settings, file);
        if (error) { 
          log_i("Failed to read file, using default configuration");
          goto Default;
        }
        JsonObject setting = settings[0];

        onlyBins = gsetOnlyBins(setting["onlyBins"].as<bool>());
        askSpiffs = gsetAskSpiffs(setting["askSpiffs"].as<bool>());
        int bright = setting["bright"].as<int>();
        log_i("Brightness: %d", bright);
        setBrightness(bright);
        rotation = setting["rot"].as<int>();
        FGCOLOR = setting["FGCOLOR"].as<uint16_t>();
        BGCOLOR = setting["BGCOLOR"].as<uint16_t>();
        ALCOLOR = setting["ALCOLOR"].as<uint16_t>();
        odd_color = setting["odd"].as<uint16_t>();
        even_color = setting["even"].as<uint16_t>();
        wui_usr = setting["wui_usr"].as<String>();
        wui_pwd = setting["wui_pwd"].as<String>();
        dwn_path = setting["dwn_path"].as<String>();
        file.close();
        log_i("Using config.conf setup file");
        return;
    } else {
Default:
        onlyBins = gsetOnlyBins();
        askSpiffs = gsetAskSpiffs();
        getBrightness();
        rotation = gsetRotation();
        log_i("Using settings stored on EEPROM");
        return;
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

    JsonObject setting = settings[0];
    setting["onlyBins"] = onlyBins;
    setting["askSpiffs"] = askSpiffs;
    EEPROM.begin(EEPROMSIZE);
    setting["bright"] = EEPROM.read(2);
    EEPROM.end();
    setting["rot"] = rotation;
    setting["FGCOLOR"] = FGCOLOR;
    setting["BGCOLOR"] = BGCOLOR;
    setting["ALCOLOR"] = ALCOLOR;
    setting["odd"] = odd_color;
    setting["even"] = even_color;
    setting["wui_usr"] = wui_usr;
    setting["wui_pwd"] = wui_pwd;
    setting["dwn_path"] = dwn_path;
    // Open file for writing
    File file = SD.open("/config.conf", FILE_WRITE);
    if (!file) {
      Serial.println(F("Failed to create file"));
      return;
    }
    // Serialize JSON to file
    if (serializeJsonPretty(settings, file) == 0) {
      Serial.println(F("Failed to write to file"));
    }

    // Close the file
    file.close();
  }
}