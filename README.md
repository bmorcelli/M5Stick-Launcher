# M5Stick Launcher
Application Launcher for M5StickC, M5StickC Plus, and M5StickC Plus 2.

Cardputer isn't working yet, need testing and debug. =(

Ir is mandatory that you have an [SDCard Hat](https://www.thingiverse.com/thing:6459069) to use with this Launcher and i have instructions [Here](https://www.thingiverse.com/thing:6459069), Cardputer users naturaly don´t need this Hat!

You can learn more about how it works o [M5Launcher Wiki](https://github.com/bmorcelli/M5Stick-Launcher/wiki/Explaining-the-project).

Where/How do I find Binaries to launch -> [Obtaining binaries to launch](https://github.com/bmorcelli/M5Stick-Launcher/wiki/Obtaining-binaries-to-launch)



## How to Install
* Download the ZIP file from Releases for your device.
* Use https://web.esphome.io/ or esptool.py and flash the file: `Launcher-{ver}-{YourDevice}.bin` into your device.

## How to Use
* Turn your Device on
* Press M5 (Enter) in the Launcher Start Screen to start Launcher
* Choose your .bin file and press M5 (Enter)
* After installed, when turn on the device, if you don't press anything, the installed program will be launched.


## Install from source
* Install the following libs:
    * [SdFat](https://github.com/greiman/SdFat)
    * [M5Stack-Sd-Updater](https://github.com/tobozo/M5Stack-SD-Updater/)
    * [M5GFX] (https://github.com/m5stack/M5GFX)
    * [M5Unified](https://github.com/m5stack/M5Unified)

* Open ~Launcher/Launcher.ino
* Build Launcher project in Arduino IDE, and "Export Compiled Binary"
* Copy the binary generated in "~Launcher/build/{your-device}/Launcher.ino.bin" the the folder "~support_files/" (there are some binaries inside there)
* Open ~StartApp/StartApp.ino
* Build FirstApp project in Arduino IDE, and "Export Compiled Binary"
* Copy the binary generated in "~FirstApp/build/{your-device}/FirstApp.ino.bin" the the folder "~support_files/" (there are some binaries inside there)
* use esptool to flash in your device
    * M5StickC and M5StickC Plus
        * Run in "~support_files\" folder:    "esptool -p COMx -b 460800 --before default_reset --after hard_reset --chip esp32 write_flash --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader_4Mb.bin 0x8000 partition-table_4Mb.bin 0xe000 ota_data_initial.bin  0x10000 Launcher.ino.bin  0xa0000 StartApp.ino.bin"
    * M5StickC Plus 2 and Cardputer
        * Run in "~support_files\" folder:    "esptool -p COMx -b 460800 --before default_reset --after hard_reset --chip esp32 write_flash --flash_mode dio --flash_freq 80m --flash_size detect 0x1000 bootloader_8Mb.bin 0x8000 partition-table_8Mb.bin 0xe000 ota_data_initial.bin  0x10000 Launcher.ino.bin  0xf0000 StartApp.ino.bin"


