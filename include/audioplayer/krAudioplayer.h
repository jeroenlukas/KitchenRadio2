#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <VS1053.h>

#include "configuration/config.h"
#include "cbuf_ps.h"

extern VS1053 player;

extern cbuf_ps circBuffer;

extern uint8_t audioplayer_soundMode;

extern char readBuffer[4096] __attribute__((aligned(4)));

void audioplayer_init();
void audioplayer_setvolume(uint8_t volume);
void audioplayer_set_soundmode(uint8_t soundMode);
void audioplayer_feedbuffer();
void audioplayer_flushbuffer();
void audioplayer_set_mute(bool mute);

void audioplayer_setbass(int8_t bass_gain);
void audioplayer_settreble(int8_t treble_gain);

#endif
