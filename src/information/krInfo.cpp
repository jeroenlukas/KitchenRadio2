#include <Arduino.h>
#include "information/krInfo.h"
#include "esp_task_wdt.h"

Information information;

void info_init(void)
{
    information.timeShort = "00:00";
    information.dateMid = "?";
    
    information.system.uptimeSeconds = 0;
    information.system.wifiRSSI = 0;

    information.weather.temperature = 0.0;

    information.system.lastResetReason = (int)esp_reset_reason();
}

// Reset reason:
/*
    0   ESP_RST_UNKNOWN,    //!< Reset reason can not be determined
    1   ESP_RST_POWERON,    //!< Reset due to power-on event
    2   ESP_RST_EXT,        //!< Reset by external pin (not applicable for ESP32)
    3   ESP_RST_SW,         //!< Software reset via esp_restart
    4   ESP_RST_PANIC,      //!< Software reset due to exception/panic
    5   ESP_RST_INT_WDT,    //!< Reset (software or hardware) due to interrupt watchdog
    6   ESP_RST_TASK_WDT,   //!< Reset due to task watchdog
    7   ESP_RST_WDT,        //!< Reset due to other watchdogs
    8   ESP_RST_DEEPSLEEP,  //!< Reset after exiting deep sleep mode
    9   ESP_RST_BROWNOUT,   //!< Brownout reset (software or hardware)
    10  ESP_RST_SDIO,       //!< Reset over SDIO
    */