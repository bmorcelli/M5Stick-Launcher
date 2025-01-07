#ifndef __EPD_TRANSLATE_H
#define __EPD_TRANSLATE_H

#include <Arduino.h>
#include <epd_driver.h>
#include "firasans.h"
#include <SPI.h>

#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFD20      /* 255, 165,   0 */
#define TFT_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define TFT_PINK        0xF81F


class EPD_translate {
    protected:
    uint8_t fg_color;
    uint8_t bg_color;
    int32_t cursor_x;
    int32_t cursor_y;
    uint8_t *framebuffer = NULL;
    inline uint8_t getColorFrom16(uint16_t c) {
        uint32_t r = ((c >> 11) & 0x1F)*254/31;
        uint32_t g = ((c >> 5) & 0x3F)*254/63;
        uint32_t b = (c & 0x1F)*254/31;
        return (r+g+b)/3; // Assuming 0 to white and 254 to black
    }
    public:
    uint32_t textsize; /*where is the text size in EPD???*/
    uint8_t textcolor = fg_color;
    uint8_t textbgcolor = bg_color;
    inline void init() { 
        framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
        if (!framebuffer) {
            Serial.println("alloc memory failed !!!");
            while (1);
        }
        memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2); 
        epd_init();  
        };
    inline SPIClass &getSPIinstance() { return SPI; };
    inline uint16_t width() { return EPD_WIDTH; };
    inline uint16_t height() { return EPD_HEIGHT; };
    inline void setAttribute(int x,bool t) { /*Not implemented */ };
    inline void setRotation(int rot) { }; /*Not implemented*/
    inline void setTextColor(uint16_t fgcolor,uint16_t bgcolor) { fg_color = getColorFrom16(fgcolor); textcolor=fg_color;  bg_color = getColorFrom16(bgcolor); textbgcolor=bg_color; }; /*Attention point, crazy conversion*/
    inline void setTextColor(uint16_t fgcolor) { fg_color = getColorFrom16(fgcolor); textcolor=fg_color; }; /*Attention point, crazy conversion*/
    inline void setCursor(uint32_t x, uint32_t y) { cursor_x=x; cursor_y=y; };
    inline void fillRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t color) { epd_draw_rect(x, y, w, h, getColorFrom16(color), framebuffer); };
    inline void print(String t) { write_string((GFXfont *)&FiraSans, t.c_str(), &cursor_x, &cursor_y, framebuffer); }; //framebuffer or NULL??
    inline void print(char c){ write_string((GFXfont *)&FiraSans, &c, &cursor_x, &cursor_y, framebuffer); }; //framebuffer or NULL??
    inline void println(String t="") { write_string((GFXfont *)&FiraSans, String(t + "\n").c_str(), &cursor_x, &cursor_y, framebuffer); }; //framebuffer or NULL??
    inline void fillScreen(uint16_t c) { epd_clear(); if(c>0) fillRect(0,0, EPD_WIDTH, EPD_HEIGHT, getColorFrom16(c)); };
    inline void setTextSize(uint32_t c) { /*Not implemented in EPD? */ };
    inline uint32_t getCursorY() { return cursor_y; };
    inline uint32_t getCursorX() { return cursor_x; };

    inline void drawRect(uint32_t x,uint32_t y,uint32_t w,uint32_t h,uint16_t c) { epd_draw_rect(x, y, w, h, getColorFrom16(c), framebuffer); };
    inline void drawRoundRect(uint32_t x,uint32_t y,uint32_t w,uint32_t h,uint32_t r,uint16_t c) { epd_draw_rect(x, y, w, h, getColorFrom16(c), framebuffer); /*No round rect for now*/ };
    inline void fillRoundRect(uint32_t x,uint32_t y,uint32_t w,uint32_t h,uint32_t r,uint16_t c) { epd_fill_rect(x, y, w, h, getColorFrom16(c), framebuffer); /*No round rect for now*/ };
    inline void fillSmoothRoundRect(uint32_t x,uint32_t y,uint32_t w,uint32_t h,uint32_t r,uint16_t c) { epd_fill_rect(x, y, w, h, getColorFrom16(c), framebuffer); /*No smooth rect for now*/ };
    inline void drawArc(uint32_t x,uint32_t y,uint32_t r0,uint32_t r1,uint32_t ang1,uint32_t ang2,uint16_t c, uint16_t bg, bool b) {
        epd_fill_circle( x, y, r0, getColorFrom16(c), framebuffer);
    };

    inline void drawLine(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint16_t c) { epd_draw_line(x0, y0, x1, y1,  getColorFrom16(c), framebuffer); };

    inline void drawString(String t,int x,int y) {
        write_string((GFXfont *)&FiraSans, t.c_str(), &x, &y, framebuffer); //framebuffer or NULL??
    };

    inline void drawRightString(String t,int x,int y,uint32_t f) {
        int32_t x0, y0, x2,y2, w, h;
        get_text_bounds((GFXfont *)&FiraSans, t.c_str(), &x, &y,
                     &x2, &y2, &w, &h, NULL);
        x0 = x-(x2-x);
        y0 = y-(y2-y);
        write_string((GFXfont *)&FiraSans, t.c_str(), &x0, &y0, framebuffer); //framebuffer or NULL??
     };

    inline void drawCentreString(String t,int x,int y,uint32_t f) { 
        int32_t x0, y0, x2,y2, w, h;
        get_text_bounds((GFXfont *)&FiraSans, t.c_str(), &x, &y,
                     &x2, &y2, &w, &h, NULL);
        x0 = x-(x2);
        y0 = y-(y2);
        write_string((GFXfont *)&FiraSans, t.c_str(), &x0, &y0, framebuffer); //framebuffer or NULL??
     };

     inline void drawChar(char c, int x, int y) { write_string((GFXfont *)&FiraSans, &c, &x, &y, framebuffer); }; //framebuffer or NULL??





};


#endif /*__EPD_TRANSLATE_H*/