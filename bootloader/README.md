# M5Launcher Bootloader source files
M5Launcher modifies the "bootloader override" example from ESP-IDF, as shown below, to manage witch partition the device should boot when we turn on the device.

The code is very simple: 
* If the device is being "turned on", it launches the "test" partition
* If the device has been rebooted (by software or other problem), it launches the OTA_0 partition, where the app you've launched resides.

To compile this project you must:
* Install VSCode (Visual Studio Code)
* Install ESP-IDF complement (using VSCode), I have used version 5.0.4 release, but it might work from 4.x to the last version.
* Rename the "sdkconfig.Cardputer" into "sdkconfig" (without the "dot" -> .Cardputer or .StickCP2 or the other... this file doesn't have extension).
* Open this project folder
* Hit the Build button

As result of the building process, you will have many files in the build folder.
* `bootloader.bin` resides in the folder: `/build/bootloader`
* `partition-table.bin` resides in the folder: `/build/partition_table`
* `ota_data_inicial.bin` resides in the folder `/build`





# Original Readme from ESP-IDF bootloader override example

| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- |

# Bootloader override

(See the README.md file in the upper level for more information about bootloader examples.)

The purpose of this example is to show how to override the second stage bootloader from a regular project.

**NOTE**: Functions called during the loading stage of the bootloader are expected to be placed in the iram_loader_seg to avoid being overwritten during loading. If you are overriding functions which are called during this stage then special care needs to be taken to avoid issues, e.g. by providing your own linkerscript which places the required functions in the correct sections.

## How to use example

Simply compile it:
```
idf.py build
```

And flash it with the following commands:
```
idf.py flash
```

This custom bootloader does not do more than the older bootloader, it only prints an extra message on start up:
```
[boot] Custom bootloader message defined in the KConfig file.
```

## Organisation of this example

This project contains an application, in the `main` directory that represents a user program.
It also contains a `bootloader_components` directory that, as it name states, contains a component that will override the current bootloader implementation.

Below is a short explanation of files in the project folder.

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c                 User application
├── bootloader_components
│   └── main
│       ├── component.mk
│       ├── CMakeLists.txt
│       ├── ld/
│       │   └── ...
│       └── bootloader_start.c Implementation of the second stage bootloader
└── README.md                  This is the file you are currently reading
```

As stated in the `README.md` file in the upper level, when the bootloader components is named `main`, it overrides
the whole second stage bootloader code.
