#ifndef KR_MENU_H
#define KR_MENU_H

#include <U8g2lib.h>

extern int menu;
extern int menuitem;

extern void display_draw_menu();
extern void display_set_brightness(uint8_t brightness);
extern void display_set_brightness_auto();
extern void display_update_scroll_offset();
extern void display_reset_scroll();

extern U8G2_SSD1322_NHD_256X64_1_4W_HW_SPI u8g2;	// Enable U8G2_16BIT in u8g2.h

#endif