#include <Arduino.h>
#include <ezTime.h>

#include "information/krTime.h"
#include "information/krInfo.h"
#include "settings/krSettings.h"
#include "logger.h"


Timezone localTimezone;

void time_init()
{
    const char * timeZone = settings["clock"]["timezone"];
    localTimezone.setLocation(timeZone);
    
}

void time_waitForSync()
{
    waitForSync(5);

    time_update();
}

void time_update()
{
    information.hour = localTimezone.hour(TIME_NOW, LOCAL_TIME);
    information.minute = localTimezone.minute(TIME_NOW, LOCAL_TIME);
    information.timeShort = String(u8x8_u8toa(information.hour, 2)) + ":" + String(u8x8_u8toa(information.minute, 2));
    information.dateMid = localTimezone.dateTime("D j M");
    Serial.println(information.timeShort);
}