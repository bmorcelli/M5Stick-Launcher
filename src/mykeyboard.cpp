#include "mykeyboard.h"
#include "display.h"
#include "globals.h"
#include "settings.h"


/* Verifies Upper Btn to go to previous item */
bool checkNextPress(){ return false; }

/* Verifies Down Btn to go to next item */
bool checkPrevPress() { return false; }

/* Verifies if Select or OK was pressed */
bool checkSelPress(){ return false; }


/***************************************************************************************
** Function name: getBattery()
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() { return 0; }


/* Starts keyboard to type data */
String keyboard(String mytext, int maxSize, String msg) { return ""; }

/* Turns off device */
void powerOff() { }

/* Verifies if the appropriate btn was pressed to turn off device */
void checkReboot() { }