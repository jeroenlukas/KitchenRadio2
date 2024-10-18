#ifndef CONFIGPINOUT_H
#define CONFIGPINOUT_H

// SPI bus
#define HSPI_SCK 12 
#define HSPI_MISO 13
#define HSPI_MOSI 11
#define HSPI_CS 48
#define HSPI_DC 10

// Wiring of VS1053 board (SPI connected in a standard way)
#define VS1053_CS 14
#define VS1053_DCS 47
#define VS1053_DREQ 21

// Buttons
#define BUTTONS     36

// Pot meters
#define POT_VOLUME   34
//#define POT_TREBLE   36
//#define POT_BASS     39

// Encoder
#define ROTARY_A    4
#define ROTARY_B    0
#define BUTTON_ENCODER  2

#endif