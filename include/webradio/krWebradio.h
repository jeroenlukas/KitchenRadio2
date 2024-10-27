#ifndef WEBRADIO_H
#define WEBRADIO_H

#include <Arduino.h>
//#include <WiFi.h>

void webradio_open_url(char * host, char * path);
void webradio_handle_stream(void);
bool webradio_buffered_enough(void);
bool webradio_isconnected();
void webradio_stop();
bool webradio_open_station(uint8_t index);
uint8_t webradio_get_num_stations();

extern uint8_t webradio_stationIndex;
extern uint8_t webradio_numStations;

#endif