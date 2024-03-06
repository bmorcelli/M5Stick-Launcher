/*            M5StickCPlus2 Simple Launcher -> Discord: @bmorcelli - Pirata#5263              */

// ================================ Choose your Destiny ================================
// #define STICK_C         // 4Mb of Flash Memory -> Need custom partitioning (mandatory)
// #define STICK_C_PLUS    // 4Mb of Flash Memory -> Need custom partitioning (mandatory)
// #define STICK_C_PLUS2   // 8Mb of Flash Memory -> Need custom partitioning (mandatory)
// #define CARDPUTER       // 8Mb of Flash Memory -> Need custom partitioning (mandatory)
// ================================== Flawless Victory ==================================

#define LAUNCHER_VERSION "1.2.e"

#if !defined(CARDPUTER) && !defined(STICK_C_PLUS2) && !defined(STICK_C_PLUS) && !defined(STICK_C) && !defined(STICK_C_PLUSv1)
#define STICK_C_PLUS2
#endif

/* -=-=-=-=-=-=-=-=-= Including Libraries =-=-=-=-=-=-=-=-=- */
#include "M5GFX.h"
#include <SPI.h>
#include <FS.h>
#include <SdFat.h>
#include <stdint.h>
#include <SD.h>
#include "esp_ota_ops.h"


#ifndef STICK_C
  #include "M5Launcher-image.h"
#endif

#if defined(STICK_C)
//  #include <M5StackUpdater.h>              // This one must be the last to declare in all devices, except STICK_C! such a noughty boy! 
#endif
#if defined(STICK_C)

#include <M5StickC.h>
  #define LNSD_CLK_PIN 0
  #define LNSD_MISO_PIN 36
  #define LNSD_MOSI_PIN 26
  #define LNSD_CS_PIN 14  //can be -1, but sends a lot of messages of error in serial monitor
  #define LNDISP M5.Lcd
#endif

#if defined(STICK_C_PLUS)
  #include <M5StickCPlus.h>
  #define LNSD_CLK_PIN 0
  #define LNSD_MISO_PIN 36
  #define LNSD_MOSI_PIN 26
  #define LNSD_CS_PIN 14  //can be -1, but sends a lot of messages of error in serial monitor
  #define LNDISP M5.Lcd
#endif

#if defined(STICK_C_PLUS2)
  #include <M5StickCPlus2.h>
  #include <M5Unified.h>
  #define M5_BUTTON_MENU 35
  #define LNSD_CLK_PIN 0
  #define LNSD_MISO_PIN 36
  #define LNSD_MOSI_PIN 26
  #define LNSD_CS_PIN 14  //can be -1, but sends a lot of messages of error in serial monitor
  #define LNDISP M5.Lcd
#endif

#if defined(CARDPUTER)
  #include <M5Cardputer.h>
  #define LNSD_CLK_PIN 40
  #define LNSD_MISO_PIN 39
  #define LNSD_MOSI_PIN 14
  #define LNSD_CS_PIN 12
  #define LNDISP M5Cardputer.Display
#endif

#ifndef STICK_C
//  #include <M5StackUpdater.h>      // This one must be the last to declare in all devices, except STICK_C! such a noughty boy! 
#endif

#define MAX_FILES 256
#define MAX_FOLDERS 256

SPIClass SPI2_SD;
File fileRoot;
File root;
String PreFolder = "/";
String fileList[MAX_FILES];
String folderList[MAX_FOLDERS];
size_t PartitionSize = 0x2F0000; //Starts with ~2,9Mb of size.
int fileListCount;
int folderListCount;
int startIndex;
int endIndex;
int selectIndex;
bool needRedraw=true;
int rot=3;

#if defined(STICK_C_PLUS2) || defined(CARDPUTER) 
  uint32_t MaxPartitionSize = 0x600000; // 6Mb of size.
  #define MAX_SPIFFS 0x100000
#else
  uint32_t MaxPartitionSize = 0x300000; // 3Mb of size.
  #define MAX_SPIFFS 0x50000
#endif

#if defined(STICK_C_PLUS) || defined(STICK_C)
  float accX = 0.0F;
  float accY = 0.0F;
  float accZ = 0.0F;
  int dispfileCount = 7;
#else
  int dispfileCount = 8;
#endif

#ifndef CARDPUTER
  int newrot=3;
  bool ClickPwrBtn=false;
  bool ClickSideBtn=false;
  int slope=5;
#endif

#include "update_functions.h"

void sortList(String fileList[], int fileListCount) {
  bool swapped;
  String temp;
  String name1, name2;
  do {
    swapped = false;
    for (int i = 0; i < fileListCount - 1; i++) {
      name1 = fileList[i];
      name1.toUpperCase();
      name2 = fileList[i + 1];
      name2.toUpperCase();
      if (name1.compareTo(name2) > 0) {
        temp = fileList[i];
        fileList[i] = fileList[i + 1];
        fileList[i + 1] = temp;
        swapped = true;
      }
    }
  } while (swapped);
}


void readFs(String folder) {
  for (int i = 0; i < 255; ++i) {   // Reset all vectors.
    fileList[i] = "";               // Reset all vectors.
    folderList[i] = "";             // Reset all vectors.
  }
  //Read files in folder
  fileRoot = SD.open(folder);
  fileListCount = 0;
  File entry = fileRoot.openNextFile();
  while (entry) {
    if (!entry.isDirectory()) {
      String fullFileName = entry.name();
      String fileName = fullFileName.substring(fullFileName.lastIndexOf("/") + 1);
      String ext = fileName.substring(fileName.lastIndexOf(".") + 1);
      ext.toUpperCase();
      if (ext.equals("BIN") == true) {
        fileList[fileListCount] = fileName;
        fileListCount++;
      }
    }
    entry = fileRoot.openNextFile();
  }
  fileRoot.close();
  //Read folders in folder
  root = SD.open(folder);
  folderListCount = 0;
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      String fullFolderName = file.path();
      String folderName = fullFolderName.substring(fullFolderName.lastIndexOf("/") + 1);
      folderList[folderListCount] = folderName;
      folderListCount++;
    }
    file = root.openNextFile();
  }
  root.close();

  sortList(fileList, fileListCount);
  sortList(folderList, folderListCount);

  startIndex = 0;
  endIndex = startIndex + 8;
  if (endIndex >= (fileListCount + folderListCount)) {
    endIndex = folderListCount + fileListCount - 1;
  }

  needRedraw = true;
  selectIndex = 0;
}

void setup() {
#if defined(CARDPUTER)
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  rot=1;  
#elif defined(STICK_C_PLUS2)
  M5.begin();
  auto imu_update = M5.Imu.update();
  auto data = M5.Imu.getImuData();
  slope=map(data.accel.x*100,-30,100,0,50);
  if (slope>11) {  rot=1; }
  else { rot=3; }
#else
  M5.begin();
  auto imu_update = M5.Imu.Init();
  M5.IMU.getAccelData(&accX, &accY, &accZ);
  slope=map(accX*100,-30,100,0,50);
  if (slope>11) {  rot=1; }
  else { rot=3; }
#endif
  
  // Print SplashScreen
#if defined(STICK_C_PLUS2) || defined(CARDPUTER) 
  LNDISP.setRotation(rot);
  LNDISP.drawBmp(M5Launcher, sizeof(M5Launcher));
  LNDISP.setCursor(198, 12);
  LNDISP.setTextSize(1);
  LNDISP.setTextColor(BLACK,WHITE);
  LNDISP.printf("v%s", LAUNCHER_VERSION);
  LNDISP.setRotation(rot-1);
  LNDISP.setCursor(2, 1);
  LNDISP.print("> Press Enter or  M5 <");
  LNDISP.setRotation(rot);
  LNDISP.setTextSize(2);
#elif defined(STICK_C_PLUS)
  LNDISP.setRotation(rot);
  LNDISP.drawBitmap(0, 0, LNDISP.width(), LNDISP.height(), M5Launcher);
  LNDISP.setCursor(198, 12);
  LNDISP.setTextSize(1);
  LNDISP.setTextColor(BLACK,WHITE);
  LNDISP.printf("v%s", LAUNCHER_VERSION);
  LNDISP.setRotation(rot-1);
  LNDISP.setCursor(2, 1);
  LNDISP.print("> Press Enter or  M5 <");
  LNDISP.setRotation(rot);
  LNDISP.setTextSize(2);
#else
  LNDISP.fillScreen(WHITE);
  LNDISP.setCursor(0, 0);
  LNDISP.setTextSize(2);
  LNDISP.setTextColor(BLACK);
  LNDISP.println(" -## M5Launcher ##- ");
  LNDISP.println("      Press M5      ");
  LNDISP.println(" to start Launcher  ");
  LNDISP.print  ("           v.");
  LNDISP.printf("%s", LAUNCHER_VERSION);
#endif

	
// Draw Battery measurement
int battery_percent = 0;
#ifdef STICK_C_PLUS2
	battery_percent = M5.Power.getBatteryLevel();
#elif defined(CARDPUTER)
	pinMode(10, INPUT);
	battery_percent = ((((analogRead(10)) - 1842) * 100) / 738);
#else
	float b = M5.Axp.GetVbatData() * 1.1 / 1000;
	battery_percent = ((b - 3.0) / 1.2) * 100;
#endif

int battery_percent_norm = (battery_percent * 38) / 100; // 38 pixels wide square
LNDISP.drawRect(197,0,43,11,0);
LNDISP.fillRect(197,0,43,11,0);	
LNDISP.drawRect(198,0,42,10,WHITE);
LNDISP.fillRect(200,2,38,6,WHITE);	

if(battery_percent<26) { LNDISP.fillRect(200,2,battery_percent_norm,6,RED); }
if(battery_percent>25 || battery_percent<50) { LNDISP.fillRect(200,2,battery_percent_norm,6,YELLOW); }
if(battery_percent>49) { LNDISP.fillRect(200,2,battery_percent_norm,6,CYAN); }

// End of Battery draw


	
  delay(300); // to avoid restart during power on
  
  //Define variables to identify if there is an app installed after Launcher 
  esp_app_desc_t ota_desc;
  esp_err_t err = esp_ota_get_partition_description(esp_ota_get_next_update_partition(NULL), &ota_desc);

  //Start Bootscreen timer
  while(millis()<5000) { // increased from 2500 to 5000
      M5.update();
      // IF M5 or Enter is pressed, enter the Launcher
    #if defined(STICK_C_PLUS2) || defined(STICK_C_PLUS) || defined(STICK_C)
      if (M5.BtnA.wasPressed())  // M5 button
    #else
      M5Cardputer.update();
      if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER))  // Enter
    #endif
        {
          goto Launcher;
        }

      // If Side btn or G0 or arrow Up/Down -> Goto application previusly installed
    #if defined(STICK_C_PLUS2) 
      if (M5.BtnB.wasPressed() || digitalRead(M5_BUTTON_MENU) == LOW)  // M5 button
    #elif defined(STICK_C_PLUS) || defined(STICK_C)
      if (M5.BtnB.wasPressed() || M5.Axp.GetBtnPress())  // M5 button
    #else    
      M5Cardputer.update();
      if (M5Cardputer.Keyboard.isPressed() && !(M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)))  // any key but Enter
    #endif
        {
          ESP.restart();
        } 
  }
  
  // If nothing is done, check if there are any app installed in the ota partition, if it does, restart device to start installed App.
  if(err == ESP_OK) { ESP.restart(); }

  // If M5 or Enter button is pressed, continue from here
  Launcher:

#if defined(M5_BUTTON_MENU)
  pinMode(M5_BUTTON_MENU, INPUT);
#endif

  LNDISP.setTextSize(2);
  
  // Verify if SD Hat or SD card is connected..
  int i = 0;
  SPI2_SD.begin(LNSD_CLK_PIN, LNSD_MISO_PIN, LNSD_MOSI_PIN, LNSD_CS_PIN);
  while (!SD.begin(LNSD_CS_PIN, SPI2_SD)) {
    if (i == 0) {  //print msg on screen once
      LNDISP.fillScreen(BLACK);
      LNDISP.setCursor(0, 0);
      LNDISP.setTextColor(RED,WHITE);
      LNDISP.println("\n Insert SD Card Hat \n         or         \n   Insert SD Card   \n      or not        \n formatted in FAT32 ");
      i = 1;
    }
    delay(1000);
  }

  // Create the index to draw menu list
  readFs(PreFolder);
}

void loop() {

// Detect rotation of M5StickCs  ==============================================================================================================
#if defined(STICK_C_PLUS2)
  M5.begin();
  auto imu_update = M5.Imu.update();
  auto data = M5.Imu.getImuData();
  slope=map(data.accel.x*100,-30,100,0,50);
#elif defined(STICK_C_PLUS) || defined(STICK_C)
  M5.begin();
  // Detect rotation of M5StickCs
  auto imu_update = M5.Imu.Init();
  M5.IMU.getAccelData(&accX, &accY, &accZ);
  slope=map(accX*100,-30,100,0,50);
#endif

#ifndef CARDPUTER
  if (slope>21) { newrot=1; }
  else if(slope < 1) { newrot=3; }

  if(newrot!=rot) {
    rot=newrot;
    LNDISP.setRotation(rot);
    delay(500);
    needRedraw=true;
  }
#endif
// END of Detect rotation ========================================================================================================
  
  // DRAW file list ==============================================================================================================
  if (needRedraw == true) {
    startIndex = selectIndex - 5;
    if (startIndex < 0) {
      startIndex = 0;
    }
    endIndex = startIndex + dispfileCount;
    if (endIndex >= (fileListCount + folderListCount)) {
      endIndex = fileListCount + folderListCount + 1;
      if (PreFolder != "/") { endIndex++; }
      if (selectIndex>6) {
        startIndex = selectIndex - 6;
      }
      else {
        startIndex = 0;
      }
    }
    if (fileListCount == 0 && folderListCount == 0 && PreFolder == "/") {
      LNDISP.fillScreen(BLACK);
      LNDISP.setCursor(0, 0);
      LNDISP.setTextColor(RED,WHITE);
      LNDISP.println("\nSD is empty or there\nare no .bin in root.\nExample: d:\\File.bin");
      delay(2000);
      readFs("/");
    } else {
      LNDISP.fillScreen(BLACK);
      LNDISP.setCursor(0, 0);
      for (int index = startIndex; index <= (endIndex + 1); index++) {
        LNDISP.setTextColor(WHITE, BLACK); // RESET BG COLOR TO BLACK
        if (index == selectIndex) {
          if (index == 0){
            LNDISP.setTextColor(BLACK, WHITE);
            LNDISP.print(">");
          } else if (index < folderListCount+1) {
            LNDISP.setTextColor(BLUE);  // folders selected in Blue
            LNDISP.print(">");
          } else if (index < (folderListCount + fileListCount+1)){
            LNDISP.setTextColor(GREEN);  // files selected in Green
            LNDISP.print(">");
          } else if (index == (folderListCount + fileListCount+1)){
            if (PreFolder != "/") {
              LNDISP.setTextColor(RED, WHITE);  // folders in yellow
              LNDISP.print("<");
            }
        }
        } else {
	  if (index == 0){
	    LNDISP.setTextColor(WHITE);
	    LNDISP.print(" ");
	  } else if (index < folderListCount+1) {
            LNDISP.setTextColor(YELLOW);  // folders in yellow
            LNDISP.print(" ");
          } else if (index < (folderListCount + fileListCount+1)){
            LNDISP.setTextColor(WHITE);  // files in white
            LNDISP.print(" ");
          } else if (index == (folderListCount + fileListCount+1)){
            if (PreFolder != "/") {
              LNDISP.setTextColor(RED, BLACK);  // folders in yellow
              LNDISP.print(" ");
            }
          }
        }
	if (index==0) {
	  LNDISP.println(">> Restart         ");
	} else if (index < folderListCount+1) {
          LNDISP.println(folderList[index-1].substring(0, 15) + "/");
        } else if (index < (folderListCount + fileListCount+1)) {
          LNDISP.println(fileList[index - folderListCount-1].substring(0, 16));
        } else if (PreFolder != "/") { 
          LNDISP.print("<< back            ");
          break;
        } else { break; } 
      }
    }
	// Draw Battery measurement
	int battery_percent = 0;
	#ifdef STICK_C_PLUS2
		battery_percent = M5.Power.getBatteryLevel();
	#elif defined(CARDPUTER)
		pinMode(10, INPUT);
	    	battery_percent = ((((analogRead(10)) - 1842) * 100) / 738);
	#else
		float b = M5.Axp.GetVbatData() * 1.1 / 1000;
		battery_percent = ((b - 3.0) / 1.2) * 100;
	#endif
	
	int battery_percent_norm = (battery_percent * 38) / 100; // 38 pixels wide square
	LNDISP.drawRect(197,0,43,17,0);
  	LNDISP.fillRect(197,0,43,17,0);	
	LNDISP.drawRect(198,0,42,16,WHITE);
	LNDISP.fillRect(200,2,38,12,0);	
	
	if(battery_percent<26) { LNDISP.fillRect(200,2,battery_percent_norm,12,RED); }
	if(battery_percent>25 || battery_percent<50) { LNDISP.fillRect(200,2,battery_percent_norm,12,YELLOW); }
	if(battery_percent>49) { LNDISP.fillRect(200,2,battery_percent_norm,12,GREEN); }
	
	// End of Battery draw
    needRedraw = false;
    delay(150);
  }

  // END of DRAW file list ============================================================================================================

// push Button Detection ==============================================================================================================
M5.update();
#if defined(STICK_C_PLUS2)
  if (digitalRead(M5_BUTTON_MENU) == LOW) ClickPwrBtn = true; // Power Button
#endif
#if defined(STICK_C_PLUS) || defined(STICK_C)
  if (M5.Axp.GetBtnPress()) ClickPwrBtn = true;  // Power Button
#endif
#if defined(STICK_C_PLUS) || defined(STICK_C) || defined(STICK_C_PLUS2)
  if (M5.BtnB.wasPressed())  ClickSideBtn = true; // Side Button
#endif


#if defined(CARDPUTER)
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isKeyPressed('.'))  //Arrow Up
#else
  if ((ClickPwrBtn==true && rot==1) || (ClickSideBtn==true && rot==3))
#endif
      {
        selectIndex++;
        if (selectIndex > (fileListCount + folderListCount + 1) || (selectIndex == (fileListCount + folderListCount + 1) && PreFolder == "/")) {
        selectIndex = 0;
        }
      #ifndef CARDPUTER
        ClickPwrBtn=false;
        ClickSideBtn=false;
      #endif
        needRedraw = true;
        delay(100);
      }

#if defined(CARDPUTER)
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isKeyPressed(';'))        // Arrow Down
#else
  if ((ClickPwrBtn==true && rot==3) || (ClickSideBtn==true && rot==1))
#endif
  {
      selectIndex--;
      if (selectIndex < 0) {
        selectIndex = fileListCount + folderListCount - 1 + 1;
        if(PreFolder!="/") {selectIndex++;}
      }
    #ifndef CARDPUTER
      ClickPwrBtn=false;
      ClickSideBtn=false;
    #endif
      needRedraw = true;
      delay(100);
  }  

// END of push Button Detection ===========================================================================================================
  
// File Selection =========================================================================================================================
#if defined(STICK_C_PLUS2) || defined(STICK_C_PLUS) || defined(STICK_C)
  if (M5.BtnA.wasPressed())  // M5 button
#else
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER))  // Enter
#endif
  {
    if (selectIndex==0) { //when Reboot is selected
		  ESP.restart(); 
		
    } else if (selectIndex < (folderListCount +1 )) { //When select a Folder
		  if(PreFolder=="/") { PreFolder = PreFolder + folderList[selectIndex - 1]; }
		  else { PreFolder = PreFolder + "/" + folderList[selectIndex - 1]; }
		  selectIndex=0;
		  readFs(PreFolder);

    } else if (fileList[selectIndex - folderListCount - 1] == "") { // When press Back btn
		  PreFolder = PreFolder.substring(0, PreFolder.lastIndexOf("/"));
		  if(PreFolder == ""){ PreFolder = "/"; }
		  selectIndex=0;
		  readFs(PreFolder);
	
    } else { // when select a file

      // ---------------------------------------------####### HERE IS WHERE ALL THE MAGIC HAPPENS!! #######-----------------------------------------------------

      // Starting preparations and process to copy the binary into other partition.
      if(PreFolder=="/") { PreFolder = ""; }
      File file = SD.open(PreFolder + "/" + fileList[selectIndex - folderListCount - 1], FILE_READ);
      if (!file) {
        Serial.println("Error: Could not open " + fileList[selectIndex - 1]);
        return;
      }

      // Read 3 bytes of the file at position 0x8000 (in order to see if it is partitions information)
      uint8_t firstThreeBytes[16];
      if (!file.seek(0x8000)) {
        Serial.println("Error: Could not move cursor into 0x8000, to read partition size.");
        return;
      }
      
      file.read(firstThreeBytes, 16);

      // Verify if the 3 bytes at 0x8000 are different to 0xAA 0x50 0x0a (meaning that the file have Bootloader, partitions and application)
      //

      // ==========================================================================================================File is a bin app, without bootloader and partitions
      if (firstThreeBytes[0] != 0xAA || firstThreeBytes[1] != 0x50 || firstThreeBytes[2] != 0x01) {
        if (!file.seek(0x0)) {
          Serial.println("Error: Could not move cursor into 0x0,");
          return;
        }
        file.read(firstThreeBytes, 16);

        if (firstThreeBytes[0] == 0xE9 && (firstThreeBytes[1] == 0x05 || firstThreeBytes[1] == 0x06 || firstThreeBytes[1] == 0x07) && firstThreeBytes[2] == 0x02) {
          // firstThreeBytes[1] == 0x05 -> C++ Apps
          // firstThreeBytes[1] == 0x06 -> MicroPython (work in progress, might need to change bootloader in the future
          // firstThreeBytes[1] == 0x07 -> Come apps are comming with this MagicNumber (Volos Watch)
          
          if (firstThreeBytes[1] == 0x06) {
            //Checks if there are Micropython signature
            Serial.println("Micropython file not supported..");
          } 

          //Perform Installation.
          Serial.println("Status: Binary is an apllication");
          if (!file.seek(0x0)) {
            Serial.println("Error: Could not move cursor into 0x0,");
            return;
          }
          LNDISP.fillScreen(BLACK);
          LNDISP.setCursor(0, 0);
          LNDISP.setTextColor(WHITE);
          LNDISP.println("\n -## M5Launcher ##- \n     Installing    ");
          LNDISP.setTextSize(1);
          LNDISP.println("\nApp   : " + fileList[selectIndex - folderListCount - 1]);

          LNDISP.drawRect(18,78,204,19,WHITE);
          LNDISP.fillRect(20,80,200,15,0);

          performUpdate(file, file.size(), U_FLASH); // ----------------------------------------------------------------- Install Flash app

	  // Restart ESP after updates
	  file.close();
          ESP.restart();        
        } else {
	  LNDISP.fillScreen(BLACK);
          LNDISP.setCursor(0, 0);
          LNDISP.setTextColor(WHITE);
          LNDISP.println("\n -## M5Launcher ##- \n\n File not Supported ");
	  needRedraw=true;
	  delay(5000);
	  return;
	}
      } 
      // ========================================================================================================== File is made to flash at 0x0
      else {
        Serial.println("Status: Binary file was made to flash at 0x0. Starting process.");

        //read 0x10000 0x10001 and 0x10002 to determine if the binary is valid        
        if (!file.seek(0x10000)) {
          Serial.println("Error: Could not move cursor into 0x10000,");
          return;
        }
        file.read(firstThreeBytes, 16);
        // E9 && (05 || 07) && 02 == valid file
        if (firstThreeBytes[0] == 0xE9 && (firstThreeBytes[1] == 0x05 || firstThreeBytes[1] == 0x06 || firstThreeBytes[1] == 0x07) && firstThreeBytes[2] == 0x02) {
          // firstThreeBytes[1] == 0x05 -> C++ Apps from Arduino IDE
	  // firstThreeBytes[1] == 0x06 -> MicroPython or ESP-IDF, idk
          // firstThreeBytes[1] == 0x07 -> Come apps are comming with this MagicNumber (Volos Watch), depend on the version of compiler
          if(firstThreeBytes[1] == 0x06) {
            //Checks if there are Micropython signature
            Serial.println("Micropython file not supported..");
          } 
        } else {
	  LNDISP.fillScreen(BLACK);
          LNDISP.setCursor(0, 0);
          LNDISP.setTextColor(WHITE);
          LNDISP.println("\n -## M5Launcher ##- \n\n File not Supported ");
	  needRedraw=true;
	  delay(5000);
	  return;
	}
        
        // detect the size of the first app partition, after 0x10000
        // Move cursor of the file into byte 0x804A                                             TODO: Check if this is really the position of the first partitions, may have problem if use Factory partition only
        if (!file.seek(0x8040)) {
          Serial.println("Error: Could not move cursor into 0x804A, to read partition size.");
          return;
        }
        file.read(firstThreeBytes, 16);
        firstThreeBytes[0x0C] = 0x00; // Sets to zero to complete the partition size
        size_t temp_size=0;
        PartitionSize = (firstThreeBytes[0x0A] << 16) | (firstThreeBytes[0x0B] << 8) | firstThreeBytes[0x0C]; // Sets max size of partition.

        //Checks if file fits in the launcher partitions
        if(file.size()<MaxPartitionSize || PartitionSize<=MaxPartitionSize) { 
		temp_size=PartitionSize;
		temp_size+=0x10000;
		if(file.size()<= temp_size) { // Check if the file is smaller than the app0 partition
        		PartitionSize = file.size();	//gets file size
			PartitionSize -= 0x10000;	//subtracts bootloader, partitions and other junks
        	} else { 
			PartitionSize = PartitionSize; 	//if file is greater then app0 partition+junk, it will limit to app0 partition size
		}
        } 
        else {

            Serial.println("Partition might be too big, waiting confirmation:");
            LNDISP.fillScreen(BLACK);
            LNDISP.setCursor(0, 0);
            LNDISP.setTextColor(WHITE);
            LNDISP.println(" -## M5Launcher ##- \n\n Binary is too big \n should I continue? \n\nEnter/M5 to continue\n\ any key to cancel ");
            delay(500); // to avoid jumping verifications

          while(1) {
          #if defined(STICK_C_PLUS2) || defined(STICK_C_PLUS) || defined(STICK_C)
            M5.update();
            if (M5.BtnA.wasPressed())  // M5 button
          #else
            M5Cardputer.update();
            if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER))  // Enter
          #endif
            {
              //Set partitionsize to MaxPartitionSize of the Launcher, and will crop the file at this point
              //This way, if the application doen´t use all the partition size, there are high chances that it will work.
              Serial.println("Challange accepted! Cropping the file and trying to launch!");
              PartitionSize = MaxPartitionSize;
              goto nextstep; 
            }
          #if defined(STICK_C_PLUS2)
            if (digitalRead(M5_BUTTON_MENU) == LOW || M5.BtnB.wasPressed())  // power button
          #elif defined(STICK_C_PLUS) || defined(STICK_C)
            if (M5.Axp.GetBtnPress() || M5.BtnB.wasPressed())  // Power Button
          #elif defined(CARDPUTER)
            M5Cardputer.update();
            if (M5Cardputer.Keyboard.isPressed() && !((M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER))))  //Any key except Ok
          #endif
            {
              Serial.println("All right.. not all battles are there to be fought!");
              needRedraw=true;
              break;
            }
          }
          if(needRedraw) return;
        }

        nextstep:

// ==========================================================================================================Detecting Spiffs partition
        // detect offset and size of the first Spiffs partition
        // Move cursor of the file into byte 0x804A
        uint16_t i=0;
        uint32_t spiffs_offset = 0;
	uint32_t spiffs_size = 0;
	Serial.println("-------- Partition Table --------- |");
        for (i=0x0; i<=0x1A0; i+=0x20) {	//Partition
          Serial.print((0x8000+i),HEX);
          if (!file.seek(0x8000+i)) {
            Serial.println("Error: Could not move cursor into 0x804A, to read partition size.");
            return;
          }
          file.read(firstThreeBytes, 16);
	  // https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/partition-tables.html -> spiffs (0x82) is for SPIFFS Filesystem.
    	  if (firstThreeBytes[3]==0xFF) Serial.println(": ------- END of Table ------- |"); 
	  if (firstThreeBytes[3]==0x00) Serial.println(": Otadata or Factory partition |");
	  if (firstThreeBytes[3]==0x01) Serial.println(": PHY inicialization partition |");
	  if (firstThreeBytes[3]==0x02) Serial.println(": NVS partition                |");
	  if (firstThreeBytes[3]==0x03) Serial.println(": Coredump partition           |");
	  if (firstThreeBytes[3]==0x04) Serial.println(": NVSkeys partition            |");
	  if (firstThreeBytes[3]==0x05) Serial.println(": Efuse partition              |");
	  if (firstThreeBytes[3]==0x06) Serial.println(": Undefined partition          |");
	  if (firstThreeBytes[3]>=0x10 && firstThreeBytes[3]<=0x1F) Serial.println(": OTA partition                |");
	  if (firstThreeBytes[3]==0x20) Serial.println(": TEST partition               |");
	  if (firstThreeBytes[3]==0x81) Serial.println(": FAT partition                |");
	  if (firstThreeBytes[3]==0x83) Serial.println(": LittleFs partition           |");
          if (firstThreeBytes[3]==0x82) { 
		  Serial.println(": Spiffs partition             |"); 
	          spiffs_offset = (firstThreeBytes[0x06] << 16) | (firstThreeBytes[0x07] << 8) | firstThreeBytes[0x08];	// Write the offset of spiffs partition
		  firstThreeBytes[0x0C]=0;
	          spiffs_size = (firstThreeBytes[0x0A] << 16) | (firstThreeBytes[0x0B] << 8) | firstThreeBytes[0x0C];	// Write the size of spiffs partition
          }
        }
	Serial.print("\nSpiffs partition on: ");
        Serial.println((0x8000+i),HEX);
        Serial.print("Spiffs Offset: ");
        Serial.println(spiffs_offset,HEX);
        Serial.print("Spiffs Size: ");
        Serial.println(spiffs_size,HEX);
	      
        // Check if there is room for spiffs in the file
        if (file.size()<spiffs_offset) {
          Serial.printf("\nError: file dont reach spiffs offset %d, to read spiffs.", spiffs_offset,HEX);
          goto finish_files;
        }
        Serial.println("Preparing launcher_temp.bin.spiffs");
        // check size of the Spiffs Partition, if it fits in the launcher
        // If it is larger the the Launcher Spiffs Partition, cut it to the limit
        if (spiffs_size>MAX_SPIFFS) { 
          spiffs_size = MAX_SPIFFS;
	  temp_size=spiffs_offset + spiffs_size;
	  if (file.size()<=temp_size) {
	 	 spiffs_size=file.size() - spiffs_offset;
	  }
          Serial.print("\nTotal spiffs size after crop: ");
          Serial.println(spiffs_size,HEX);
        }
        // =========================================================================================================== INSTALLING APP AND UPDATING PARTITIONS
        finish_files: 
        // Move cursor of the file into byte 0x10000
        if (!file.seek(0x10000)) {
          Serial.println("Error: Could not move cursor into 0x10000");
          return;
        }
        LNDISP.fillScreen(BLACK);
        LNDISP.setCursor(0, 0);
        LNDISP.setTextColor(WHITE);
        LNDISP.println("\n -## M5Launcher ##- \n     Installing    ");
        LNDISP.setTextSize(1);
        LNDISP.println("\nApp   : " + fileList[selectIndex - folderListCount - 1]);

        LNDISP.drawRect(18,78,204,19,WHITE);
        LNDISP.fillRect(20,80,200,15,0);

        performUpdate(file, PartitionSize, U_FLASH); // ----------------------------------------------------------------- Install Flash app

	// Check if there is room for spiffs partition
        if (file.size()>spiffs_offset) {
	        // Move cursor of the file into spiffs offset
	        if (!file.seek(spiffs_offset)) {
	          Serial.println("Error: Could not move cursor into 0x10000");
	          return;
	        }
		
		// Sets the size of Spiffs partition
		PartitionSize = spiffs_size;
	        LNDISP.println("Spiffs: " + fileList[selectIndex - folderListCount - 1] + ".spiffs");
	
	      	LNDISP.drawRect(18,78,204,19,WHITE);
	        LNDISP.fillRect(20,80,200,15,0);
	      	
	        performUpdate(file, PartitionSize, U_SPIFFS); // ----------------------------------------------------------------- Install Spiffs
	}

        // Restart ESP after updates
	file.close();
        ESP.restart();

      } 
    }
  }
}
