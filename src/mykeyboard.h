#pragma once
#include "globals.h"
#include <Arduino.h>

String  __attribute__((weak)) keyboard(String mytext, int maxSize = 76, String msg = "Type your message:");

bool __attribute__((weak)) checkNextPress();

bool __attribute__((weak)) checkPrevPress();

bool __attribute__((weak)) checkSelPress();

bool __attribute__((weak)) checkEscPress();

bool __attribute__((weak)) checkAnyKeyPress();

void __attribute__((weak)) powerOff();

void __attribute__((weak)) checkReboot();

//It is here to not create a new set of files just to use AXP192 library again
int __attribute__((weak)) getBattery();
