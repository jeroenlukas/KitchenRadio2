#include <Arduino.h>

#include "logger.h"
#include "configuration/config.h"
#include "main.h"
#include "flags.h"
//#include "configMisc.h"

U8G2LOG u8g2log_boot;

U8G2LOG u8g2log_debug;


uint8_t u8log_buffer_boot[U8LOG_WIDTH * U8LOG_HEIGHT];
uint8_t u8log_buffer_debug[U8LOG_WIDTH * U8LOG_HEIGHT];





String logline;
bool updateLog = false;

// Boot log

void log_boot_begin()
{
    u8g2log_boot.begin(u8g2, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer_boot);
    u8g2log_boot.setRedrawMode(0);
}

// Print a boot message. Message is also written to Serial output by default
void log_boot(String line, bool log_to_serial)
{
    u8g2log_boot.print(line + "\n");
    if(log_to_serial)
    {
        Serial.println(">> " + line);
    }
}

void log_boot(String line)
{
    log_boot(line, true);
}


// Debug log

void log_debug_init()
{
    u8g2log_debug.begin(U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer_debug );
}

void log_debug(String line)
{
  logline = line + "\n";
  //updateLog = true;
  flags.main.updateLog = true;
}

// Should only be called from u8g2 draw loop!
void log_debug_print()
{
    u8g2log_debug.print(logline);
}

void log_debug_draw()
{
    u8g2.drawLog(90, 2, u8g2log_debug);
}