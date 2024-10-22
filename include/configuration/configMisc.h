#ifndef CONFIGMISC_H
#define CONFIGMISC_H

#define FORMAT_LITTLEFS_IF_FAILED true

// Volume pot hysteresis
#define POT_HYST    1

// Circular buffer size in PSRAM
#define CIRCBUFFER_SIZE 2 * 1024 * 1024 // 2 MB

// Minimum no. of bytes required in circular buffer to play
#define CONF_AUDIO_MIN_BYTES    256 * 1024

#define DISPLAY_RETURN_TIME    20 //*100 ms

// Multibutton ranges
#define BTN_ADC_OFF_MIN         3400
#define BTN_ADC_OFF_MAX         3600
#define BTN_ADC_RADIO_MIN       2700
#define BTN_ADC_RADIO_MAX       2900
#define BTN_ADC_BLUETOOTH_MIN   2150
#define BTN_ADC_BLUETOOTH_MAX   2320
#define BTN_ADC_SYSTEM_MIN      500
#define BTN_ADC_SYSTEM_MAX      600
#define BTN_ADC_ALARM_MIN       1600
#define BTN_ADC_ALARM_MAX       1730
#define BTN_ADC_LAMP_MIN        1050
#define BTN_ADC_LAMP_MAX        1150

// Log - note: these values are not pixels but the number of characters!
#define U8LOG_WIDTH 30
#define U8LOG_HEIGHT 10

// Fonts
#define U8LOG_FONT              u8g2_font_tom_thumb_4x6_mf
#define FONT_BOOTLOG
#define FONT_DEBUGLOG
#define FONT_S                  u8g2_font_simple1_tf

#endif
