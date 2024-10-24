#ifndef FRONTPANEL_H
#define FRONTPANEL_H

// Flags to signal change
//extern bool f_front_pot_volume_changed;
/*extern bool f_front_pot_treble_changed;
extern bool f_front_pot_bass_changed;

extern bool f_front_button_encoder_pressed;

extern bool f_front_encoder_turn_left;
extern bool f_front_encoder_turn_right;

extern bool f_button_off_pressed ;
extern bool f_button_radio_pressed;
extern bool f_button_bluetooth_pressed;*/

// Pot values
extern int front_pot_vol;

void frontpanel_setup();
void front_multibuttons_loop();
void front_read_pots();

void front_led_on(uint8_t led);
void front_led_off(uint8_t led);
void front_read_buttons();
//void front_read_encoder();

#endif