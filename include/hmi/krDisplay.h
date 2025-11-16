#ifndef KR_MENU_H
#define KR_MENU_H

extern void draw_menu();
/*
enum menu_t {
    HOME, LAMP, SYSTEM, ALARM, MENU_COUNT
};

enum mitem_lamp_t { STATE, HUE, SATURATION, LIGHTNESS, EFFECTTYPE , EFFECTSPEED, MITEM_COUNT };
enum mitem_system_t { INFO, BASS, TREBLE, MITEM_COUNT };

uint8_t mitems[2];
mitem_lamp_t mitem_lamp;
mitem_system_t mitem_system;



menu_t menu;


enum menuitem {}*/

extern int menu;
extern int menuitem;

#endif