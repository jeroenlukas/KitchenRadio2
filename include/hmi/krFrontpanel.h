#ifndef FRONTPANEL_H
#define FRONTPANEL_H

extern void front_init();

extern void front_led_on(uint8_t led);
extern void front_led_off(uint8_t led);
extern void front_buttons_read();
extern void front_encoders_read();
extern void front_ldr_read();
extern void front_handle();

#endif