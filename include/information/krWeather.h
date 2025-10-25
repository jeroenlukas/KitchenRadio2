#ifndef WEATHER_H
#define WEATHER_H

bool weather_retrieve();

extern int weather_statecode_to_glyph(int statecode);
extern int weather_windkmh_to_beaufort(double wind_kmh);

#endif