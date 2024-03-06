#include "Update.h"

void DrawPercentage(int percent) {
  LNDISP.fillRect(20,80,percent*2,15,WHITE);
}


void performUpdate(Stream &updateSource, size_t updateSize, int command) {
  // command = U_SPIFFS = 100
  // command = U_FLASH = 0

   if (Update.begin(updateSize, command)) {      
      size_t written = 0;
      uint8_t buf[1024];
      int bytesRead;
      size_t totalSize = updateSize;
      
      Serial.println("Starting update...");
      while (updateSource.available() > 0 && written < updateSize) {
         bytesRead = updateSource.readBytes(buf, sizeof(buf));
         written += Update.write(buf, bytesRead);
         //Serial.printf("Progress: %d%%\n", (int)(written * 100 / totalSize));
         DrawPercentage((written*100)/totalSize);
      }

      if (written == updateSize) {
         Serial.println("Written : " + String(written) + " successfully");
      }
      else {
         Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
      }
      if (Update.end()) {
         Serial.println("OTA done!");
         if (Update.isFinished()) {
            Serial.println("Update successfully completed. Rebooting.");
         }
         else {
            Serial.println("Update not finished? Something went wrong!");
         }
      }
      else {
         Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }

   }
   else
   {
      Serial.println("Not enough space to begin OTA");
   }
}
