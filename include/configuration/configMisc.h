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
#define BTN_ADC_OFF_MAX         3550
#define BTN_ADC_RADIO_MIN       2700
#define BTN_ADC_RADIO_MAX       2900
#define BTN_ADC_BLUETOOTH_MIN   2150
#define BTN_ADC_BLUETOOTH_MAX   2300
#define BTN_ADC_SYSTEM_MIN      1550
#define BTN_ADC_SYSTEM_MAX      1700
#define BTN_ADC_ALARM_MIN       400
#define BTN_ADC_ALARM_MAX       500
#define BTN_ADC_LAMP_MIN        980
#define BTN_ADC_LAMP_MAX        1100

// Log
#define U8LOG_WIDTH 100
#define U8LOG_HEIGHT 60

// Fonts
#define U8LOG_FONT  u8g2_font_smallsimple_tr
#define FONT_S  u8g2_font_simple1_tf

#endif
