# M5Stick Launcher
Application Launcher for Cardputer, M5StickC, M5StickC Plus, and M5StickC Plus 2.


<p align="center" width="100%">
    <img src="https://github.com/bmorcelli/M5Stick-Launcher/blob/main/M5Launcher.png?raw=true"> => <img src="https://github.com/bmorcelli/M5Stick-Launcher/blob/main/New Launcher.jpg?raw=true" width="240" height="135"> 
</p>

## With M5Launcher you'll be able to:
### OTA Update
- Install binaries from M5Burner repository (yes, online, without the need of a USB Cable)
- Install binaries from a WebUI, that you can start from CFG option, installing binaries you have on your computer or smartphone
- Install binaries from your SD Card

### SD Card Management
- Create new Folders,
- Delete files and folders,
- Rename files,
- Copy and paste files,
- Install binaries

### Customization
- You can change brightness
- change rotation (StickCs)
- Start WebUI
- set to see all files on SD Card (see only .bins is default)

## tips
* Having an SD card is good for better experience, but not really needed. [SDCard Hat for M5StickCs](https://www.thingiverse.com/thing:6459069) 
* You can learn more about how it works o [M5Launcher Wiki](https://github.com/bmorcelli/M5Stick-Launcher/wiki/Explaining-the-project).
* Where/How do I find Binaries to launch -> [Obtaining binaries to launch](https://github.com/bmorcelli/M5Stick-Launcher/wiki/Obtaining-binaries-to-launch)

## How to Install
* Use the Flasher: [M5Launcher Flasher](https://bmorcelli.github.io/M5Stick-Launcher/flash0.html). (Unavailable for cardputer, yet)
* Use M5Burner, or
* Download the .bin file from Releases for your device and use https://web.esphome.io/ or esptool.py and flash the file: `Launcher-{ver}-{YourDevice}.bin` into your device.

## How to Use
* Turn your Device on
* Press M5 (Enter) in the Launcher Start Screen to start Launcher
* Choose OTA to install new binaries from M5Burner repo
* After installed, when turn on the device, if you don't press anything, the installed program will be launched.

## Install from source
Sourcecode will be released in the future..

## Changelog
* 2.0.0:
     * SD: added Folder creation, delete an rename files and folders, copy and paste files
     * OTA (Over-The-Air update): Added feature to list the programs available in M5Burner and install it fro the internet.
     * WebUI: Added a WebUI where you can manage your SD Card and install new binaries wirelessly
     * Some other minnor features
     * 
* 1.3.0:
     * Added support to Micropython based binaries (MicroHydra), with 1Mb FAT partition to Cardputer and StickCPlus2 and 64kb to StickC and Plus1.1
* 1.2.1:
     * Launcher now lower the LCD power and fill the screen black before restart, to prevent lcd burn when using apps that don't use the Screen
     * Fixed display things and positions for the M5StickC
* 1.2.0:
     * Excluded ota_data.bin file as it is not needed
     * Excluded StartApp application
     * Excluded OTA_1 partitions form .csv files because i found out it is not needed
     * Realocated free spaces into "SPIFFS" partition, giving room to improvements, and support to applications that use it (OrcaOne)
     * Added Bootscreen with battery monitor
     * Added Restart option and battery monitor to launcher
     * Added auto orientation to M5StickCs
     * Laucher does not create .bak files anymore!!
     * .bin file handling to avoid some errors: File is too big, file is not valid, etc etc..
     
* 1.1.3:
     * Fixed menu files that ware occasionally hiding files and folders.
* 1.1.2:
     * Adjusted Magic numbers to work with some apps (Volos Watch).
* 1.1.1: 
     * Changed OTA_0 Partition size from 3Mb to 6Mb on Cardputer and M5StickCPlus2
     * ~~Added verification to identify MicroPython binaries and don't corrupt them with the cropping process (these apps still don't work, need more work...)~~
* 1.1.0:
     * Fixed issues that prevented M5Launcher to launch apps on Cardputer
* 1.0.1:
     * Fixed blackscreen and keyboard capture on Cardputer.
