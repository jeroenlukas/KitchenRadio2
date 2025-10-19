#include <Arduino.h>

#include "logger.h"
#include "configuration/config.h"
#include "information/krInfo.h"
#include "settings/krSettings.h"
#include "main.h"
#include "flags.h"
//#include "configMisc.h"

U8G2LOG u8g2log_boot;

U8G2LOG u8g2log_debug;


uint8_t u8log_buffer_boot[LOG_BOOT_WIDTH * LOG_BOOT_HEIGHT];
uint8_t u8log_buffer_debug[LOG_DEBUG_WIDTH * LOG_DEBUG_HEIGHT];


String bootlog;

String logline;
bool updateLog = false;

// Boot log

void log_boot_begin()
{
    u8g2log_boot.begin(u8g2, LOG_BOOT_WIDTH, LOG_BOOT_HEIGHT, u8log_buffer_boot);
    u8g2log_boot.setRedrawMode(0);

    bootlog = "*** Boot Log ***\n";
}

// Print a boot message. Message is also written to Serial output by default
void log_boot(String line, bool log_to_serial)
{
    u8g2log_boot.print(line + "\n");
    if(log_to_serial)
    {
        Serial.println(">> " + line);
    }

    char timestamp[12];
    itoa(millis(), timestamp, 10);
    
    bootlog.concat(String(timestamp) + " " + line + '\n');
}

void log_boot(String line)
{
    log_boot(line, true);
}


// Debug log

void log_debug_init()
{
    u8g2log_debug.begin(LOG_DEBUG_WIDTH, LOG_DEBUG_HEIGHT, u8log_buffer_debug );
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
    u8g2log_debug.print(information.timeShort + " " + logline);
}

void log_debug_draw()
{
    u8g2.setFont(FONT_DEBUGLOG);    

    // Add a log line if needed      
    if(flags.main.updateLog)
    {
    flags.main.updateLog = false;
    log_debug_print();
    }

    if(settings["homedisplay"] == "debug")
    {
        u8g2.drawLog(LOG_DEBUG_POSX + 3, LOG_DEBUG_POSY + 1, u8g2log_debug);

        // Draw a frame around the log window
        u8g2.drawFrame(LOG_DEBUG_POSX, LOG_DEBUG_POSY, LOG_DEBUG_WIDTH * u8g2.getMaxCharWidth(), (LOG_DEBUG_HEIGHT - 1) * u8g2.getMaxCharHeight()) ;
    }
}