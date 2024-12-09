#ifndef KR_LAMP_H
#define KR_LAMP_H

#include <Arduino.h>

void lamp_init();

void lamp_toggle();
void lamp_setcolor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);
void lamp_off();

#endif