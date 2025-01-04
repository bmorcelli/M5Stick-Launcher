#include <globals.h>

/*********************************************************************
** Function: touchHeatMap
** Touchscreen Mapping, include this function after reading the touchPoint
**********************************************************************/
void touchHeatMap(struct TouchPoint t) {
    int third_x=tftWidth/3;
    int third_y=tftHeight/3;
    
    if(t.x>third_x*0 && t.x<third_x*1 && t.y>third_y) PrevPress = true;
    if(t.x>third_x*1 && t.x<third_x*2 && ((t.y>third_y && t.y<third_y*2) || t.y>tftHeight)) SelPress = true; 
    if(t.x>third_x*2 && t.x<third_x*3) NextPress = true;
    if(t.x>third_x*0 && t.x<third_x*1 && t.y<third_y) EscPress = true;
    if(t.x>third_x*1 && t.x<third_x*2 && t.y<third_y) UpPress = true; 
    if(t.x>third_x*1 && t.x<third_x*2 && t.y>third_y*2 && t.y<third_y*3) DownPress = true; 
/*
                    Touch area Map
            ________________________________ 0
            |   Esc   |   UP    |         |
            |_________|_________|         |_> third_y
            |         |   Sel   |         |
            |         |_________|  Next   |_> third_y*2
            |  Prev   |  Down   |         |
            |_________|_________|_________|_> third_y*3
            |__Prev___|___Sel___|__Next___| 20 pixel touch area where the touchFooter is drawn
            0         L third_x |         |
                                Lthird_x*2|
                                          Lthird_x*3
*/
}