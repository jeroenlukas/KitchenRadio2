#ifndef KR_TIME_H
#define KR_TIME_H

#include <Arduino.h>
#include <ezTime.h>

void time_init();
void time_waitForSync();

void time_update();

extern Timezone localTimezone;


#endif