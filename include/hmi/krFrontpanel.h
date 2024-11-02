#ifndef FRONTPANEL_H
#define FRONTPANEL_H

void front_init();

void front_multibuttons_loop();
void front_read_pots();
void front_led_on(uint8_t led);
void front_led_off(uint8_t led);
void front_read_buttons();
void front_read_encoder();
void front_read_ldr();

#endif