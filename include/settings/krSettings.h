#ifndef KRSETTINGS_H
#define KRSETTINGS_H

#include <ArduinoJson.h>

extern JsonDocument settings;

bool settings_read_config();

#endif