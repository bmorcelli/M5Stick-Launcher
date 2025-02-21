#include "tft.h"

#if defined(E_PAPER_DISPLAY)

#elif defined(HEADLESS)

#else
void Ard_eSPI::drawCentreString(String s, uint16_t x, uint16_t y, int f) {
    
}

void Ard_eSPI::drawString(String s, uint16_t x, uint16_t y){

}

void Ard_eSPI::drawRightString(String s, uint16_t x, uint16_t y, int f) {
    
}
#endif