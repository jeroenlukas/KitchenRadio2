#ifndef KR_LAMP_H
#define KR_LAMP_H

#include <Arduino.h>

#define LAMP_EFFECT_NONE        0   // No effect
#define LAMP_EFFECT_COLORFADE   1   // Fade through the complete color range

void lamp_init();

void lamp_toggle();
void lamp_setcolor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);
void lamp_off();
void lamp_sethue(float hue);
void lamp_setsaturation(float saturation);
void lamp_setlightness(float lightness);
void lamp_seteffecttype(uint8_t effect);
void lamp_seteffectspeed(float speed);

#endif