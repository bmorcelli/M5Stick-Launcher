# M5Stick Launcher
Application Launcher for Cardputer, M5StickC, M5StickC Plus, M5StickC Plus 2, Core (16Mb), Core2 & Tough and CoreS3.


<p align="center" width="100%">
    <img src="https://github.com/bmorcelli/M5Stick-Launcher/blob/main/M5Launcher.png?raw=true">  <img src="https://github.com/bmorcelli/M5Stick-Launcher/blob/main/New Launcher.jpg?raw=true" width="240" height="135"> 
</p>

## With M5Launcher you'll be able to:
### *OTA* OTA Update
- Install binaries from M5Burner repository (yes, online, without the need of a USB Cable)
- Install binaries from a WebUI, that you can start from CFG option, installing binaries you have on your computer or smartphone
- Install binaries from your SD Card

### *SD* SD Card Management
- Create new Folders,
- Delete files and folders,
- Rename files,
- Copy and paste files,
- Install binaries

### *WUI* Web User Interface 
- 

### *CFG* Configurations (Customization)
- Charge Mode
- Change brightness
- Change Dim Time
- Change UI Color
- Avoid/Ask Spiffs (Change to not ask to install Spiffs file system, only Orca One uses this feature)
- Change rotation
- All files/Only Bins (see all files or only .bins - default)
- Change Partition Scheme (allows installing big apps or UiFlow2, for example)
- List of Partitions
- Clear FAT partition
- Save SPIFFS (Save a copy of the SPIFFS partition to restore when needed)
- Save FAT vfs (Save a copy of the FAT partition to restore when needed)
- Restore SPIFFS
- Restore FAT vfs

## tips
* Having an SD card is good for better experience, but not really needed. [SDCard Hat for M5StickCs](https://www.thingiverse.com/thing:6459069) 
* You can learn more about how it works o [M5Launcher Wiki](https://github.com/bmorcelli/M5Stick-Launcher/wiki/Explaining-the-project).
* Where/How do I find Binaries to launch -> [Obtaining binaries to launch](https://github.com/bmorcelli/M5Stick-Launcher/wiki/Obtaining-binaries-to-launch)
* Now you can download binaries from [HERE!](https://bmorcelli.github.io/M5Stick-Launcher/m5lurner.html)

## How to Install
* Use the Flasher: [M5Launcher Flasher](https://bmorcelli.github.io/M5Stick-Launcher/flash0.html).
* Use M5Burner, or
* Download the .bin file from Releases for your device and use https://web.esphome.io/ or esptool.py and flash the file: `Launcher-{ver}-{YourDevice}.bin` into your device.

## How to Use
* Turn your Device on
* Press M5 (Enter) in the Launcher Start Screen to start Launcher
* Choose OTA to install new binaries from M5Burner repo
* After installed, when turn on the device, if you don't press anything, the installed program will be launched.

## Install from source
Sourcecode will be released in the future..

## Known Issues
* UiFlow 1 doesn´t work with M5Launcher.. it uses an old MicroPython distro, that uses an old ESP-IDF distro with lots os secrets that I couldn´t figure out.

## Changelog
* 2.2.0: 
     * [x] M5Launcher 2.2+ now can be updated OverTheAir or Using SD Card
     * [x] Ui Color settings (can be customized on /config.conf file)
     * [x] Reduced flickering on SD files navigation
     * [x] Fixed problem when firmware names have "/" that prevent downloading
     * [x] Appended firmware Version into download Name.
     * [x] Added Dim time to lower brightness and CPU freq while idle
     * [x] Added "Chage Mode" on settings, reducing CPU fre to 80Mhz and brightness to 5%, https://github.com/bmorcelli/M5Stick-Launcher/issues/40
     * [x] Fixed Download progressbar https://github.com/bmorcelli/M5Stick-Launcher/issues/41
     * [x] Change default folder for download (manually on /config.conf) https://github.com/bmorcelli/M5Stick-Launcher/issues/15     
     * [x] Save more SSIDs and pwd, connect automatically if is a known network (config.conf) https://github.com/bmorcelli/M5Stick-Launcher/issues/30
     * [x] Slightlty increased Wifi Download/OTA Speed, using a customized framework.
* 2.1.2:
     * [x] Fixed OTA error message
     * [x] Increased Options Menu width and reduced menu flickering
* 2.1.1:
     * [x] Fixed UIFlow Compatibility
     * [x] Fixed SD card issues
     * [x] Small Fixes https://github.com/bmorcelli/M5Stick-Launcher/issues/37
* 2.1.0:
     * [x] Core Fire (all 16Mb Core devices) and Core2 compatibility
     * [x] Turn SPIFFs update optional (turne off by default) (config.conf)
     * [x] De-Sprite-fied the screens for Core devices (No PSRam, unable to handle huge Sprites) https://github.com/bmorcelli/M5Stick-Launcher/issues/34
     * [x] Make keyboard work with touchscreen capture in Core devices
     * [x] Dedicated btn for WebUI on main screen https://github.com/bmorcelli/M5Stick-Launcher/issues/22
     * [x] Multiple files upload on WebUI https://github.com/bmorcelli/M5Stick-Launcher/issues/28
     * [x] Update FAT vfs partition to make compatible with UIFlow2 https://github.com/bmorcelli/M5Stick-Launcher/issues/29
     * [x] Partition changer, to allow running DOOM and UIFlow on Cardputer and StickC
     * [x] Fixed (increased number of files) https://github.com/bmorcelli/M5Stick-Launcher/issues/33
     * [x] Backup and Restore FAT and SPIFFS Filesystems. If you use UIFlow, you can save all sketches into your device and make a backup to restore after reinstall UIFlow or MicroHydra or CircuitPython
* 2.0.1:
     * Fixed UIFlow Instalation https://github.com/bmorcelli/M5Stick-Launcher/issues/20
     * Fixed Folder creation on WebUI https://github.com/bmorcelli/M5Stick-Launcher/issues/18
     * Fexed problem that preven webUI to open in some cases https://github.com/bmorcelli/M5Stick-Launcher/issues/16
     * Now M5Launcher formats FAT vfs partition, so make sure you have saved your data into SDCard when using MicroPython, UIFlow or MicroHydra https://github.com/bmorcelli/M5Stick-Launcher/issues/19
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
