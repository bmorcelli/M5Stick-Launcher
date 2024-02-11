/*            M5StickCPlus2 Simple Launcher -> Discord: @bmorcelli - Pirata#5263              */


// ============= Choose your Destiny ==============
// #define STICK_C         // 4Mb of Flash Memory -> Need custom partitioning (mandatory)
// #define STICK_C_PLUS    // 4Mb of Flash Memory -> Need custom partitioning (mandatory)
// #define STICK_C_PLUS2   // 8Mb of Flash Memory -> Need custom partitioning (mandatory)
// #define CARDPUTER       // 8Mb of Flash Memory -> Need custom partitioning (mandatory)
// ============== Flawless Victory ================

#define LAUNCHER_VERSION "dev"

#if !defined(CARDPUTER) && !defined(STICK_C_PLUS2) && !defined(STICK_C_PLUS) && !defined(STICK_C) && !defined(STICK_C_PLUSv1)
#define STICK_C_PLUS2
#endif

/* -=-=-=-=-=-=-=-=-= Including Libraries =-=-=-=-=-=-=-=-=- */
#include "M5GFX.h"
#include <SPI.h>
#include <FS.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <SdFat.h>
#include <stdint.h>

#if defined(STICK_C)
  #include <M5StackUpdater.h>              // This one must be the last to declare in all devices, except STICK_C! such a noughty boy! 
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
  #include <M5StackUpdater.h>      // This one must be the last to declare in all devices, except STICK_C! such a noughty boy! 
#endif

SPIClass SPI2_SD;
#define MAX_FILES 256
#define MAX_FOLDERS 256

File fileRoot;
File root;
String PreFolder = "/";
String fileList[MAX_FILES];
String folderList[MAX_FOLDERS];
uint32_t PartitionSize = 0x2F0000; //Starts with 3Mb of size.
int fileListCount;
int folderListCount;
int startIndex;
int endIndex;
#if defined(STICK_C_PLUS2)
  int dispfileCount = 8;
#else
  int dispfileCount = 7;
#endif
int selectIndex;
boolean needRedraw;


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

  root = SD.open(folder);
  folderListCount = 0;
  File file = root.openNextFile();
  if (folder == "/") { file = root.openNextFile(); }  //Jump "System Volume Information"
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
#else
  M5.begin();
#endif

  // Print SplashScreen
  LNDISP.setRotation(1);
  LNDISP.setTextSize(2);
  LNDISP.fillScreen(WHITE);
  LNDISP.setCursor(0, 0);
  LNDISP.setTextColor(BLACK);
  LNDISP.println("                    ");
  LNDISP.println("                    ");
  LNDISP.println(" -## M5Launcher ##- ");
  LNDISP.println("Choose your Destiny!");
  LNDISP.println("                    ");
  LNDISP.println("  Press M5 (Enter)  ");
  LNDISP.println(" to start Launcher  ");
  LNDISP.print  ("           v.");
  LNDISP.printf("%s", LAUNCHER_VERSION);


  while(millis()<2500) {
    M5.update();
#if defined(STICK_C_PLUS2) || defined(STICK_C_PLUS) || defined(STICK_C)
  if (M5.BtnA.wasPressed())  // M5 button
#else
  if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER))  // Enter
#endif
    {
      goto Launcher;
    }

  }
  // If nothing is done, restart device to start installed App.
  ESP.restart();


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
      LNDISP.fillScreen(WHITE);
      LNDISP.setCursor(0, 0);
      LNDISP.setTextColor(RED);
      LNDISP.println("                    ");
      LNDISP.println(" Insert SD Card Hat ");
      LNDISP.println("         or         ");
      LNDISP.println("   Insert SD Card   ");
      i = 1;
    }
    delay(1000);
  }

  // Create the index to draw menu list
  readFs(PreFolder);
}

void loop() {

  if (needRedraw == true) {
    LNDISP.setCursor(0, 0);
    startIndex = selectIndex - 5;
    if (startIndex < 0) {
      startIndex = 0;
    }
    endIndex = startIndex + dispfileCount;
    if (endIndex >= (fileListCount + folderListCount)) {
      endIndex = fileListCount + folderListCount + 1;
      //endIndex = fileListCount - 1;
      if (PreFolder != "/") { endIndex++; }
      // startIndex = endIndex - 12;
      startIndex = endIndex - dispfileCount;
      if (startIndex < 0) {
        startIndex = 0;
      }
    }

    
    if (fileListCount == 0 && folderListCount == 0 && PreFolder == "/") {
      LNDISP.fillScreen(WHITE);
      LNDISP.setTextColor(RED);
      LNDISP.println();
      LNDISP.println("SD is empty or there");
      LNDISP.println("are no .bin in root.");
      LNDISP.println("Example: d:\\menu.bin");
      delay(2000);
      readFs("/");  //  Todo: Make it comes back one folder
    } else {
      LNDISP.fillScreen(BLACK);
      for (int index = startIndex; index <= endIndex; index++) {
        LNDISP.setTextColor(WHITE, BLACK); // RESET BG COLOR TO BLACK
        if (index == selectIndex) {
          if (index < folderListCount) {
            LNDISP.setTextColor(BLUE);  // folders selected in Blue
            LNDISP.print(">");
          } else if (index < (folderListCount + fileListCount)){
            LNDISP.setTextColor(GREEN);  // files selected in Green
            LNDISP.print(">");
          } else if (index == (folderListCount + fileListCount)){
            if (PreFolder != "/") {
              LNDISP.setTextColor(RED, WHITE);  // folders in yellow
              LNDISP.print("<");
            }
        }
        } else {
          if (index < folderListCount) {
            LNDISP.setTextColor(YELLOW);  // folders in yellow
            LNDISP.print(" ");
          } else if (index < (folderListCount + fileListCount)){
            LNDISP.setTextColor(WHITE);  // files in white
            LNDISP.print(" ");
          } else if (index == (folderListCount + fileListCount)){
            if (PreFolder != "/") {
              LNDISP.setTextColor(RED, BLACK);  // folders in yellow
              LNDISP.print(" ");
            }
          }
        }
        if (index < folderListCount) {
          LNDISP.println(folderList[index].substring(0, 15) + "/");
        } else if (index < (folderListCount + fileListCount)) {
          LNDISP.println(fileList[index - folderListCount].substring(0, 16));
        } else if (PreFolder != "/") { 
          LNDISP.print("<< back            ");
          break;
        } else { break; } 
      }
    }
    needRedraw = false;
    delay(250);
  }

  M5.update();

#if defined(STICK_C_PLUS2)
  if (digitalRead(M5_BUTTON_MENU) == LOW)  // power button
#endif
#if defined(STICK_C_PLUS) || defined(STICK_C)
    if (M5.Axp.GetBtnPress())  // Power Button
#endif
#if defined(CARDPUTER)
      if (M5Cardputer.Keyboard.isKeyPressed(';'))  //Arrow Up
#endif
      {
        selectIndex--;
        if (selectIndex < 0) {
          selectIndex = fileListCount + folderListCount - 1;
        }
        needRedraw = true;
      }

#if defined(STICK_C_PLUS2) || defined(STICK_C_PLUS) || defined(STICK_C)
  if (M5.BtnB.wasPressed())  // Side Button
#else
  if (M5Cardputer.Keyboard.isKeyPressed('.'))        // Arrow Down
#endif
  {
    selectIndex++;
    if (selectIndex > (fileListCount + folderListCount) || (selectIndex == (fileListCount + folderListCount) && PreFolder == "/")) {
      selectIndex = 0;
    }

    needRedraw = true;
    delay(150);
  }  

#if defined(STICK_C_PLUS2) || defined(STICK_C_PLUS) || defined(STICK_C)
  if (M5.BtnA.wasPressed())  // M5 button
#else
  if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER))  // Enter
#endif
  {
    if (selectIndex < folderListCount) {
      if(PreFolder=="/") { PreFolder = PreFolder + folderList[selectIndex]; }
      else { PreFolder = PreFolder + "/" + folderList[selectIndex]; }
      readFs(PreFolder);

    } else if (fileList[selectIndex - folderListCount] == "") {

      PreFolder = PreFolder.substring(0, PreFolder.lastIndexOf("/"));
      if(PreFolder == ""){ PreFolder = "/"; }
      readFs(PreFolder);

    } else {


      // -------####### HERE IS WHERE ALL THE MAGIC HAPPENS!! #######----------

      // Starting preparations and process to copy the binary into other partition.
      if(PreFolder=="/") { PreFolder = ""; }
      File file = SD.open(PreFolder + "/" + fileList[selectIndex - folderListCount], FILE_READ);
      if (!file) {
        Serial.println("Error: Could not open " + fileList[selectIndex]);
        return;
      }

      // Read first 3 bytes of the file
      uint8_t firstThreeBytes[3];
      file.read(firstThreeBytes, 3);

      // Verify if the first 3 bytes are different from 0xE9 0x05 0x02 (meaning that the file have Bootloader, partitions and application)
      //
      if (firstThreeBytes[0] != 0xE9 && firstThreeBytes[1] != 0x05 && firstThreeBytes[2] != 0x02) {
        Serial.println("Status: Binary file is made to flash at 0x0. Starting splitting.");
        File outputFile = SD.open(PreFolder + "/Z_file.bin", FILE_WRITE);

        // Reuse firstThreeBytes to detect the size of the first app partition, after 0x10000
            // Move cursor of the file into byte 0x804A
        if (!file.seek(0x804A)) {
          Serial.println("Error: Could not move cursor into 0x084A, to read partition size.");
          return;
        }

        file.read(firstThreeBytes, 3);
  
        if (firstThreeBytes[2]==0x61) { Serial.println("Partition type: application "); }
        if (firstThreeBytes[2]==0x66) { Serial.println("Partition type: factory "); }
        firstThreeBytes[2] = 0x00; // 
        
        PartitionSize = (firstThreeBytes[0] << 16) | (firstThreeBytes[1] << 8) | firstThreeBytes[2]; // Sets max size of partition.
        PartitionSize = PartitionSize + 0x10000; // adds the apps partition offset... turns into the offset of the next partition
        
        // Try to create Z_file.bin (temp file)
        if (!outputFile) {
          Serial.println("Error: Could not create Z_file.bin. (Temp file)");
          return;
        }
        // Move cursor of the file into byte 0x10000
        if (!file.seek(0x10000)) {
          Serial.println("Error: Could not move cursor into 0x10000");
          return;
        }

        // read file.bin after byte 0x10000 and write in Z_file.bin
        const size_t bufferSize = 512;
        uint8_t buffer[bufferSize];
        size_t bytesRead;
        LNDISP.fillScreen(WHITE);
        LNDISP.setCursor(0, 0);
        LNDISP.setTextColor(BLACK);
        LNDISP.println("                    ");
        LNDISP.println("                    ");
        LNDISP.println("                    ");
        LNDISP.println(" -## M5Launcher ##- ");
        LNDISP.println("    Processing...   ");
        LNDISP.println("                    ");
        LNDISP.println("                    ");
        LNDISP.println("                    ");
        long i = 0;
        Serial.println(PartitionSize,HEX);
        delay(2000);
        Serial.println("Loading .");
        while ((bytesRead = file.read(buffer, bufferSize)) > 0) {
          if (outputFile.write(buffer, bytesRead) != bytesRead) {
            Serial.println("Error: Could not write into Z_file.bin. (temp file)");
            return;
          }
          if (file.position() >= PartitionSize) {
            Serial.println("Loading Status: Reached end of app partition.");
            break;
          }
          Serial.print(".");
          LNDISP.setCursor(210, 105);
          if (i == 240) { i = 0; }
          if (i == 210) {
            LNDISP.setTextColor(WHITE);
            LNDISP.print("\\");
          }
          if (i == 180) {
            LNDISP.setTextColor(BLACK);
            LNDISP.print("\\");
          }
          if (i == 150) {
            LNDISP.setTextColor(WHITE);
            LNDISP.print("-");
          }
          if (i == 120) {
            LNDISP.setTextColor(BLACK);
            LNDISP.print("-");
          }
          if (i == 90) {
            LNDISP.setTextColor(WHITE);
            LNDISP.print("/");
          }
          if (i == 60) {
            LNDISP.setTextColor(BLACK);
            LNDISP.print("/");
          }
          if (i == 30) {
            LNDISP.setTextColor(WHITE);
            LNDISP.print("|");
          }
          if (i == 0) {
            LNDISP.setTextColor(BLACK);
            LNDISP.print("|");
          }
          i++;
        }
        Serial.print(" Complete!");
        
        // Close Files
        file.close();
        outputFile.close();

        // Rename original name into .bak
        if (!SD.rename(PreFolder + "/" + fileList[selectIndex - folderListCount],PreFolder + "/" + fileList[selectIndex - folderListCount] + ".bak")) {
          Serial.println("Error: Could not renome " + fileList[selectIndex - folderListCount] + " to " + fileList[selectIndex - folderListCount] + ".bak");
          return;
        }

        Serial.println(fileList[selectIndex - folderListCount] + "renomed to " + fileList[selectIndex - folderListCount] + ".bak");

        // Rename temp file to original name
        if (!SD.rename(PreFolder + "/Z_file.bin",PreFolder + "/" + fileList[selectIndex - folderListCount])) {
          Serial.println("Error: could not rename Z_file.bin (tempfile)");
          return;
        }

        Serial.println("Z_file.bin renamed to " + fileList[selectIndex - folderListCount]);

        updateFromFS(SD,PreFolder +  "/" + fileList[selectIndex - folderListCount], LNSD_CS_PIN);
        ESP.restart();

      } else {
        Serial.println("First 3 bytes are 0xE9 0x05 0x02");
        updateFromFS(SD,PreFolder + "/" + fileList[selectIndex - folderListCount], LNSD_CS_PIN);
        ESP.restart();
      }
    }
  }
}
