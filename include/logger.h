#ifndef LOG_H
#define LOG_H

#include <U8g2lib.h>

void log_boot(String line);
void printDebugLog(String line);
void log_boot_begin();
void log_debug_print();
void log_debug(String line);
void log_debug_draw();
void log_debug_init();

#endif