#ifndef CONFIGPINOUT_H
#define CONFIGPINOUT_H

// SPI bus
#define HSPI_SCK 12 
#define HSPI_MISO 13
#define HSPI_MOSI 11
#define HSPI_CS 48
#define HSPI_DC 10

// VS1053 board (SPI connected in a standard way)
#define VS1053_CS 14
#define VS1053_DCS 47
#define VS1053_DREQ 21

// UART for comms with the KCX bluetooth module
// Note, this is seen from the ESP's perspective, so TX = ESP output and RX = ESP input
#define KCX_TX  43
#define KCX_RX  44
#define KCX_CONNECT 42


// LEDs
#define LED_WEBRADIO    6
#define LED_BLUETOOTH   7
#define LED_ALARM   15
#define LED_LAMP    16

// Buttons
#define BUTTONS     4


// Pot meters
#define POT_VOLUME   5

// Encoder
#define ROTARY_A    38
#define ROTARY_B    39
#define BUTTON_ENCODER  40

#endif