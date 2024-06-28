#include "globals.h"    
#include "sd_functions.h"
#include "display.h"
#include "mykeyboard.h"
#include "esp_log.h"


SPIClass sdcardSPI;
String fileToCopy;
// Protected global variables
String fileList[MAXFILES][3];

#ifndef STICK_C_PLUS
/***************************************************************************************
** Function name: eraseFAT
** Description:   erase FAT partition to micropython compatibilities
***************************************************************************************/
bool eraseFAT() {
  esp_err_t err;

  // Find FAT partition with its name
  const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "vfs");
  if (!partition) {
      //log_e("Failed to find partition");
      return false;
  }

  // erase all FAT partition
  err = esp_partition_erase_range(partition, 0, partition->size);
  if (err != ESP_OK) {
      //log_e("Failed to erase partition: %s", esp_err_to_name(err));
      return false;
  } 

  // Find FAT partition with its name
  partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "sys");
  if (!partition) {
      //log_e("Failed to find partition");
      goto Exit;
  }

  // erase all FAT partition
  err = esp_partition_erase_range(partition, 0, partition->size);
  if (err != ESP_OK) {
      return false;
  }
  Exit:
  return true;

}
#endif
/***************************************************************************************
** Function name: setupSdCard
** Description:   Start SD Card
***************************************************************************************/
bool setupSdCard() {
  #if !defined(M5STACK) // Core2 uses the same SPI bus as TFT
  sdcardSPI.begin(SDCARD_SCK, SDCARD_MISO, SDCARD_MOSI, SDCARD_CS); // start SPI communications
  delay(10);
  if (!SD.begin(SDCARD_CS, sdcardSPI))  
  #else
  if (!SD.begin(SDCARD_CS)) 
  #endif
  {
   // sdcardSPI.end(); // Closes SPI connections and release pin header.
    //Serial.println("Failed to mount SDCARD");
    sdcardMounted = false;
    return false;
  }
  else {
    //Serial.println("SDCARD mounted successfully");
    sdcardMounted = true;
    return true;
  }
}

/***************************************************************************************
** Function name: closeSdCard
** Description:   Turn Off SDCard, set sdcardMounted state to false
***************************************************************************************/
void closeSdCard() {
  SD.end();
  #if !defined(M5STACK)
  //sdcardSPI.end(); // Closes SPI connections and release pins.
  #endif
  //Serial.println("SD Card Unmounted...");
  sdcardMounted = false;
}

/***************************************************************************************
** Function name: ToggleSDCard
** Description:   Turn Off or On the SDCard, return sdcardMounted state
***************************************************************************************/
bool ToggleSDCard() {
  if (sdcardMounted == true) {
    closeSdCard();
    sdcardMounted = false;
    return false;
  } else {
      sdcardMounted = setupSdCard();
      return sdcardMounted;
  }
}
/***************************************************************************************
** Function name: deleteFromSd
** Description:   delete file or folder
***************************************************************************************/
  bool deleteFromSd(String path) {
  File dir = SD.open(path);
  if (!dir.isDirectory()) {
    return SD.remove(path.c_str());
  }

  dir.rewindDirectory();
  bool success = true;

  File file = dir.openNextFile();
  while(file) {
    if (file.isDirectory()) {
      success &= deleteFromSd(file.path());
    } else {
      success &= SD.remove(file.path());
    }
    file = dir.openNextFile();
  }

  dir.close();
  // Apaga a própria pasta depois de apagar seu conteúdo
  success &= SD.rmdir(path.c_str());
  return success;
}

/***************************************************************************************
** Function name: renameFile
** Description:   rename file or folder
***************************************************************************************/
bool renameFile(String path, String filename) {
  String newName = keyboard(filename,76,"Type the new Name:");
    if(!SD.begin()) {
        //Serial.println("Falha ao inicializar o cartão SD");
        return false;
    }

    // Rename the file of folder
    if (SD.rename(path, path.substring(0,path.lastIndexOf('/')) + "/" + newName)) {
        //Serial.println("Renamed from " + filename + " to " + newName);
        return true;
    } else {
        //Serial.println("Fail on rename.");
        return false;
    }
}

/***************************************************************************************
** Function name: copyFile
** Description:   copy file address to memory
***************************************************************************************/
bool copyFile(String path) {
  if(!SD.begin()) {
    //Serial.println("Fail to start SDCard");
    return false;
  }
  File file = SD.open(path, FILE_READ);
  if(!file.isDirectory()) {
    fileToCopy = path;
    file.close();
    return true;
  }
  else {
    displayRedStripe("Cannot copy Folder");
    file.close();
    return false;
  }
  
}

/***************************************************************************************
** Function name: pasteFile
** Description:   paste file to new folder
***************************************************************************************/
bool pasteFile(String path) {
  // Tamanho do buffer para leitura/escrita
  const size_t bufferSize = 2048*2; // Ajuste conforme necessário para otimizar a performance
  uint8_t buffer[bufferSize];

  // Abrir o arquivo original
  File sourceFile = SD.open(fileToCopy, FILE_READ);
  if (!sourceFile) {
    //Serial.println("Falha ao abrir o arquivo original para leitura");
    return false;
  }

  // Criar o arquivo de destino
  File destFile = SD.open(path + "/" + fileToCopy.substring(fileToCopy.lastIndexOf('/') + 1), FILE_WRITE);
  if (!destFile) {
    //Serial.println("Falha ao criar o arquivo de destino");
    sourceFile.close();
    return false;
  }

  // Ler dados do arquivo original e escrever no arquivo de destino
  size_t bytesRead;
  int tot=sourceFile.size();
  int prog=0;
  //tft.drawRect(5,HEIGHT-12, (WIDTH-10), 9, FGCOLOR);
  while ((bytesRead = sourceFile.read(buffer, bufferSize)) > 0) {
    if (destFile.write(buffer, bytesRead) != bytesRead) {
      //Serial.println("Falha ao escrever no arquivo de destino");
      sourceFile.close();
      destFile.close();
      return false;
    } else {
      prog+=bytesRead;
      float rad = 360*prog/tot;
      tft.drawArc(WIDTH/2,HEIGHT/2,HEIGHT/4,HEIGHT/5,0,int(rad),ALCOLOR,BGCOLOR,true);
      //tft.fillRect(7,HEIGHT-10, (WIDTH-14)*prog/tot, 5, FGCOLOR);
    }
  }

  // Fechar ambos os arquivos
  sourceFile.close();
  destFile.close();
  return true;
}


/***************************************************************************************
** Function name: createFolder
** Description:   create new folder
***************************************************************************************/
bool createFolder(String path) {
  String foldername=keyboard("",76,"Folder Name: ");
  if(!SD.begin()) {
    //Serial.println("Fail to start SDCard");
    return false;
  }
  if(!SD.mkdir(path + foldername)) {
    displayRedStripe("Couldn't create folder");
    return false;
  }
  return true;
}

/***************************************************************************************
** Function name: sortList
** Description:   sort files for name
***************************************************************************************/
void sortList(String fileList[][3], int fileListCount) {
    bool swapped;
    String temp[3];
    String name1, name2;

    do {
        swapped = false;
        for (int i = 0; i < fileListCount - 1; i++) {
            name1 = fileList[i][0];
            name1.toUpperCase();
            name2 = fileList[i + 1][0];
            name2.toUpperCase();

            // Verificar se ambos são pastas ou arquivos
            bool isFolder1 = fileList[i][2] == "folder";
            bool isFolder2 = fileList[i + 1][2] == "folder";

            // Primeiro, ordenar pastas
            if (isFolder1 && !isFolder2) {
                continue; // Se o primeiro for uma pasta e o segundo não, não troque
            } else if (!isFolder1 && isFolder2) {
                // Se o primeiro for um arquivo e o segundo uma pasta, troque
                for (int j = 0; j < 3; j++) {
                    temp[j] = fileList[i][j];
                    fileList[i][j] = fileList[i + 1][j];
                    fileList[i + 1][j] = temp[j];
                }
                swapped = true;
            } else {
                // Ambos são pastas ou arquivos, então ordenar alfabeticamente
                if (name1.compareTo(name2) > 0) {
                    for (int j = 0; j < 2; j++) {
                        temp[j] = fileList[i][j];
                        fileList[i][j] = fileList[i + 1][j];
                        fileList[i + 1][j] = temp[j];
                    }
                    swapped = true;
                }
            }
        }
    } while (swapped);
}

/***************************************************************************************
** Function name: sortList
** Description:   sort files for name
***************************************************************************************/
void readFs(String folder, String result[][3]) {

    int allFilesCount = 0;
    while(allFilesCount<MAXFILES) {
      result[allFilesCount][0]="";
      result[allFilesCount][1]="";
      result[allFilesCount][2]="";
      allFilesCount++;
    }
    allFilesCount=0;

    if (!SD.begin()) {
        //Serial.println("Falha ao iniciar o cartão SD");
        displayRedStripe("SD not found or not formatted in FAT32");
        delay(2000);
        return; // Retornar imediatamente em caso de falha
    }

    File root = SD.open(folder);
    if (!root || !root.isDirectory()) {
        //Serial.println("Não foi possível abrir o diretório");
        return; // Retornar imediatamente se não for possível abrir o diretório
    }

    File file2 = root.openNextFile();
    while (file2 && allFilesCount < (MAXFILES-1)) {
        String fileName = file2.name();
        if (!file2.isDirectory()) {
            String ext = fileName.substring(fileName.lastIndexOf(".") + 1);
            ext.toUpperCase();
            if(onlyBins) {
              if (ext.equals("BIN")) {
                result[allFilesCount][0] = fileName.substring(fileName.lastIndexOf("/") + 1);
                result[allFilesCount][1] = file2.path();
                result[allFilesCount][2] = "file";
                allFilesCount++;
              }
            } 
            else {
              result[allFilesCount][0] = fileName.substring(fileName.lastIndexOf("/") + 1);
              result[allFilesCount][1] = file2.path();
              result[allFilesCount][2] = "file";
              allFilesCount++;
            }
        }
        
        file2 = root.openNextFile();
    }
    file2.close();
    root.close();

    root = SD.open(folder);
    File file = root.openNextFile();
    while (file && allFilesCount < (MAXFILES-1)) {
        String fileName = file.name();
        if (file.isDirectory()) {
            result[allFilesCount][0] = fileName.substring(fileName.lastIndexOf("/") + 1);
            result[allFilesCount][1] = file.path();
            result[allFilesCount][2] = "folder";
            allFilesCount++;
        } 
        file = root.openNextFile();
    }
    file.close();
    root.close();

    // Ordenar os arquivos e pastas
    sortList(result, allFilesCount);
    //allFilesCount++;
    result[allFilesCount][0] = "> Back";
    folder = folder.substring(0,folder.lastIndexOf('/'));
    if(folder=="") folder = "/";
    result[allFilesCount][1] = folder;
    result[allFilesCount][2] = "operator";    
}
/*********************************************************************
**  Function: loopSD                          
**  Where you choose what to do wuth your SD Files   
**********************************************************************/
void loopSD(){
  bool reload=false;
  bool redraw = true;
  int index = 0;
  int maxFiles = 0;
  String Folder = "/";
  String PreFolder = "/";
  tft.fillScreen(BGCOLOR);
  tft.drawRoundRect(5,5,WIDTH-10,HEIGHT-10,5,FGCOLOR);

  readFs(Folder, fileList);

  for(int i=0; i<MAXFILES; i++) if(fileList[i][2]!="") maxFiles++; else break;
  while(1){
    if(returnToMenu) break; // stop this loop and retur to the previous loop

    if(redraw) { 
      if(strcmp(PreFolder.c_str(),Folder.c_str()) != 0 || reload){
        index=0;
        readFs(Folder, fileList);
        PreFolder = Folder;
        maxFiles=0;
        for(int i=0; i<MAXFILES; i++) if(fileList[i][2]!="") maxFiles++; else break;
        reload=false;
      }
      listFiles(index, fileList);

      delay(150);
      redraw = false;
    }

    if(checkPrevPress()) {
      if(index==0) index = maxFiles - 1;
      else if(index>0) index--;
      redraw = true;
    }
    /* DW Btn to next item */
    if(checkNextPress()) { 
      index++;
      if(index==maxFiles) index = 0;
      redraw = true;
    }

    /* Select to install */
    if(checkSelPress()) { 
      delay(200);
      if(checkSelPress()) 
      {
        while(checkSelPress()) yield();
        // Definição da matriz "Options" 
        if(fileList[index][2]=="folder") {
          options = {
            {"New Folder", [=]() { createFolder( Folder); }},
            {"Rename", [=]() { renameFile(fileList[index][1], fileList[index][0]); }},
            {"Delete", [=]() { deleteFromSd(fileList[index][1]); }},
            {"Main Menu", [=]() { returnToMenu=true; }},
          };
          delay(200);
          loopOptions(options);
          tft.drawRoundRect(5,5,WIDTH-10,HEIGHT-10,5,FGCOLOR);  
          reload = true;     
          redraw = true;
        } else if(fileList[index][2]=="file"){
          goto Files;
        } else {
          options = {
            {"New Folder", [=]() { createFolder(Folder); }},
          };
          if(fileToCopy!="") options.push_back({"Paste", [=]() { pasteFile(Folder); }});
          options.push_back({"Main Menu", [=]() { returnToMenu=true; }});
          delay(200);
          loopOptions(options);
          reload = true;  
          redraw = true;
        }
      } else {
        Files:
        if(fileList[index][2]=="folder") {
          Folder = fileList[index][1];
          redraw=true;
        } else if (fileList[index][2]=="file") {
          options = {
            {"Install", [=]() { updateFromSD(fileList[index][1]); }},
            {"New Folder", [=]() { createFolder(Folder); }},
            {"Rename", [=]() { renameFile(fileList[index][1], fileList[index][0]); }},
            {"Copy", [=]() { copyFile(fileList[index][1]); }},
          };
          if(fileToCopy!="") options.push_back({"Paste",  [=]() { pasteFile(Folder); }});
          options.push_back({"Delete", [=]() { deleteFromSd(fileList[index][1]); }});
          options.push_back({"Main Menu", [=]() { returnToMenu=true; }});
          delay(200);
          loopOptions(options);
          reload = true;  
          redraw = true;
        } else {
          if(Folder == "/") break;
          Folder = fileList[index][1];
          index = 0;
          redraw=true;
        }
        redraw = true;
      }
      tft.fillSmoothRoundRect(6,6,WIDTH-12,HEIGHT-12,5,BGCOLOR);
      tft.drawRoundRect(5,5,WIDTH-10,HEIGHT-10,5,FGCOLOR);
      redraw = true;
    }

    #ifdef CARDPUTER
      Keyboard.update();
      if(Keyboard.isKeyPressed('`')) break;
    #endif
  }
  //clear fileList memory
  for(int i=0; i<MAXFILES; i++) {
    fileList[i][0] = "";
    fileList[i][1] = "";
    fileList[i][2] = "";
  }
  closeSdCard();
  setupSdCard();  
  tft.fillScreen(BGCOLOR);
}
/***************************************************************************************
** Function name: performUpdate
** Description:   this function performs the update 
***************************************************************************************/
void performUpdate(Stream &updateSource, size_t updateSize, int command) {
  // command = U_FAT_vfs = 300 
  // command = U_FAT_sys = 400 
  // command = U_SPIFFS = 100
  // command = U_FLASH = 0

  tft.fillSmoothRoundRect(6,6,WIDTH-12,HEIGHT-12,5,BGCOLOR);
  progressHandler(0, 500);

  if (Update.begin(updateSize, command)) {
    int written = 0;
    uint8_t buf[1024];
    int bytesRead;

  #ifndef STICK_C_PLUS
    //Erase FAT partition
    eraseFAT();
  #endif

    prog_handler = 0; // Install flash update
    if (command==U_SPIFFS || command == U_FAT_vfs || command == U_FAT_sys) prog_handler = 1; // Install flash update
    log_i("updateSize = %d",updateSize);
    while (written < updateSize) { //updateSource.available() > 0 && 
      bytesRead = updateSource.readBytes(buf, sizeof(buf));
      written += Update.write(buf, bytesRead); 
      progressHandler(written, updateSize);
    }
    if (Update.end()) {
      if (Update.isFinished()) log_i("Update successfully completed.");
      else log_i("Update not finished? Something went wrong!");
    }
    else {
      log_i("Error Occurred. Error #: %s", String(Update.getError()));
    }
  }
  else
  {
    uint8_t error = Update.getError();
    displayRedStripe("E:" + String(error) + "-Wrong Partition Scheme");
    delay(2000);
  }
}

/***************************************************************************************
** Function name: updateFromSD
** Description:   this function analyse the .bin and calls performUpdate
***************************************************************************************/
void updateFromSD(String path) {
  uint8_t firstThreeBytes[16];

  uint32_t spiffs_offset = 0;
  uint32_t spiffs_size = 0;
  uint32_t app_size = 0;
  bool spiffs = false;

  uint32_t fat_offset_sys = 0;
  uint32_t fat_size_sys = 0;
  uint32_t fat_offset_vfs = 0;
  uint32_t fat_size_vfs = 0;  
  bool fat = false;

  File file = SD.open(path);
  //resetTftDisplay();
  displayRedStripe("Preparing..");

  if (!file) goto Exit;
  if (!file.seek(0x8000)) goto Exit; 
  file.read(firstThreeBytes, 16);
  // Read 3 bytes of the file at position 0x8000 (in order to see if it is partitions information)
  if (firstThreeBytes[0] != 0xAA || firstThreeBytes[1] != 0x50 || firstThreeBytes[2] != 0x01) {
    // Install binaries made to be flashed at 0x10000
    if (!file.seek(0x0)) goto Exit;
    //file.read(firstThreeBytes, 16);
    // File is a firmware binary made to flash at 0x10000 address
    performUpdate(file, file.size(), U_FLASH);

    file.close();
    tft.fillScreen(BGCOLOR);
    ESP.restart(); 
  } 
  // Start Verifications to install binaries made to be flashed at 0x0
  else {
    if (!file.seek(0x8000)) goto Exit;
    for(int i=0; i<0x0A0;i+=0x20) {
      //Serial.print((0x8000+i),HEX);
      if (!file.seek(0x8000+i)) goto Exit;
      file.read(firstThreeBytes, 16);
      if((firstThreeBytes[0x03]==0x00 || firstThreeBytes[0x03]==0x10 || firstThreeBytes[0x03]==0x20) && firstThreeBytes[0x06]==0x01) {
        app_size = (firstThreeBytes[0x0A] << 16) | (firstThreeBytes[0x0B] << 8) | 0x00; // Sets max size of partition.

        if(file.size()<(app_size+0x10000)) app_size = file.size() - 0x10000;
        else if(app_size>MAX_APP) app_size = MAX_APP; 

      }
      if(firstThreeBytes[3] == 0x82) { // SPIFFS
        spiffs_offset = (firstThreeBytes[0x06] << 16) | (firstThreeBytes[0x07] << 8) | firstThreeBytes[0x08];	// Write the offset of spiffs partition
        spiffs_size = (firstThreeBytes[0x0A] << 16) | (firstThreeBytes[0x0B] << 8) | 0x00;	                  // Write the size of spiffs partition
        if(file.size()<spiffs_offset) spiffs=false;                                                           // check if there is room for spiffs in file
        else if(spiffs_size>MAX_SPIFFS) { spiffs_size = MAX_SPIFFS; spiffs = true; }                          // if there is spiffs in file, check its size
        if(spiffs && file.size()<(spiffs_offset+spiffs_size)) spiffs_size = file.size() - spiffs_offset;      // if there is spiffs, check if spiffs size is lesser then maximum
      } 
      if(firstThreeBytes[3] == 0x81 && firstThreeBytes[0x0C] == 0x73) { // FAT sys //
        fat_offset_sys = (firstThreeBytes[0x06] << 16) | (firstThreeBytes[0x07] << 8) | firstThreeBytes[0x08];	// Write the offset of spiffs partition
        fat_size_sys = (firstThreeBytes[0x0A] << 16) | (firstThreeBytes[0x0B] << 8) | 0x00;	                  // Write the size of spiffs partition
        if(file.size()<fat_offset_sys) fat=false;                                                             // check if there is room for spiffs in file
        else  fat = true;
        if(fat && fat_size_sys>MAX_FAT_sys) { fat_size_sys = MAX_FAT_sys;  }     // if there is spiffs in file, check its size
        if(fat && file.size()<(fat_offset_sys+fat_size_sys)) fat_size_sys = file.size() - fat_offset_sys; // if there is spiffs, check if spiffs size is lesser then maximum
        
      }
      if(firstThreeBytes[3] == 0x81 && firstThreeBytes[0x0C] == 0x76) { // FAT vfs  
        fat_offset_vfs = (firstThreeBytes[0x06] << 16) | (firstThreeBytes[0x07] << 8) | firstThreeBytes[0x08];	// Write the offset of spiffs partition
        fat_size_vfs = (firstThreeBytes[0x0A] << 16) | (firstThreeBytes[0x0B] << 8) | 0x00;	                  // Write the size of spiffs partition
        if(file.size()<fat_offset_vfs) fat=false;                                                             // check if there is room for spiffs in file
        else  fat = true;
        if(fat && fat_size_vfs>MAX_FAT_vfs) { fat_size_vfs = MAX_FAT_vfs; }     // if there is spiffs in file, check its size
        if(fat && file.size()<(fat_offset_vfs+fat_size_vfs)) fat_size_vfs = file.size() - fat_offset_vfs; // if there is spiffs, check if spiffs size is lesser then maximum
      }
    }
    log_i("Appsize: %d",app_size);
    log_i("Spiffsize: %d",spiffs_size);
    log_i("FATsize[0]: %d - max: %d",fat_size_sys, MAX_FAT_sys);
    log_i("FATsize[1]: %d - max: %d",fat_size_vfs, MAX_FAT_vfs);
    log_i("FAT: %d",fat);
    log_i("------------------------");
  
    if(!fat) {
      fat_size_sys = 0;
      fat_size_vfs = 0;
      fat_offset_sys = 0;
      fat_offset_vfs = 0;
    } 

    prog_handler = 0; // Install flash update
    if(spiffs && askSpiffs) {
      options = {
        {"SPIFFS No", [&](){ spiffs = false; }},
        {"SPIFFS Yes", [&](){ spiffs = true; }},
      };
      delay(200);
      loopOptions(options);
      tft.fillSmoothRoundRect(6,6,WIDTH-12,HEIGHT-12,5,BGCOLOR);
    }
    
    log_i("Appsize: %d",app_size);
    log_i("Spiffsize: %d",spiffs_size);
    log_i("FATsize[0]: %d - max: %d",fat_size_sys, MAX_FAT_sys);
    log_i("FATsize[1]: %d - max: %d",fat_size_vfs, MAX_FAT_vfs);
    
    if (!file.seek(0x10000)) goto Exit;
    performUpdate(file, app_size, U_FLASH);

    prog_handler = 1; // Install SPIFFS update
    if(spiffs) {
      if (!file.seek(spiffs_offset)) goto Exit;
      performUpdate(file, spiffs_size, U_SPIFFS);
    }

  #if !defined(STICK_C_PLUS)
    if(fat) {
      eraseFAT();
      
      if(fat_size_sys>0) {
        if (!file.seek(fat_offset_sys)) goto Exit;
        if(!performFATUpdate(file, fat_size_sys, "sys")) log_i("FAIL updating FAT sys");
      }
      if(fat_size_vfs>0) {
        if (!file.seek(fat_offset_vfs)) goto Exit;
        if(!performFATUpdate(file, fat_size_vfs, "vfs")) log_i("FAIL updating FAT sys");
      }
    }
  #endif
    ESP.restart();

  }

Exit:
  displayRedStripe("Error on updating.");
}


/***************************************************************************************
** Function name: performFATUpdate
** Description:   this function performs the update 
***************************************************************************************/
bool performFATUpdate(Stream &updateSource, size_t updateSize,  const char *label) {
  uint8_t* buffer = (uint8_t*)heap_caps_malloc(4096, MALLOC_CAP_INTERNAL);
  if (buffer == NULL) {
      ESP_LOGE("FLASH", "Failed to allocate buffer in DRAM");
      return false;
  }  
  // Preencher o buffer com 0xFF
  memset(buff, 0xFF, 4096);
  log_i("Start updating: %s", label);

  const esp_partition_t* partition;
  esp_err_t error;
  size_t paroffset=0;
  int written=0;
  int bytesRead=0;
  partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, label);
  if(!partition){
      error = UPDATE_ERROR_NO_PARTITION;
      return false;
  }
  paroffset = partition->address;
  log_i("Erasing updating: %s", label);
  error = spi_flash_erase_range(partition->address, partition->size);
  if(error != ESP_OK) {
      log_i("Erase error %d", error);
      return false;
  }
  progressHandler(0,500);
  displayRedStripe("Updating FAT");
  log_i("Updating updating: %s", label);
  while (updateSource.available() && written < updateSize) { //updateSource.available() > 0 && 
    //memset(buff, 0xFF, 4096);
    bytesRead = updateSource.readBytes(buff, sizeof(buff));
    //memcpy(buffer, buff, bytesRead);
    written += bytesRead;
    error = spi_flash_write(paroffset,buff,sizeof(buff)); 
    paroffset+=bytesRead;
    progressHandler(written, updateSize);
    if (error != ESP_OK) {
        log_i("[FLASH] Failed to write to flash (0x%x)", error);
        heap_caps_free(buffer);
        return false;
    }    
  }
  if(written==updateSize)  {
    log_i("Success updating %s", label);
  }
  else {
    log_i("FAIL updating %s", label);
    return false;
  }
    heap_caps_free(buffer);
    return true;
}
