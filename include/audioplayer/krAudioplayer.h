#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <VS1053.h>

#include "configuration/config.h"
#include "cbuf_ps.h"

extern VS1053 player;

extern cbuf_ps circBuffer;

extern char readBuffer[4096] __attribute__((aligned(4)));

void audioplayer_init();

void audioplayer_feedbuffer();

void audioplayer_flushbuffer();

#endif
