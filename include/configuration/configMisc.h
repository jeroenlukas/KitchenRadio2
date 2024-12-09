#ifndef CONFIGMISC_H
#define CONFIGMISC_H

#define FORMAT_LITTLEFS_IF_FAILED true

// Volume pot hysteresis
#define POT_HYSTERESIS          20
#define POT_MA_SIZE             10

// Circular buffer size in PSRAM
#define CIRCBUFFER_SIZE         2 * 1024 * 1024 // 2 MB

// Minimum no. of bytes required in circular buffer to play
#define CONF_AUDIO_MIN_BYTES    256 * 1024

//#define DISPLAY_RETURN_TIME     20 //*100 ms

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

// Boot Log - note: these values are not pixels but the number of characters!
#define LOG_BOOT_WIDTH          40
#define LOG_BOOT_HEIGHT         10

// Debug Log (position in pixels, width and height in no. of characters)
#define LOG_DEBUG_POSX          80
#define LOG_DEBUG_POSY          0
#define LOG_DEBUG_WIDTH         24
#define LOG_DEBUG_HEIGHT        8

// Fonts
// https://github.com/olikraus/u8g2/wiki/fntgrp
#define FONT_CLOCK              u8g2_font_logisoso24_tf
#define FONT_BOOTLOG            u8g2_font_4x6_tf
#define FONT_DEBUGLOG           u8g2_font_4x6_tf
#define FONT_S                  u8g2_font_smallsimple_tr //u8g2_font_simple1_tf=h13w9

// Widget positions
#define POSX_CLOCK              186
#define POSY_CLOCK              26

#define POSX_AUDIO_ICON         2
#define POSY_AUDIO_ICON         62
#define POSX_AUDIO              22
#define POSY_AUDIO              62

// LED ring
#define LED_RING_NUM_LEDS       24

#endif
