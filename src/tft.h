#ifndef __TFT_H
#define __TFT_H

#if defined(E_PAPER_DISPLAY)
#include <EPD_translate.h>
#define DARKGREY TFT_DARKGREY
#define BLACK TFT_BLACK
#define RED TFT_RED
#define GREEN TFT_GREEN
class Ard_eSPI: public EPD_translate {
    public:
    inline int getTextsize() { return textsize; };
    inline uint16_t getTextcolor() { return textcolor; };
    inline uint16_t getTextbgcolor() { return textbgcolor; };
    inline void drawChar(char c, uint32_t x, uint32_t y, uint16_t a, uint16_t b) { EPD_translate::drawChar(c,x,y); };
    inline void drawArc(int a, int b, int c, int d, int e, int f, int g) { };
    inline void begin() {EPD_translate::init(); };
    private:

};
#elif defined (HEADLESS)
// do nothing
#else

#include <Arduino_GFX_Library.h>

#if ST7789_DRIVER
#define _TFT_DRV Arduino_ST7789
#define _TFT_DRVF(a,b,c,d,e,f,g,h,i,j) Arduino_ST7789(a,b,c,d,e,f,g,h,i,j)
#elif ST7735_DRIVER
#define _TFT_DRV Arduino_ST7735
#define _TFT_DRVF(a,b,c,d,e,f,g,h,i,j) Arduino_ST7735(a,b,c,d,e,f,g,h,i,j)
#elif ILI9342_DRIVER
#define _TFT_DRV Arduino_ILI9342
#define _TFT_DRVF(a,b,c,d,e,f,g,h,i,j) Arduino_ILI9342(a,b,c,d)
#elif ST7796_DRIVER
#define _TFT_DRV Arduino_ST7796
#define _TFT_DRVF(a,b,c,d,e,f,g,h,i,j) Arduino_ST7796(a,b,c,d,e,f,g,h,i,j)
#endif

class Ard_eSPI: public _TFT_DRV {
    public:
    Arduino_ST7789 *gfx;  // Instância do driver real

    // Construtor que inicializa o driver
    Ard_eSPI(Arduino_DataBus *bus, int8_t rst, uint8_t rotation, bool ips, uint8_t w, uint8_t h, uint8_t co1, uint8_t ro1 ,uint8_t co2, uint8_t ro2)
        : _TFT_DRVF(bus, rst, rotation, ips, w, h,co1,ro1,co2,ro2) {}
    
    void drawString(String s, uint16_t x, uint16_t y);
    void drawCentreString(String s, uint16_t x, uint16_t y, int f);
    void drawRightString(String s, uint16_t x, uint16_t y, int f);
    inline int getTextsize() { return textsize_x; };
    inline uint16_t getTextcolor() { return textcolor; };
    inline uint16_t getTextbgcolor() { return textbgcolor; };

    private:
  
};
#endif
#endif //__TFT_H