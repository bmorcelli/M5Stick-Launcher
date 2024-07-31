
#include "globals.h"
#include "display.h"
#include "settings.h"
#include "sd_functions.h"
#include "mykeyboard.h"

/**************************************************************************************
EEPROM ADDRESSES MAP


0	N/B Rot 	  16		      32	Pass	  48	Pass	64	Pass	80	Pass	96		112	
1	N/B Dim	    17		      33	Pass	  49	Pass	65	Pass	81	Pass	97		113	
2	N/B Bri     18		      34	Pass  	50	Pass	66	Pass	82	Pass	98		114	
3	N	          19		      35	Pass	  51	Pass	67	Pass	83	Pass	99		115	(L- Brigh)
4	N	          20	Pass  	36	Pass	  52	Pass	68	Pass	84	Pass	100		116	(L- Dim)
5	N	          21	Pass  	37	Pass  	53	Pass	69	Pass	85		    101		117	(L- Rotation)
6	B-IrTX      22	Pass  	38	Pass  	54	Pass	70	Pass	86		    102		118	(L-odd)
7	B-IrRx      23	Pass  	39	Pass  	55	Pass	71	Pass	87		    103		119	(L-odd)
8	B-RfTX      24	Pass  	40	Pass  	56	Pass	72	Pass	88		    104		120	(L-even)
9	B-RfRx      25	Pass  	41	Pass  	57	Pass	73	Pass	89		    105		121	(L-even)
10 TimeZone	  26	Pass  	42	Pass  	58	Pass	74	Pass	90		    106		122	(L-BGCOLOR)
11 FGCOLOR    27	Pass  	43	Pass  	59	Pass	75	Pass	91		    107		123	(L-BGCOLOR)
12 FGCOLOR    28	Pass  	44	Pass  	60	Pass	76	Pass	92		    108		124	(L-FGCOLOR)
13		        29	Pass  	45	Pass  	61	Pass	77	Pass	93		    109		125	(L-FGCOLOR)
14		        30	Pass	  46	Pass  	62	Pass	78	Pass	94		    110		126	(L-AskSpiffs)
15		        31	Pass  	47	Pass  	63	Pass	79	Pass	95		    111		127	(L-OnlyBins)

From 1 to 5: Nemo shared addresses
(L -*) stands for Launcher addresses

***************************************************************************************/


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
  #elif  defined(T_DISPLAY_S3) || defined(CYD) || defined(MARAUDERV4) || defined(MARAUDERMINI)
  int dutyCycle;
  if (brightval==100) dutyCycle=255;
  else if (brightval==75) dutyCycle=130;
  else if (brightval==50) dutyCycle=70;
  else if (brightval==25) dutyCycle=20;
  else if (brightval==0) dutyCycle=5;
  else dutyCycle = ((brightval*255)/100);

  log_i("dutyCycle for bright 0-255: %d",dutyCycle);
  ledcWrite(TFT_BRIGHT_CHANNEL,dutyCycle); // Channel 0
  #endif

 if (save) {
  EEPROM.begin(EEPROMSIZE); // open eeprom
  bright = brightval;
  EEPROM.write(EEPROMSIZE-15, brightval); //set the byte
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
  bright = EEPROM.read(EEPROMSIZE-15);
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
  #elif  defined(T_DISPLAY_S3) || defined(CYD) || defined(MARAUDERV4) || defined(MARAUDERMINI)
  int dutyCycle;
  if (bright==100) dutyCycle=255;
  else if (bright==75) dutyCycle=130;
  else if (bright==50) dutyCycle=70;
  else if (bright==25) dutyCycle=20;
  else if (bright==0) dutyCycle=5;
  else dutyCycle = ((bright*255)/100);
  log_i("dutyCycle for bright 0-255: %d",dutyCycle);
  ledcWrite(TFT_BRIGHT_CHANNEL,dutyCycle); // Channel 0
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
  #elif  defined(T_DISPLAY_S3) || defined(CYD) || defined(MARAUDERV4) || defined(MARAUDERMINI)
  int dutyCycle;
  if (bright==100) dutyCycle=255;
  else if (bright==75) dutyCycle=130;
  else if (bright==50) dutyCycle=70;
  else if (bright==25) dutyCycle=20;
  else if (bright==0) dutyCycle=5;
  else dutyCycle = ((bright*255)/100);
  log_i("dutyCycle for bright 0-255: %d",dutyCycle);
  ledcWrite(TFT_BRIGHT_CHANNEL,dutyCycle); // Channel 0
#endif

}

/*********************************************************************
**  Function: gsetOnlyBins                             
**  get onlyBins from EEPROM
**********************************************************************/
bool gsetOnlyBins(bool set, bool value) {
  EEPROM.begin(EEPROMSIZE);
  int onlyBin = EEPROM.read(EEPROMSIZE-1);
  bool result = false;

  if(onlyBin>1) { 
    set=true;
  } 
   
  if(onlyBin==0) result = false;
  else result = true;

  if(set) {
    result=value;
    onlyBins=value;         //update the global variable
    EEPROM.write(EEPROMSIZE-1, result);
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
  int getRot = EEPROM.read(EEPROMSIZE-13);
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
    EEPROM.write(EEPROMSIZE-13, result);    // Left rotation
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
  EEPROM.write(EEPROMSIZE-14, dimmerSet);  // 20s Dimm time
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
    if(!SDM.exists(CONFIG_FILE)) {
      File conf = SDM.open(CONFIG_FILE, FILE_WRITE);
      if(conf) {
        #if ROTATION >1
        conf.print("[{\"rot\":3,\"dimmerSet\":10,\"onlyBins\":1,\"bright\":100,\"askSpiffs\":1,\"wui_usr\":\"admin\",\"wui_pwd\":\"m5launcher\",\"dwn_path\":\"/downloads/\",\"FGCOLOR\":2016,\"BGCOLOR\":0,\"ALCOLOR\":63488,\"even\":13029,\"odd\":12485,\"wifi\":[{\"ssid\":\"myNetSSID\",\"pwd\":\"myNetPassword\"}]}]");
        #else
        conf.print("[{\"rot\":1,\"dimmerSet\":10,\"onlyBins\":1,\"bright\":100,\"askSpiffs\":1,\"wui_usr\":\"admin\",\"wui_pwd\":\"m5launcher\",\"dwn_path\":\"/downloads/\",\"FGCOLOR\":2016,\"BGCOLOR\":0,\"ALCOLOR\":63488,\"even\":13029,\"odd\":12485,\"wifi\":[{\"ssid\":\"myNetSSID\",\"pwd\":\"myNetPassword\"}]}]");
        #endif
      }
      conf.close();
      delay(50);
    } else log_i("getConfigs: config.conf exists");
    File file = SDM.open(CONFIG_FILE, FILE_READ);
      if(file) {
        // Deserialize the JSON document
        DeserializationError error = deserializeJson(settings, file);
        if (error) { 
          log_i("Failed to read file, using default configuration");
          goto Default;
        } else log_i("getConfigs: deserialized correctly");

        int count=0;
        JsonObject setting = settings[0];
        if(setting.containsKey("onlyBins"))  { onlyBins  = gsetOnlyBins(setting["onlyBins"].as<bool>()); } else { count++; log_i("Fail"); }
        if(setting.containsKey("askSpiffs")) { askSpiffs = gsetAskSpiffs(setting["askSpiffs"].as<bool>()); } else { count++; log_i("Fail"); }
        if(setting.containsKey("bright"))    { bright    = setting["bright"].as<int>(); } else { count++; log_i("Fail"); }
        if(setting.containsKey("dimmerSet")) { dimmerSet = setting["dimmerSet"].as<int>(); } else { count++; log_i("Fail"); }
        if(setting.containsKey("rot"))       { rotation  = setting["rot"].as<int>(); } else { count++; log_i("Fail"); }
        if(setting.containsKey("FGCOLOR"))   { FGCOLOR   = setting["FGCOLOR"].as<uint16_t>(); } else { count++; log_i("Fail"); }
        if(setting.containsKey("BGCOLOR"))   { BGCOLOR   = setting["BGCOLOR"].as<uint16_t>(); } else { count++; log_i("Fail"); }
        if(setting.containsKey("ALCOLOR"))   { ALCOLOR   = setting["ALCOLOR"].as<uint16_t>(); } else { count++; log_i("Fail"); }
        if(setting.containsKey("odd"))       { odd_color = setting["odd"].as<uint16_t>(); } else { count++; log_i("Fail"); }
        if(setting.containsKey("even"))      { even_color= setting["even"].as<uint16_t>(); } else { count++; log_i("Fail"); }
        if(setting.containsKey("wui_usr"))   { wui_usr   = setting["wui_usr"].as<String>(); } else { count++; log_i("Fail"); }
        if(setting.containsKey("wui_pwd"))   { wui_pwd   = setting["wui_pwd"].as<String>(); } else { count++; log_i("Fail"); }
        if(setting.containsKey("dwn_path"))  { dwn_path  = setting["dwn_path"].as<String>(); } else { count++; log_i("Fail"); }
        if(!setting.containsKey("wifi"))  { count++; log_i("Fail"); }
        if(count>0) saveConfigs();

        log_i("Brightness: %d", bright);
        setBrightness(bright);
        if(dimmerSet<10) dimmerSet=10;
        file.close();

        
        
        EEPROM.begin(EEPROMSIZE); // open eeprom
        EEPROM.write(EEPROMSIZE-13, rotation);
        EEPROM.write(EEPROMSIZE-14, dimmerSet);
        EEPROM.write(EEPROMSIZE-15, bright);
        EEPROM.write(EEPROMSIZE-1, int(onlyBins));
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
    if(SDM.remove(CONFIG_FILE)) log_i("config.conf deleted");
    else log_i("fail deleting config.conf");

    JsonObject setting = settings[0];
    //if(!settings[0].containsKey("onlyBins")) 
    setting["onlyBins"] = onlyBins;
    setting["askSpiffs"] = askSpiffs;
    setting["bright"] = bright;
    setting["dimmerSet"] = dimmerSet;    
    setting["rot"] = rotation;
    setting["FGCOLOR"] = FGCOLOR;
    setting["BGCOLOR"] = BGCOLOR;
    setting["ALCOLOR"] = ALCOLOR;
    setting["odd"] = odd_color;
    setting["even"] = even_color;
    setting["wui_usr"] = wui_usr;
    setting["wui_pwd"] = wui_pwd;
    setting["dwn_path"] = dwn_path;
    if(!setting.containsKey("wifi")) {
      JsonArray WifiList = setting["wifi"].to<JsonArray>();
      if(WifiList.size()<1) {
        JsonObject WifiObj = WifiList.add<JsonObject>();
        WifiObj["ssid"] = "myNetSSID";
        WifiObj["pwd"] = "myNetPassword";
      } 
    }
    // Open file for writing
    File file = SDM.open(CONFIG_FILE, FILE_WRITE);
    if (!file) {
      log_i("Failed to create file");
      file.close();
      return;
    } else log_i("config.conf created");
    // Serialize JSON to file
    if (serializeJsonPretty(settings, file) < 5) {
      log_i("Failed to write to file");
    } else log_i("config.conf written successfully");

    // Close the file
    file.close();
  }
}
