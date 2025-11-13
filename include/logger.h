#ifndef LOGGER_H
#define LOGGER_H

#include <U8g2lib.h>

extern String bootlog;

void log_boot(String line);
void printDebugLog(String line);
void log_boot_begin();
void log_debug_print();
void log_debug(String line);
void log_debug(String line, bool log_to_serial);
void log_debug_draw();
void log_debug_init();

#endif